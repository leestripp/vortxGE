
# Blender scripts

## export_scene.py
Export your scene objects. This is a standard Blender addon. To use it, install and enable it through Blender's preferences. You will find the exporter under `File > Export > vortxGE Scene (.vgs)`.

The script transforms the coordinates from Blender's Z-up system to vortxGE's Y-up system. For each mesh object, it exports a separate `.glb` file into a `meshes/` subdirectory and references it in the main scene file, which aligns with the engine's asset management approach.
