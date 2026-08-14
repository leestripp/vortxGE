# Procedural Sky & Day/Night Cycle (lsSky)

## Overview

`lsSky` (`vortxGE/src/scene/lsSky.{h,cpp}`) is the engine's procedural
atmosphere: a time-of-day state machine that computes sun/moon
directions, gradient colors and star intensity in pure C++ (unit
tested), hands them to the `sky` shader as uniforms, and drives the
scene's directional light so the world lighting matches the sky.

It replaced the static HDRI skybox (2026-08-14). The sky sphere mesh
is **generated in code** — there is no `SkySphere.glb` dependency and
no texture-skybox fallback path.

## Pieces

- **`lsSky::computeParams()`** — pure function: time-of-day →
  `lsSkyParams`. Parametric solar path (rise 06:00 east, set 18:00
  west, tilted 23.5°), sun/moon directions, zenith/horizon/ground
  gradient colors, sun color+intensity, `dayFactor`, `starAmount`,
  `sunsetGlow`. Covered by doctests (`tests/src/scene/lsSky_tests.cpp`).
- **Shader** — `demos/data/shaders/sky.{vert,frag}`,
  `ShaderKind::Sky`. Vertex stage: rotation-only view matrix +
  clip-space `z = w` (far plane) — the classic skybox trick, so the
  sky is camera-centric and never needs repositioning. Fragment stage:
  3-stop atmosphere gradient, sun disc + halo, moon disc, procedural
  cell-hash stars. Uniforms go through the generic render API, so GL
  and Vulkan behave identically.
- **Geometry** — `lsSky::buildSphereMesh()`: UV sphere, radius 100,
  positions only (the shader derives view direction from the vertex
  position, so no normals/UVs are needed), 1-based indices (the
  engine's pinned LoadGLTF index quirk). Radius 100 is far inside the
  camera's 1000 far plane; combined with the rotation-only view the
  sphere always covers the view regardless of camera position.
- **Scene integration** — `lsSky::init()` registers the sky object via
  `lsScene::setSkyObject()` (drawn first, `LS_LEQUAL`, no depth write,
  cull off — see `lsScene::Draw`). `attachSunLight(lsLight_Direct*)`
  drives the sun: warm directional light by day, faint blue
  "moonlight" (direction = −sunDir) at night.

## Usage

```cpp
lsSky* sky = new lsSky();
sky->setTimeOfDay( 10.0f );          // hours [0, 24)
sky->setDayLength( 60.0f );          // seconds per 24h; 0 = frozen
sky->init( scene, shaderManager, render );
sky->attachSunLight( sunLight );     // optional
// per frame:
sky->update( deltaSeconds );
sky->apply();
```

## Demo env vars (demoscene)

| Var | Default | Effect |
|---|---|---|
| `VORTX_TIME_OF_DAY` | `10` | Start time, hours [0, 24) |
| `VORTX_DAY_LENGTH` | `0` | Seconds per 24h cycle; `0` = frozen (capture-deterministic) |

## Caveats

- The sky object is **not** part of the scene traversal; it is drawn
  directly by `lsScene::Draw` and never frustum-culled.
- On Vulkan, the sky's uniforms live in the per-frame UBO ring like
  every other draw — because the sky draws first, a ring overflow hits
  it first. See
  [renderer/VulkanBackend.md](../renderer/VulkanBackend.md) for the
  2026-08-14 black-sky incident.
