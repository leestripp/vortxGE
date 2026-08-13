bl_info = {
    "name": "vortxGE Scene Exporter",
    "author": "Lee A. Stripp",
    "version": (1, 5),
    "blender": (5, 2, 0),
    "location": "File > Export > vortxGE Scene (.vgs)",
    "description": "Exports the scene to a vortxGE .vgs file and all meshes to .glb files",
    "warning": "",
    "doc_url": "",
    "category": "Import-Export",
}

import bpy
import os
import math
from mathutils import Matrix, Vector
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty, FloatProperty # Added FloatProperty
from bpy.types import Operator, Panel


def get_vortxge_transform(blender_obj, is_mesh=False):
    """
    Converts a Blender object's world matrix to vortxGE's coordinate system
    and decomposes it into location, rotation (ZYX Euler degrees), and scale.

    Blender coordinate system (Right-Handed):
    +X: Right
    +Y: Forward
    +Z: Up

    vortxGE coordinate system (Right-Handed, OpenGL style):
    +X: Right
    +Y: Up
    -Z: Forward

    Conversion from Blender to vortxGE:
    vortx_x = blender_x
    vortx_y = blender_z
    vortx_z = -blender_y
    """
    # 4x4 matrix to convert from Blender's coordinate system to vortxGE's
    C_mat4 = Matrix(((1.0, 0.0,  0.0, 0.0),
                     (0.0, 0.0,  1.0, 0.0),
                     (0.0, -1.0, 0.0, 0.0),
                     (0.0, 0.0,  0.0, 1.0)))

    # The object's transform in Blender's world space
    world_matrix = blender_obj.matrix_world

    if is_mesh:
        # For meshes, apply a corrective rotation of +90 degrees around the X-axis.
        R_corr = Matrix.Rotation(math.radians(90.0), 4, 'X')
        world_matrix = world_matrix @ R_corr

    # Apply the coordinate system conversion
    final_vortx_mat = C_mat4 @ world_matrix

    # Decompose the transformed matrix to get location, rotation, and scale
    loc, rot_quat, scale = final_vortx_mat.decompose()

    # Convert rotation to ZYX Euler angles in degrees, as expected by the engine
    rot_euler = rot_quat.to_euler('ZYX')
    rot_degrees = Vector((math.degrees(rot_euler.x),
                          math.degrees(rot_euler.y),
                          math.degrees(rot_euler.z)))

    return loc, rot_degrees, scale


def get_mesh_data_name(obj):
    """
    Get the mesh data name for an object.
    For objects with shared mesh data (Alt+D copies), this will be the same.
    """
    if obj.type == 'MESH' and obj.data:
        return obj.data.name
    return obj.name


def write_vortxge_scene(context, filepath):
    """
    Writes the current Blender scene to the specified filepath in .vgs format
    and exports all mesh objects to individual .glb files.
    """
    # Store original selection to restore it later
    original_selection = context.selected_objects
    original_active = context.view_layer.objects.active

    try:
        # --- Prepare Directories ---
        vgs_dir = os.path.dirname(filepath)
        meshes_dir = os.path.join(vgs_dir, "meshes")
        os.makedirs(meshes_dir, exist_ok=True)

        # Keep track of already exported mesh data names to avoid duplicates
        exported_mesh_data = set()

        with open(filepath, 'w') as f:
            # --- Scene Header ---
            scene_name = context.scene.name
            f.write(f"add_scene `{scene_name}`\n\n")

            # --- Export Lights ---
            lights = [obj for obj in context.scene.objects if obj.type == 'LIGHT']
            if lights:
                for obj in lights:
                    light = obj.data

                    light_type_str = 'spot' # Default
                    if light.type == 'POINT':
                        light_type_str = 'point'
                    elif light.type == 'SUN':
                        light_type_str = 'direct'
                    elif light.type == 'SPOT':
                        light_type_str = 'spot'
                    else:
                        print(f"Skipping unsupported light type '{light.type}' for object '{obj.name}'")
                        continue

                    name = f"`{obj.name}`"
                    parent = f"`{scene_name}`"

                    loc, rot, _ = get_vortxge_transform(obj, is_mesh=False)
                    color = light.color
                    intensity = light.energy

                    f.write(f"add_light {light_type_str} {name} {parent} ")
                    f.write(f"({loc.x:.6f}, {loc.y:.6f}, {loc.z:.6f}) ")
                    f.write(f"({rot.x:.6f}, {rot.y:.6f}, {rot.z:.6f}) ")
                    cast_shadows = 1 if obj.get("cast_shadows", True) else 0
                    f.write(f"({color.r:.6f}, {color.g:.6f}, {color.b:.6f}) {intensity:.6f} {cast_shadows}\n")
                f.write("\n")

            # --- Export Objects (Meshes) and Physics-Only Meshes ---
            meshes = [obj for obj in context.scene.objects if obj.type == 'MESH']
            if meshes:
                for obj in meshes:
                    # Skip objects with the "no_export" custom property
                    if "no_export" in obj and obj["no_export"]:
                        print(f"Skipping object '{obj.name}' due to 'no_export' property")
                        continue
                    
                    # Get the object's own name and its underlying mesh data name.
                    object_name = obj.name
                    mesh_data_name = get_mesh_data_name(obj)

                    # The GLB file and asset path are based on the MESH DATA name.
                    # This ensures linked duplicates (Alt+D) point to the same asset.
                    glb_filename = f"{mesh_data_name}.glb"
                    glb_filepath = os.path.join(meshes_dir, glb_filename)
                    asset_path = f"`data/meshes/{glb_filename}`"

                    # The object entry in the .vgs file uses the unique OBJECT name.
                    # This ensures each instance has its own transform in the scene.
                    vgs_object_name = f"`{object_name}`"

                    # --- Export GLB for each unique mesh data ---
                    if mesh_data_name not in exported_mesh_data:
                        print(f"Exporting mesh data '{mesh_data_name}' to {glb_filepath}")

                        # Deselect all, then select all objects that share this mesh data.
                        bpy.ops.object.select_all(action='DESELECT')
                        for scene_obj in context.scene.objects:
                            if (scene_obj.type == 'MESH' and
                                scene_obj.data and
                                scene_obj.data.name == mesh_data_name):
                                scene_obj.select_set(True)

                        # Set active object to one of the objects with this mesh data
                        context.view_layer.objects.active = obj

                        # Export the selected objects as a single GLB file.
                        bpy.ops.export_scene.gltf(
                            filepath=glb_filepath,
                            use_selection=True,
                            export_format='GLB',
                            export_apply=True,
                            export_texcoords=True,
                            export_normals=True,
                            export_tangents=True,
                            export_materials='EXPORT',
                            export_yup=True
                        )

                        # Mark this mesh data as exported to prevent duplicates.
                        exported_mesh_data.add(mesh_data_name)
                    else:
                        print(f"Mesh data '{mesh_data_name}' for object '{object_name}' already exported, skipping GLB creation...")

                    # --- Write object entry to .vgs file ---
                    # Check for "bullet_only_mesh" property
                    if "bullet_only_mesh" in obj:
                        try:
                            prop_val = str(obj["bullet_only_mesh"]).strip()
                            parts = prop_val.split(maxsplit=1)
                            
                            # bullet_only_mesh uses format "type mass", e.g., "box 1.0"
                            if len(parts) >= 2:
                                physics_type = parts[0]
                                mass = parts[1]

                                # Get the position for bullet-only meshes
                                loc, rot, scale = get_vortxge_transform(obj, is_mesh=True)

                                f.write(f"add_bullet_only_mesh {vgs_object_name} {physics_type} {mass} {asset_path} ")
                                f.write(f"({loc.x:.6f}, {loc.y:.6f}, {loc.z:.6f}) ")
                                f.write(f"({rot.x:.6f}, {rot.y:.6f}, {rot.z:.6f}) ")
                                f.write(f"({scale.x:.6f}, {scale.y:.6f}, {scale.z:.6f})")

                                # Add friction properties if they exist
                                friction = obj.get("friction", 0.8)
                                rolling_friction = obj.get("rolling_friction", 0.0)
                                spinning_friction = obj.get("spinning_friction", 0.0)
                                f.write(f" {friction:.6f} {rolling_friction:.6f} {spinning_friction:.6f}\n")
                            else:
                                print(f"WARNING: Invalid format for 'bullet_only_mesh' on '{object_name}'. Expected 'type mass'.")
                        except Exception as e:
                            print(f"ERROR processing 'bullet_only_mesh' for '{object_name}': {e}")
                    else:
                        # Normal mesh object
                        parent = f"`{scene_name}`"
                        loc, rot, scale = get_vortxge_transform(obj, is_mesh=True)

                        f.write(f"add_object {vgs_object_name} {parent} ")
                        f.write(f"({loc.x:.6f}, {loc.y:.6f}, {loc.z:.6f}) ")
                        f.write(f"({rot.x:.6f}, {rot.y:.6f}, {rot.z:.6f}) ")
                        f.write(f"({scale.x:.6f}, {scale.y:.6f}, {scale.z:.6f}) ")
                        f.write(f"{asset_path}\n")

                        # --- Check for existing physics properties (for rendered objects) ---
                        try:
                            if "bullet_box" in obj:
                                prop_val = str(obj["bullet_box"])
                                parts = prop_val.split(' ', 1)
                                if len(parts) == 2:
                                    mass = parts[0]
                                    size_str = parts[1]
                                    f.write(f"add_bullet_box {vgs_object_name} {mass} {size_str}")

                                    # Add friction properties if they exist
                                    friction = obj.get("friction", 0.8)
                                    rolling_friction = obj.get("rolling_friction", 0.0)
                                    spinning_friction = obj.get("spinning_friction", 0.0)
                                    f.write(f" {friction:.6f} {rolling_friction:.6f} {spinning_friction:.6f}\n")
                                else:
                                    print(f"WARNING: Invalid format for 'bullet_box' on '{object_name}'. Expected 'mass (sx, sy, sz)'.")

                            if "bullet_pill" in obj:
                                prop_val = str(obj["bullet_pill"])
                                parts = prop_val.split()
                                if len(parts) == 3:
                                    mass, radius, height = parts
                                    f.write(f"add_bullet_pill {vgs_object_name} {mass} {radius} {height}")

                                    # Add friction properties if they exist
                                    friction = obj.get("friction", 0.8)
                                    rolling_friction = obj.get("rolling_friction", 0.0)
                                    spinning_friction = obj.get("spinning_friction", 0.0)
                                    f.write(f" {friction:.6f} {rolling_friction:.6f} {spinning_friction:.6f}\n")
                                else:
                                    print(f"WARNING: Invalid format for 'bullet_pill' on '{object_name}'. Expected 'mass radius height'.")

                            if "bullet_convexhull" in obj:
                                mass = str(obj["bullet_convexhull"]).strip()
                                if mass:
                                    f.write(f"add_bullet_convexhull {vgs_object_name} {mass}")

                                    # Add friction properties if they exist
                                    friction = obj.get("friction", 0.8)
                                    rolling_friction = obj.get("rolling_friction", 0.0)
                                    spinning_friction = obj.get("spinning_friction", 0.0)
                                    f.write(f" {friction:.6f} {rolling_friction:.6f} {spinning_friction:.6f}\n")
                                else:
                                    print(f"WARNING: Empty 'bullet_convexhull' property on '{object_name}'.")

                            if "bullet_trimesh" in obj:
                                mass = str(obj["bullet_trimesh"]).strip()
                                if mass:
                                    f.write(f"add_bullet_trimesh {vgs_object_name} {mass}")

                                    # Add friction properties if they exist
                                    friction = obj.get("friction", 0.8)
                                    rolling_friction = obj.get("rolling_friction", 0.0)
                                    spinning_friction = obj.get("spinning_friction", 0.0)
                                    f.write(f" {friction:.6f} {rolling_friction:.6f} {spinning_friction:.6f}\n")
                                else:
                                    print(f"WARNING: Empty 'bullet_trimesh' property on '{object_name}'.")

                        except Exception as e:
                            print(f"ERROR processing custom properties for '{object_name}': {e}")
                f.write("\n")

        print(f"Scene successfully exported to {filepath}")
        return True

    except IOError as e:
        print(f"Error writing to file {filepath}: {e}")
        return False
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        raise
    finally:
        bpy.ops.object.select_all(action='DESELECT')
        if original_active:
            try:
                context.view_layer.objects.active = original_active
            except:
                pass
        for sel_obj in original_selection:
            try:
                sel_obj.select_set(True)
            except:
                pass


class ExportVortxGEScene(Operator, ExportHelper):
    """Export the current scene to a vortxGE .vgs file and its meshes to .glb"""
    bl_idname = "export_scene.vortxge"
    bl_label = "Export vortxGE Scene"

    filename_ext = ".vgs"

    filter_glob: StringProperty(
        default="*.vgs",
        options={'HIDDEN'},
        maxlen=255,
    )

    def execute(self, context):
        try:
            if write_vortxge_scene(context, self.filepath):
                return {'FINISHED'}
            else:
                self.report({'ERROR'}, "Scene export failed. Check console for details.")
                return {'CANCELLED'}
        except Exception as e:
            self.report({'ERROR'}, f"Scene export failed: {e}")
            return {'CANCELLED'}


def menu_func_export(self, context):
    self.layout.operator(ExportVortxGEScene.bl_idname, text="vortxGE Scene (.vgs)")


class VortxGEPanel(Panel):
    """Creates a Panel in the Object properties window"""
    bl_label = "vortxGE Tools"
    bl_idname = "OBJECT_PT_vortxge_tools"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        obj = context.object
        
        # Check if the object is a MESH to show physics tools
        if obj and obj.type == 'MESH':
            
            # No export property
            row = layout.row()
            row.prop(obj, "no_export", text="Exclude from Export")

            # Physics properties panel
            box = layout.box()
            box.label(text="Physics Properties")
            
            # Mass input (uses the registered custom property on Object)
            row = box.row()
            row.prop(obj, "mass", text="Mass")
            
            # Create physics buttons
            col = box.column(align=True) # Use a column for better layout
            col.label(text="Add Physics Component:")
            
            # Row 1: Box and Pill
            row1 = col.row()
            row1.operator("object.create_physics_box", text="Box")
            row1.operator("object.create_physics_pill", text="Pill")
            
            # Row 2: Convex Hull and Tri-Mesh
            row2 = col.row()
            row2.operator("object.create_physics_convexhull", text="Convex Hull")
            row2.operator("object.create_physics_trimesh", text="Tri-Mesh")
            
            # Row 3: Bullet Only Mesh
            col.separator()
            col.label(text="Collider Only:")
            col.operator("object.create_physics_bullet_only_mesh", text="Collider (Invisible in Game)", icon='HIDE_ON')


# Helper function to clean up other physics properties
def clean_physics_properties(obj, exclude_prop=""):
    """Removes all physics custom properties except the one specified."""
    props_to_check = [
        "bullet_box", "bullet_pill", "bullet_convexhull", 
        "bullet_trimesh", "bullet_only_mesh"
    ]
    for prop in props_to_check:
        if prop in obj and prop != exclude_prop:
            del obj[prop]


class CreatePhysicsBulletOnlyMesh(Operator):
    """Create bullet_only_mesh physics for selected object and set wireframe display"""
    bl_idname = "object.create_physics_bullet_only_mesh"
    bl_label = "Create Bullet Only Mesh"
    bl_options = {'REGISTER', 'UNDO'}
    
    # Enum to select the underlying physics shape type for the bullet_only_mesh property
    physics_shape: EnumProperty(
        name="Collider Shape",
        description="The collision shape the engine will use for this invisible collider",
        items=[
            ('box', 'Box', 'Use a box shape collider'),
            ('pill', 'Pill', 'Use a pill/capsule shape collider'),
            ('convexhull', 'Convex Hull', 'Use a convex hull shape collider'),
            ('trimesh', 'Tri-Mesh', 'Use a triangle mesh shape collider (slowest)')
        ],
        default='box'
    )

    def invoke(self, context, event):
        # Open a dialog to choose the physics shape before executing
        wm = context.window_manager
        return wm.invoke_props_dialog(self)

    def execute(self, context):
        obj = context.active_object
        if obj is None or obj.type != 'MESH':
            self.report({'ERROR'}, "No active mesh object selected.")
            return {'CANCELLED'}
            
        clean_physics_properties(obj, exclude_prop="bullet_only_mesh")
        
        # Set the custom property: "bullet_only_mesh": "SHAPE MASS"
        obj["bullet_only_mesh"] = f"{self.physics_shape} {obj.mass:.6f}" 
        
        # Set the object's viewport display to wireframe
        obj.display_type = 'WIRE'
        
        # Set friction properties (standard defaults)
        obj["friction"] = 0.8
        obj["rolling_friction"] = 0.0
        obj["spinning_friction"] = 0.0
        
        self.report({'INFO'}, f"Collider-Only physics ({self.physics_shape}) set for {obj.name}. Display set to Wireframe.")
        return {'FINISHED'}


class CreatePhysicsBox(Operator):
    """Create box physics for selected object"""
    bl_idname = "object.create_physics_box"
    bl_label = "Create Box Physics"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.active_object
        if obj is None or obj.type != 'MESH':
            self.report({'ERROR'}, "No active mesh object selected.")
            return {'CANCELLED'}
            
        clean_physics_properties(obj, exclude_prop="bullet_box")
                
        # Set physics property. obj.mass is the registered float property.
        obj["bullet_box"] = f"{obj.mass:.6f} (0.5, 0.5, 0.5)" # Use mass from the panel
        obj.display_type = 'TEXTURED' # Reset display type for rendered object
        
        # Set friction properties (standard defaults)
        obj["friction"] = 0.8
        obj["rolling_friction"] = 0.0
        obj["spinning_friction"] = 0.0
        
        self.report({'INFO'}, f"Box physics set for {obj.name}")
        return {'FINISHED'}


class CreatePhysicsPill(Operator):
    """Create pill physics for selected object"""
    bl_idname = "object.create_physics_pill"
    bl_label = "Create Pill Physics"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.active_object
        if obj is None or obj.type != 'MESH':
            self.report({'ERROR'}, "No active mesh object selected.")
            return {'CANCELLED'}
            
        clean_physics_properties(obj, exclude_prop="bullet_pill")
                
        # Set physics property. obj.mass is the registered float property.
        obj["bullet_pill"] = f"{obj.mass:.6f} 0.5 1.0" # Use mass from the panel
        obj.display_type = 'TEXTURED' # Reset display type for rendered object
        
        # Set friction properties
        obj["friction"] = 0.8
        obj["rolling_friction"] = 0.0
        obj["spinning_friction"] = 0.0

        self.report({'INFO'}, f"Pill physics set for {obj.name}")
        return {'FINISHED'}


class CreatePhysicsConvexHull(Operator):
    """Create convex hull physics for selected object"""
    bl_idname = "object.create_physics_convexhull"
    bl_label = "Create Convex Hull Physics"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.active_object
        if obj is None or obj.type != 'MESH':
            self.report({'ERROR'}, "No active mesh object selected.")
            return {'CANCELLED'}
            
        clean_physics_properties(obj, exclude_prop="bullet_convexhull")
                
        # Set physics property. obj.mass is the registered float property.
        obj["bullet_convexhull"] = f"{obj.mass:.6f}" # Use mass from the panel
        obj.display_type = 'TEXTURED' # Reset display type for rendered object
        
        # Set friction properties
        obj["friction"] = 0.8
        obj["rolling_friction"] = 0.0
        obj["spinning_friction"] = 0.0
        
        self.report({'INFO'}, f"Convex Hull physics set for {obj.name}")
        return {'FINISHED'}


class CreatePhysicsTriMesh(Operator):
    """Create tri-mesh physics for selected object"""
    bl_idname = "object.create_physics_trimesh"
    bl_label = "Create Tri-Mesh Physics"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        obj = context.active_object
        if obj is None or obj.type != 'MESH':
            self.report({'ERROR'}, "No active mesh object selected.")
            return {'CANCELLED'}
            
        clean_physics_properties(obj, exclude_prop="bullet_trimesh")
                
        # Set physics property. obj.mass is the registered float property.
        obj["bullet_trimesh"] = f"{obj.mass:.6f}" # Use mass from the panel
        obj.display_type = 'TEXTURED' # Reset display type for rendered object
        
        # Set friction properties
        obj["friction"] = 0.8
        obj["rolling_friction"] = 0.0
        obj["spinning_friction"] = 0.0

        self.report({'INFO'}, f"Tri-Mesh physics set for {obj.name}")
        return {'FINISHED'}


# --- Register/Unregister Functions ---

classes = (
    ExportVortxGEScene,
    VortxGEPanel,
    CreatePhysicsBox,
    CreatePhysicsPill,
    CreatePhysicsConvexHull,
    CreatePhysicsTriMesh,
    CreatePhysicsBulletOnlyMesh # New operator added here
)

def register():
    # Register custom properties
    bpy.types.Object.no_export = BoolProperty(
        name="No Export",
        description="Exclude this object from vortxGE scene export",
        default=False
    )
    
    bpy.types.Object.physics_type = EnumProperty(
        name="Physics Type (Info only)",
        description="Physics shape type (Set by buttons)",
        items=[
            ('box', 'Box', 'Box shape'),
            ('pill', 'Pill', 'Pill (cylinder) shape'),
            ('convexhull', 'Convex Hull', 'Convex hull shape'),
            ('trimesh', 'Tri-Mesh', 'Triangle mesh shape')
        ],
        default='box'
    )
    
    bpy.types.Object.mass = FloatProperty(
        name="Mass",
        description="Physics mass value",
        default=1.0,
        min=0.0
    )
    
    # Register all classes
    for cls in classes:
        bpy.utils.register_class(cls)
    
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    
    # Unregister all classes
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
        
    # Unregister custom properties
    del bpy.types.Object.no_export
    del bpy.types.Object.physics_type
    del bpy.types.Object.mass


if __name__ == "__main__":
    register()
