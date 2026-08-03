# Changelog

All notable changes to vortxGE are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- This CHANGELOG.md (Keep a Changelog format).
- **Math tests Phase 3 (lsQuaternion):** 7 test cases (413 total
  assertions): degrees axis-angle construction, magnitude/normalize
  (zero-guard), conjugate/inverse (unit-quat `q*q⁻¹=identity`),
  composition + non-commutativity, Slerp endpoints/halfway (45° at
  t=0.5), Lerp, dot, `toMatrix4x4` orthonormality + consistency with
  `RotateMatrix` + internal normalisation. Finding pinned:
  `inverse()==conjugate()` is only valid for unit quaternions.
- **Math tests Phase 2 (lsMatrix4x4 full coverage):** 12 more test
  cases (348 total assertions): transform composition (post-multiply),
  degrees-based `RotateMatrix`/`PerspectiveProjection`, GL-form
  perspective/ortho NDC invariants, `LookAt` orthonormality,
  `extractRotation`, `Set(quaternion)` cross-check,
  `getLocalTransformFromParent` round-trip, float[16] operators.
  **BUG FIXED:** `lsMatrix4x4::transpose()` corrupted the matrix when
  source aliased destination (`m.transpose(m)`, used by
  `getLocalTransformFromParent`) — now alias-safe via a source copy,
  with a regression test. Findings pinned: `ToEulerZYX` returns degrees;
  `lsQuaternion(axis, angle)` takes degrees.
- **Math tests Phase 1 (lsVector2/3/4):** 6 more test cases (228 total
  assertions) pinning real engine semantics discovered by the suite:
  `lsVector3::operator*(vec)` is intentionally the cross product;
  `lsVector4` is homogeneous (ops act on xyz, preserve w; 3D
  length/distance). **Finding:** `lsVector3::normalize()` lacks the
  zero-length guard that `lsVector4::normalize()` has (div-by-zero →
  NaN) — engine fix deferred per Plan.md §7.
- **Real test suite + CTest (Plan.md Phase 0):** doctest 2.4.12 vendored
  (`tests/third_party/doctest/`), new doctest entry point
  `tests/src/test_main.cpp`, `enable_testing()` + `add_test()` wiring,
  and the first assertion-based suites for `lsMatrix4x4` (transpose,
  identity, operator* — 4 test cases / 71 assertions), converted from
  the old visual-debug `tests/main.cpp`. Run with
  `ctest --output-on-failure`.
- **`lsVulkan` placeholder backend** (`vortxGE/src/render/lsVulkan.{h,cpp}`):
  a skeleton subclass of `lsRender` overriding all 64 pure-virtual
  methods with no-op stubs, as a scaffold/reminder for the future Vulkan
  backend (recipe in `docs/plans/renderer_abstraction.md` §10). The constructor prints a warning
  if instantiated; `RenderType::Vulkan` already existed in
  `lsRenderTypes.h`.
- **Tag-driven versioning:** the project version is now derived from the
  latest git tag at CMake configure time. `config.h` is generated from
  `vortxGE/src/config.h.in` into the build tree (`build/generated/`) and
  additionally exposes `vortxGE_VERSION_STRING` and
  `vortxGE_VERSION_GIT`. Tarball builds without git metadata fall back to
  the version pinned in the root `CMakeLists.txt`. To cut a release,
  create a semver git tag and re-run CMake — no code edits needed.

### Changed
- **`Plan.md` re-purposed:** the completed renderer-abstraction plan
  moved to `docs/plans/renderer_abstraction.md`; `Plan.md` now holds the active plan
  (real test suite + CTest, Phases 0-5).
- Synced docs with the post-abstraction API:
  `docs/datablocks/DataBlocks.md` (engine handle types `lsTextureID` /
  `lsBufferID` / `lsVAOID` / `lsFBOID`, real `lsFBO` / `lsTexture`
  interfaces), `docs/scene/SceneGraph.md` (shadow-map handle type),
  `AGENTS.md` (removed stale `src/opengl/` path), `Readme.md`
  (renderer-abstraction feature bullet, new Documentation section).

### Removed
- Hardcoded `vortxGE/src/config.h` (pinned at 0.7.2) — replaced by the
  generated header.
- Broken `screenshots/vortxGE.png` image reference in `Readme.md`
  (the `screenshots/` directory no longer exists).

## [0.1.0] - 2026-08-02

First tagged release. `0.1.0` marks the completion of the renderer
abstraction refactor (Plan.md Phases 0-8) and captures the full feature
set developed since the project's first public commit (2023-10-13).

> **Note on engine version:** at the time of this tag the version macros
> in the old static `vortxGE/src/config.h` still read **0.7.2**. This was
> resolved after the tag — see `[Unreleased]`: versioning is now derived
> from git tags automatically.

### Renderer

- **Renderer abstraction (Phases 0-8, complete):** `lsOpenGL.cpp` is now
  the only translation unit containing raw `gl*()` calls. All engine code
  talks to the GPU through `lsRender` (75 pure-virtual methods) using
  engine-level handle types (`lsTextureID`, `lsShaderID`, `lsProgramID`,
  `lsFBOID`, `lsBufferID`, `lsVAOID`, `lsQueryID`, `lsUniformID`, …) and
  engine-level `LS_*` constants. 289 raw GL calls and ~250 `GL_*`
  references removed from 16 non-renderer files. See
  `docs/plans/renderer_abstraction.md` §10 for
  the full metrics.
- **HiDPI / desktop-scale fix:** the output quad now fills the window at
  125% (and any other) OS scale factor. FBOs gained engine-level
  `Resize()`; the GLFW demo uses framebuffer-size callbacks and a
  content-scale multiplier. Documented in
  `docs/glfw_desktop_scale_factor.md`.
- **PBR pipeline:** Cook-Torrance BRDF with albedo, normal, and
  metallic/roughness/AO maps.
- **Multi-light shadow mapping:** dynamic shadow maps for spot lights;
  shadow maps are skipped for inactive lights.
- **OpenGL 4.3+ core profile** rendering with pass-based architecture
  (direct, FBO, occlusion, shadow-map passes).

### Scene & Gameplay

- Hierarchical scene graph (`lsNode`) with Objects, Point/Spot lights,
  Cameras, Emitters, and first-person rigs.
- Right-handed coordinate system, view along **-Z** (changed to fix
  shadow/lighting handedness issues).
- **First-person rig:** physics-based character controller with walking,
  sprinting, lerped crouching, and jumping.
- **Object pickup system** with physics sleep fixes.
- **Object emitter** (`lsObjectEmitter`) with data-race fix on
  `m_children` in `MoveThread`.
- Light activation/deactivation support.
- SkySphere skybox.

### Physics

- Bullet Physics integration: rigid bodies with Box, Capsule, Plane,
  Convex Hull, and Triangle Mesh shapes.
- Friction parameters exported per-object from Blender.
- Bullet-only collision shapes exported from Blender and loadable
  without render meshes.

### Assets & Tooling

- **glTF / .glb** model loading via bundled `tinygltf` (with
  `stb_image`).
- **Blender addon** (`blender_scripts/export_scene.py`) exporting `.vgs`
  scene files plus per-mesh `.glb` files, with an object settings panel
  (physics shapes, friction, etc.).
- `obj2vbm` OBJ-to-mesh converter tool restored and maintained.
- Asset manager with mesh, texture, quad, mesh-guide, and bullet-shape
  asset types.

### Audio

- **OpenAL** 3D audio support (`lsOpenAL`).

### Animation

- Keyframe animation for node transforms and material properties with
  cubic-curve interpolation (`lsAnimProperty*`, `lsCubicCurve`).

### Demos

- **GLFW3 demo:** standalone app with FPS controller, fullscreen toggle
  (F1), mouse look, HiDPI handling.
- **GTKmm 4.0 demo:** engine embedded as a `Gtk::GLArea` widget.
- (The old SDL2 demo was removed.)

### Documentation

- `AGENTS.md`: full guide for AI coding agents (architecture, build,
  conventions, renderer abstraction §5.9).
- `Plan.md`: renderer-abstraction plan, now completed (§10 Final State).
- `docs/`: per-subsystem architecture notes (scene graph, rendering
  pipeline, physics, shaders, assets, animation, audio, controls,
  events, properties, math, utilities, demo application development).
- Comprehensive Doxygen comments across public headers.

### Build & Project

- Requires **C++20** and **CMake ≥ 3.29**.
- License changed to **MIT** (© 2025 Lee A. Stripp).
- Removed obsolete `CodeMax.md` and `screenshots/`; `.daedalus/` added
  to `.gitignore`.
- Stray threads are now joined at their correct locations instead of at
  the end of `Move`/`Optimise`.

[Unreleased]: https://gitlab.com/leestripp/vortxGE/-/compare/0.1.0...main
[0.1.0]: https://gitlab.com/leestripp/vortxGE/-/tags/0.1.0
