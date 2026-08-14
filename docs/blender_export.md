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

## Light custom properties (on Blender lights)

- `cast_shadows` (bool, default true) — uncheck for decorative/fill lights.
  Exported as a trailing flag on `add_light`; the pipeline only creates
  shadow-map stages for casters (a 64-light scene would otherwise try to
  build 64 shadow passes). Shadow casters stay in the hero slots 0–2.

## Live-link (hot-apply edits into a running demo)

Phase 1 (engine side) is done; phase 2 (one-click push from Blender) is
in progress.

1. Run a demo with the live-link enabled:

   ```bash
   VORTX_PIPELINE=1 VORTX_SCENE=light_stress VORTX_LIVELINK=1 ./vortxGEdemo_glfw3
   ```

2. The demo polls `data/<scene>.vgs` at 2 Hz. Any write re-applies
   transforms + light color/intensity to the matching nodes **in place**
   (`lsSceneManager::ApplySceneTransforms`) — no scene rebuild, so the
   pipeline's shadow stages keep their light pointers.

3. Push from Blender by re-exporting the current scene state onto the
   watched file (via BlenderMCP `execute_code`, or the exporter):

   ```python
   import sys; sys.path.insert(0, '<repo>/blender_scripts')
   import export_scene
   export_scene.write_vortxge_scene(bpy.context, '<repo>/demos/data/light_stress.vgs')
   ```

   The demo logs `INFO: live-link applied N node update(s)` and the edit
   appears within ~0.5 s.

Limitations: transform/property updates only — adding or removing objects
is structural and still requires a restart.

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
