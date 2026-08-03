# vortxGE Documentation Index

Start here. Everything below is one click away — topic docs live in
this directory, project-level docs live at the repo root.

> **Convention (since 2026-08-03):** docs are updated *as we go* —
> every significant change lands with its doc update, and new docs get
> linked from this index.

## Quick start

| I want to… | Read |
|---|---|
| Build, run tests, run demos | [AGENTS.md §4](../AGENTS.md) |
| Understand the engine architecture | [vortxGE.md](vortxGE.md) |
| See what's planned / in progress | [Plan.md](../Plan.md) |
| See what changed recently | [CHANGELOG.md](../CHANGELOG.md) |

## Project root documents

| File | What it is |
|---|---|
| [Readme.md](../Readme.md) | Project intro, features, build instructions |
| [AGENTS.md](../AGENTS.md) | Guide for AI coding agents: build/test commands, code style, repo layout |
| [Plan.md](../Plan.md) | Active plan: test suite (done), C++23 (done), **Phase 7 streaming Asset Manager** |
| [CHANGELOG.md](../CHANGELOG.md) | Versioned change log ([Unreleased] on top) |

## Engine topics (docs/)

| Area | Doc | Covers |
|---|---|---|
| Overview | [vortxGE.md](vortxGE.md) | Engine architecture overview |
| Assets | [assets/AssetManagement.md](assets/AssetManagement.md) | Asset system (→ being reworked: Plan.md Phase 7) |
| Audio | [audio/AudioSystem.md](audio/AudioSystem.md) | OpenAL audio |
| Controls | [controls/ControlsSystem.md](controls/ControlsSystem.md) | Input/controls mapping |
| Datablocks | [datablocks/DataBlocks.md](datablocks/DataBlocks.md) | File formats & data loaders |
| Demos | [demos/ApplicationDevelopment.md](demos/ApplicationDevelopment.md) | Writing apps on the engine |
| Events | [events/EventSystem.md](events/EventSystem.md) | Event bus / dispatch |
| Math | [math/MathLibrary.md](math/MathLibrary.md) | Vectors, matrices, quaternions (✅ fully unit-tested) |
| Rendering | [passes/RenderingPipeline.md](passes/RenderingPipeline.md) | Render passes / pipeline |
| Physics | [physics/PhysicsSystem.md](physics/PhysicsSystem.md) | Bullet physics integration |
| Properties | [properties/PropertiesSystem.md](properties/PropertiesSystem.md) | Reflective property system |
| Scene | [scene/SceneGraph.md](scene/SceneGraph.md) | lsNode scene graph |
| Scene | [scene/SceneLoader.md](scene/SceneLoader.md) | .vgs scene files |
| Scene | [scene/FirstPersonRig.md](scene/FirstPersonRig.md) | FPS camera rig |
| Shaders | [shader/ShaderSystem.md](shader/ShaderSystem.md) | Shader management |
| Animation | [animation/Readme.md](animation/Readme.md) | Animation notes |
| Utils | [utils/Utilities.md](utils/Utilities.md) | Misc helpers (timers, etc.) |

## Reference / notes

| Doc | Topic |
|---|---|
| [blender_export.md](blender_export.md) | Blender → engine asset export |
| [glfw_desktop_scale_factor.md](glfw_desktop_scale_factor.md) | HiDPI / desktop scaling handling |
| [plans/renderer_abstraction.md](plans/renderer_abstraction.md) | Renderer abstraction plan (lsOpenGL/lsVulkan) |

## Testing

The test suite (doctest + CTest, `tests/`) is documented in
[AGENTS.md §4](../AGENTS.md) and planned in [Plan.md](../Plan.md):
48 test cases / 1710 assertions, run with `ctest --output-on-failure`.
