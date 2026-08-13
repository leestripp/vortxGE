# Blender → vortxGE scene pipeline

Blender is the vortxGE scene editor. `.blend` files are the source of
truth; the engine consumes exported `.vgs` text scenes + `.glb` meshes.

## Layout

- `demos/scenes_src/<name>.blend` — committed scene sources (Blender 5.2 LTS)
- `demos/data/<name>.vgs` — exported scene (committed)
- `demos/data/meshes/*.glb` — exported mesh assets, **shared global namespace**
  across all scenes: give mesh data unique names (e.g. `GymFloor`) unless you
  deliberately share an asset between scenes. A `Cube` in two scenes collides.
- `blender_scripts/export_scene.py` — Blender addon (File > Export > vortxGE
  Scene) with the vortxGE Tools panel (physics custom properties)
- `blender_scripts/export_blend_headless.py` — headless export wrapper
- `blender_scripts/make_scene_<name>.py` — reproducible scene generators
  (`blender -b --factory-startup -P make_scene_<name>.py`)
- `tools/export_scenes.py` — freshness check + headless re-export of stale
  scenes (Godot-style: `.blend` newer than `.vgs` → re-export)

## Workflow

1. Edit `demos/scenes_src/<name>.blend` in Blender (or write/extend a
   `make_scene_*.py` generator).
2. `python3 tools/export_scenes.py` (or `make export_scenes`) re-exports
   stale scenes. `VORTX_BLENDER=/path/to/blender` overrides the binary.
3. Run the demo with `VORTX_SCENE=<name>` (default: `demoscene`).
   Tech scenes are pure .vgs content; the base scene additionally keeps its
   procedural extras (emitter, sample models).

## Physics custom properties (on Blender objects)

- `bullet_box` = `"mass (sx, sy, sz)"`, `bullet_pill` = `"mass radius height"`,
  `bullet_convexhull` / `bullet_trimesh` = `"mass"` — rendered + physical
- `bullet_only_mesh` = `"type mass"` — invisible collider only
- optional `friction`, `rolling_friction`, `spinning_friction`
- `no_export` (bool) — skip object entirely

## Blender 5.2 gotchas (learned 2026-08-13)

- `bpy.context.active_object` is gone → use
  `bpy.context.view_layer.objects.active`.
- `bpy.ops.object.transform_apply(scale=True)` applies LOCATION too unless
  you pass `location=False, rotation=False` explicitly.
