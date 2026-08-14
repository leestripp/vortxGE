# Debugging & Diagnostic Hooks

Runtime env-var hooks for reproducing and capturing rendering bugs.
All are compiled under `VORTX_DEBUG_TOOLS` and are **inert when their
env var is unset**, so they are safe to leave in. Compile-time debug
flags (`VORTX_DEBUG_RENDER` etc.) are listed in
[utils/Utilities.md](utils/Utilities.md); Vulkan-specific hooks are in
[renderer/VulkanBackend.md](renderer/VulkanBackend.md).

## Frame captures

| Var | Effect |
|---|---|
| `VORTX_SHOT=path.png` | Capture the framebuffer: GL writes `path_30.png` + `path_90.png` (two-shot temporal A/B), VK writes frame 30 |
| `VORTX_SHOT_EVERY=N` | Multi-frame mode: capture every N frames, frames 20..40000, GL + VK |
| `VORTX_FIXED_DT=<ms>` | Fixed sim timestep (deterministic captures). **Disables the demo cube emitter** — don't use it when reproducing long-session/draw-count bugs |
| `VORTX_CAM_ORBIT=1` | Tiny deterministic camera orbit (shadow-flicker repro) |

Typical repro run:

```sh
cd build/demos/vortxGEdemo_glfw3
VORTX_SHOT=/tmp/repro.png VORTX_SHOT_EVERY=15 ./vortxGEdemo_glfw3
# play normally; frames land in /tmp/repro_<N>.png
```

## Cameras & player input (demoscene)

| Var | Effect |
|---|---|
| `VORTX_DEBUG_CAM=1` | Create a standalone scene-owned camera and make it active. Never touches the FirstPersonRig's camera |
| `VORTX_CAM_YAW=<deg>` / `VORTX_CAM_PITCH=<deg>` | Steer the debug camera's rotation |
| `VORTX_CAM_POS="x,y,z"` | Steer the debug camera's position |
| `VORTX_RIG_YAW=<deg>` / `VORTX_RIG_PITCH=<deg>` | Rotate the player rig once through its REAL input path (`HandleMouseLook` → Bullet yaw quaternion) |
| `VORTX_MOUSE_SPIN=<deg/frame>` (+ `VORTX_MOUSE_SPIN_PITCH`) | Continuous mouse-look through the real input path |
| `VORTX_WALK=w\|a\|s\|d` | Hold a movement key through `HandleKeyboardInput` |

The rig hooks exist because driving the rig's camera directly fights
the rig (it owns and re-derives its camera every frame). To reproduce
input-path bugs, drive the rig through `HandleMouseLook` /
`HandleKeyboardInput`, and watch through `VORTX_DEBUG_CAM` if you need
an independent viewpoint.

## Sky / day-night (demoscene)

| Var | Effect |
|---|---|
| `VORTX_TIME_OF_DAY=<hours>` | Start time, default 10 |
| `VORTX_DAY_LENGTH=<seconds>` | Day-cycle length; 0 = frozen. See [scene/ProceduralSky.md](scene/ProceduralSky.md) |

## Misc

| Var | Effect |
|---|---|
| `VORTX_RENDERER=opengl\|vulkan` | Force backend (default: probe Vulkan first) |
| `VORTX_SHADOW_QUALITY=low\|med\|high` | Shadow preset (default high; high adds the depth prepass) |
| `VORTX_PHYSICS_COUNT=1` | Log physics body count + fps every 5 s |
| `VORTX_PASS_TRACE=1` / `VORTX_SHADOW_TRACE=1` | Pass / shadow-stage tracing |
