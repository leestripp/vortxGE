bl_info = {
    "name": "vortxGE Scene Exporter",
    "author": "Lee A. Stripp",
    "version": (1, 0),
    "blender": (4, 0, 0),
    "location": "File > Export > vortxGE Scene (.vgs)",
    "description": "Exports the scene to a vortxGE .vgs file",
    "warning": "",
    "doc_url": "",
    "category": "Import-Export",
}

import bpy
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
        # This is to counteract a -90 degree rotation that is often baked into
        # exported models (e.g., glTF) to convert from Blender's Z-up to a Y-up
        # system. The combination of this correction and the C_mat4 transform
        # results in using the Blender world transform directly, which is correct
        # if the exported model data is already in the engine's expected Y-up space.
        R_corr = Matrix.Rotation(math.radians(90.0), 4, 'X')
        world_matrix = world_matrix @ R_corr

    # Transform the matrix into vortxGE's coordinate space
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
    Writes the current Blender scene to the specified filepath in .vgs format.
    """
    try:
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
                        light_type_str = 'sun'
                    elif light.type == 'SPOT':
                        light_type_str = 'spot'
                    else:
                        print(f"Skipping unsupported light type '{light.type}' for object '{obj.name}'")
                        continue

                    name = f"`{obj.name}`"
                    parent = f"`{scene_name}`"
                    
                    loc, rot, _ = get_vortxge_transform(obj, is_mesh=False)
                    color = light.color

                    f.write(f"add_light {light_type_str} {name} {parent} ")
                    f.write(f"({loc.x:.6f}, {loc.y:.6f}, {loc.z:.6f}) ")
                    f.write(f"({rot.x:.6f}, {rot.y:.6f}, {rot.z:.6f}) ")
                    f.write(f"({color.r:.6f}, {color.g:.6f}, {color.b:.6f})\n")
                f.write("\n")

            # --- Export Objects (Meshes) ---
            meshes = [obj for obj in context.scene.objects if obj.type == 'MESH']
            if meshes:
                for obj in meshes:
                    name = f"`{obj.name}`"
                    parent = f"`{scene_name}`"
                    
                    loc, rot, scale = get_vortxge_transform(obj, is_mesh=True)

                    # Assume asset path convention: data/meshes/OBJECT_NAME.glb
                    # Blender's obj.name is the name of the object.
                    asset_path = f"`data/meshes/{obj.name}.glb`"

                    f.write(f"add_object {name} {parent} ")
                    f.write(f"({loc.x:.6f}, {loc.y:.6f}, {loc.z:.6f}) ")
                    f.write(f"({rot.x:.6f}, {rot.y:.6f}, {rot.z:.6f}) ")
                    f.write(f"({scale.x:.6f}, {scale.y:.6f}, {scale.z:.6f}) ")
                    f.write(f"{asset_path}\n")

        print(f"Scene successfully exported to {filepath}")
        return True

    except IOError as e:
        print(f"Error writing to file {filepath}: {e}")
        return False
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        return False


class ExportVortxGEScene(Operator, ExportHelper):
    """Export the current scene to a vortxGE .vgs file"""
    bl_idname = "export_scene.vortxge"
    bl_label = "Export vortxGE Scene"

    # ExportHelper mixin class uses these properties to display the file browser
    filename_ext = ".vgs"

    filter_glob: StringProperty(
        default="*.vgs",
        options={'HIDDEN'},
        maxlen=255,  # Max path length
    )

    def execute(self, context):
        if write_vortxge_scene(context, self.filepath):
            return {'FINISHED'}
        else:
            self.report({'ERROR'}, "Scene export failed. Check console for details.")
            return {'CANCELLED'}


# Add to a menu
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

