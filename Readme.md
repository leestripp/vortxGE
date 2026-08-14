# vortxGE

A C++23 game engine distributed as a **static library** — you link it into
your game and drive everything from your own code. Linux-only, MIT licensed.

The programs under `demos/` are guidance: complete working examples of how to
build an application on the engine (scene setup, physics, input, audio,
rendering configuration).

## Features

- **Two render backends** — Vulkan (primary) and OpenGL (fallback), chosen at
  runtime with `VORTX_RENDERER=vulkan|opengl`. Vulkan is tried first; the
  engine falls back to OpenGL automatically if Vulkan init fails.
- **Clustered forward lighting** — up to 64 lights per scene (point, spot,
  directional). A screen-space cluster grid culls lights per fragment, with
  smooth windowed falloff, so light-dense scenes stay fast.
- **Shadows** — cascaded shadow maps for the sun (3 practical-split,
  texel-snapped cascades), shadow-mapped spots and points, PCSS soft shadows
  plus screen-space contact shadows at the High quality preset.
- **PBR materials** with glTF 2.0 mesh assets.
- **Bullet physics** — rigid bodies (box, capsule, plane, convex hull,
  triangle mesh), raycasts, and a physics-based first-person controller
  (walk / sprint / crouch / jump).
- **OpenAL audio**.
- **Blender as the scene editor** — `.blend` files are the source of truth,
  exported to `.vgs` text scenes + `.glb` meshes, with an optional
  live-link that hot-applies edits into a running game.
- Scene graph, event system, reflective property system, animation,
  async asset streaming.

## Build

Requirements: CMake, a C++23 compiler, and development packages for
**Bullet, OpenGL/GLEW, OpenAL, Vulkan, glfw3 (≥ 3.3), glslang**.

```bash
mkdir -p build && cd build
cmake ..
make -j"$(nproc)"
make symlink_glfw3   # data symlinks for the demos
```

This builds the `vortxGE` static library, the test suite, the `obj2vbm`
tool, and both demos (`vortxGEdemo_glfw3`, `vortxGEdemo_gtkmm`).

There is no CI by policy — the local build + full test suite gates every
change (see `AGENTS.md` §4).

## Test

```bash
cd build && ctest --output-on-failure
```

83 doctest cases / 1979 assertions, all passing.

## Run the demos

```bash
cd build/demos/vortxGEdemo_glfw3
./vortxGEdemo_glfw3                          # default scene, Vulkan
VORTX_RENDERER=opengl ./vortxGEdemo_glfw3    # OpenGL fallback
VORTX_SCENE=light_stress ./vortxGEdemo_glfw3 # 64-light tech scene
```

Scenes are exported from Blender with:

```bash
cd build && make export_scenes   # re-exports stale .blend → .vgs + .glb
```

See [docs/blender_export.md](docs/blender_export.md).

## Use vortxGE in your game

```cmake
add_subdirectory(vortxGE)
target_link_libraries(yourgame PRIVATE vortxGE)
```

Engine headers are included module-style, e.g.
`#include <scene/lsScene.h>`, `#include <render/lsRender.h>`.
The demos (`demos/shared_code/`) show a complete application skeleton:
window/input via glfw3, scene creation, physics world, main loop.

## Runtime configuration (environment variables)

User-facing:

| Variable | Values | Effect |
|---|---|---|
| `VORTX_RENDERER` | `vulkan` \| `opengl` | Render backend (Vulkan first, GL fallback) |
| `VORTX_SCENE` | scene name | Load `data/<name>.vgs` instead of `demoscene` |
| `VORTX_LEGACY_PIPELINE` | `1` | Old pass-manager render path (default is the stage pipeline) |
| `VORTX_LIVELINK` | `1` | Watch the scene `.vgs`; hot-apply external edits (Blender push) |
| `VORTX_SHADOW_QUALITY` | `low` \| `med` \| `high` | Shadow preset (default `high`: PCSS + contact shadows) |
| `VORTX_CSM` | `1` | Cascaded shadow maps for directional lights |
| `VORTX_SHOT` | path | Write a screenshot at frame 30 (both backends) |
| `VORTX_FIXED_DT` + `VORTX_RAND_SEED` | ms, int | Deterministic mode for repeatable runs |

Debug tooling (many gated behind the `VORTX_DEBUG_TOOLS` build define):
`VORTX_DEBUG_VIEW`, `VORTX_ONLY_LIGHT`, `VORTX_LIGHTS_OFF`,
`VORTX_DISABLE_SHADOWS`, `VORTX_NOCLUSTER`, `VORTX_CLUSTER_TRACE`,
`VORTX_PASS_TRACE`, `VORTX_SHADOW_TRACE`, `VORTX_FRAME_DUMP`,
`VORTX_PROF` (built-in CPU/GPU profiler: `=1` console, `=path.csv`),
`VORTX_DEPTH_SHOT`, `VORTX_FBO_SHOT`, plus per-subsystem `VORTX_DEBUG_*`
toggles and Vulkan-side `VORTX_VK_*` switches.

## Documentation

Start at [docs/index.md](docs/index.md) — engine architecture
([docs/vortxGE.md](docs/vortxGE.md)), every subsystem, the Blender
pipeline, and the active development plan ([Plan.md](Plan.md)).
`AGENTS.md` documents build/test/style conventions for contributors.

## License

MIT — see [LICENSE](LICENSE).
