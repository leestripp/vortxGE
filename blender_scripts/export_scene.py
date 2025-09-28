bl_info = {
    "name": "vortxGE Scene Exporter",
    "author": "Lee A. Stripp",
    "version": (1, 2),
    "blender": (4, 5, 0),
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
from bpy.props import StringProperty
from bpy.types import Operator


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
        # This is to counteract a -90 degree rotation that is baked into
        # exported glTF models to convert from Blender's Z-up to the standard Y-up
        # system. This ensures the transform in the .vgs file correctly orients
        # the already Y-up model data from the .glb file.
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
                    f.write(f"({color.r:.6f}, {color.g:.6f}, {color.b:.6f}) {intensity:.6f}\n")
                f.write("\n")

            # --- Export Objects (Meshes) and Physics-Only Meshes ---
            meshes = [obj for obj in context.scene.objects if obj.type == 'MESH']
            if meshes:
                for obj in meshes:
                    # Determine GLB export path
                    glb_filepath = os.path.join(meshes_dir, f"{obj.name}.glb")
                    asset_path = f"`data/meshes/{obj.name}.glb`"
                    name = f"`{obj.name}`"

                    # --- Export GLB for all meshes ---
                    print(f"Exporting mesh '{obj.name}' to {glb_filepath}")
                    bpy.ops.object.select_all(action='DESELECT')
                    obj.select_set(True)
                    context.view_layer.objects.active = obj
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

                    # --- Write object or physics-only entry to .vgs file ---
                    if "bullet_only_mesh" in obj:
                        try:
                            prop_val = str(obj["bullet_only_mesh"]).strip()
                            parts = prop_val.split(maxsplit=1)
                            if len(parts) >= 2:
                                physics_type = parts[0]
                                mass = parts[1]
                                
                                # Get the position for bullet-only meshes
                                loc, rot, scale = get_vortxge_transform(obj, is_mesh=True)
                                
                                f.write(f"add_bullet_only_mesh {name} {physics_type} {mass} {asset_path} ")
                                f.write(f"({loc.x:.6f}, {loc.y:.6f}, {loc.z:.6f}) ")
                                f.write(f"({rot.x:.6f}, {rot.y:.6f}, {rot.z:.6f}) ")
                                f.write(f"({scale.x:.6f}, {scale.y:.6f}, {scale.z:.6f})\n")
                            else:
                                print(f"WARNING: Invalid format for 'bullet_only_mesh' on '{obj.name}'. Expected 'type mass'.")
                        except Exception as e:
                            print(f"ERROR processing 'bullet_only_mesh' for '{obj.name}': {e}")
                    else:
                        # Normal mesh object
                        parent = f"`{scene_name}`"
                        loc, rot, scale = get_vortxge_transform(obj, is_mesh=True)

                        f.write(f"add_object {name} {parent} ")
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
                                    f.write(f"add_bullet_box {name} {mass} {size_str}\n")
                                else:
                                    print(f"WARNING: Invalid format for 'bullet_box' on '{obj.name}'. Expected 'mass (sx, sy, sz)'.")

                            if "bullet_pill" in obj:
                                prop_val = str(obj["bullet_pill"])
                                parts = prop_val.split()
                                if len(parts) == 3:
                                    mass, radius, height = parts
                                    f.write(f"add_bullet_pill {name} {mass} {radius} {height}\n")
                                else:
                                    print(f"WARNING: Invalid format for 'bullet_pill' on '{obj.name}'. Expected 'mass radius height'.")

                            if "bullet_convexhull" in obj:
                                mass = str(obj["bullet_convexhull"]).strip()
                                if mass:
                                    f.write(f"add_bullet_convexhull {name} {mass}\n")
                                else:
                                    print(f"WARNING: Empty 'bullet_convexhull' property on '{obj.name}'.")

                            if "bullet_trimesh" in obj:
                                mass = str(obj["bullet_trimesh"]).strip()
                                if mass:
                                    f.write(f"add_bullet_trimesh {name} {mass}\n")
                                else:
                                    print(f"WARNING: Empty 'bullet_trimesh' property on '{obj.name}'.")

                        except Exception as e:
                            print(f"ERROR processing custom properties for '{obj.name}': {e}")
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


def register():
    bpy.utils.register_class(ExportVortxGEScene)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportVortxGEScene)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

