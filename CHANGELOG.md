# Changelog

All notable changes to vortxGE are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **GPU instancing + terrain vegetation (2026-08-15):** real instanced
  rendering — `lsRender::VertexAttribDivisor` / `DrawElementsInstanced`
  on both backends (GL: `glVertexAttribDivisor`/`glDrawElementsInstanced`;
  Vulkan: divisor recorded per attribute, pipeline builder maps it to
  `VK_VERTEX_INPUT_RATE_INSTANCE`). `lsBatch_Mesh::setInstanceData()`
  packs 8 floats/instance into attrs 5/6 (pos+scale, rotY cos/sin +
  colour variation), `setInstanceDrawCount()` for LOD prefix clamping,
  `setSkipPrepass()`. New instanced shaders `light_tex_none_inst` +
  `shadowmap_inst` (per-instance Ry*scale+translate in the vertex
  shader). Demo terrain vegetation rewritten: 625k grass / 10.8k bush /
  5.1k tree instances in 64 chunk draws per species — one VBO per
  chunk, no per-object scene nodes for individual plants.
- **`lsObject::setMaxDrawDist()` (2026-08-15):** bSphere-aware
  distance culling in `Optimize` (camera-to-surface distance), skipped
  for shadow passes (their camera is the light). Used by the demo for
  per-species veg draw distances (grass 90 m, bush 300 m, tree 800 m).
- **Debug hooks (2026-08-15):** `VORTX_PHASE_TIME=1` prints
  update/render/poll ms per 120 frames; `VORTX_NOVEG`,
  `VORTX_VEG_NOSHADOW`, `VORTX_VEG_NOPREPASS`, `VORTX_VEG_MAXD`,
  `VORTX_GRASS_DIST` isolate vegetation cost/placement.

### Fixed
- **Node thread-per-child perf collapse (2026-08-15):**
  `lsNode::MoveThread`/`OptimizeThread` spawned one `std::thread` per
  child node per pass (~20-50 µs each). The terrain+veg scene pushes
  350+ root children through ~6 thread passes per frame -> 2000+
  spawns ~= 45 ms/frame (14 fps). Both now run serially in the calling
  thread (per-child work is ~1 µs; serial is also race-free — threaded
  `Optimize` concurrently pushed to the unprotected scene draw list).
  Result: 14 -> 59.8 fps (vsync) with full vegetation on GL and
  Vulkan. `VORTX_NODE_THREADS=1` restores the old path for comparison.

### Removed
- **Legacy render path (R6):** `vortxGE/src/passes/` (lsPassManager and
  all pass classes) and `vortxGE/src/controls/` (lsControl,
  lsControlManager, lsComposite, lsViewer) deleted. The stage-based
  pipeline is the only render path. Scene-graph Draw/Optimize take
  `int pass_type` (LSPT_* now in `render/lsRenderTypes.h`); demos own
  an `lsSceneManager` directly.


### Added
- **L5 — clustered forward lighting (2026-08-14):** the fixed 3-light
  limit is gone. Light property uniform arrays raised to 64 in all
  `light_tex_*` shaders; a screen-space cluster grid (32 px tiles ×
  12 exponential depth slices, `lsClusterGrid` + `lsClusterStage`,
  RG32UI/R32UI textures on units 12/13) culls lights per fragment on
  both backends. Smooth windowed falloff `saturate(1-(d/r)⁴)²` with
  `r = sqrt(intensity/0.25)` (`lsLight::clusterRadius`) applies to
  cluster-shaded lights; hero slots 0–2 keep legacy falloff and the
  shadow-caster slots. Vulkan gained integer-sampler support
  (`usampler/isampler` in the shader transform, `R32UI`/`RG32UI`
  uploads) and a `clusterFlipY` sampling-site V-mirror fix. Base scene
  is pixel-identical with clustering on (RMSE 2e-6). Kill switch
  `VORTX_NOCLUSTER=1`, stats via `VORTX_CLUSTER_TRACE=1`.
- **L5.4 — `cast_shadows` light property + light_stress scene
  (2026-08-14):** `.vgs` `add_light` gained an optional trailing
  cast_shadows flag (default on); the pipeline only creates shadow
  stages for casters. Blender custom property `cast_shadows` round-trips
  through the exporter. New `make_scene_light_stress.py` generator +
  committed `light_stress` scene: 64 rainbow point lights, 11 pillars,
  sun — the clustered-lighting stress test (`VORTX_SCENE=light_stress`).
- **Live-link phase 1 (2026-08-14):** `VORTX_LIVELINK=1` makes the demo
  watch the loaded scene's `.vgs` (2 Hz) and hot-apply external edits —
  transforms and light color/intensity re-applied in place by name
  (`lsSceneManager::ApplySceneTransforms`), no scene rebuild, pipeline
  shadow stages keep their light pointers. Structural changes
  (add/remove) still require a restart.
- **L4 — soft shadows (2026-08-12):** PCSS (blocker search + variable
  penumbra PCF) and screen-space contact shadows, enabled at the
  `VORTX_SHADOW_QUALITY=high` preset (the default).
- **L2 — CSM for the sun (2026-08-12):** cascaded shadow maps for
  directional lights — practical split scheme, per-cascade ortho
  fitting, world-anchored texel snapping for stable cascades
  (`lsCsm` math module + `lsCsmStage`), `VORTX_CSM=1`.
- **Networking — OmniNet integration (2026-08-04):** OmniNet
  (gitlab.com/leestripp/omninet — HTTP/WebSocket server library, TLS,
  sessions) is now fetched via CMake FetchContent (pinned commit) and
  linked statically into the vortxGE engine library. Foundation for
  engine-level remote inspection, MCP-over-HTTP debug hooks
  (save_fbo / save_framebuffer and friends), and future multiplayer
  support. Requires OmniNet >= b6e8084 which auto-disables its
  tests/demos/benchmarks/tools when consumed as a dependency.
  Smoke test: `tests/src/net/omninet_smoke.cpp` serves a real HTTP
  round-trip from the doctest runner.
- **V3 — Vulkan textures (2026-08-04):** real texture uploads replace
  the dummy 1×1 white texture. \`Gen/Bind/DeleteTextures\`,
  \`ActiveTexture\`, \`TexImage2D\` (sRGB/UNORM/16-bit formats, RGB→RGBA
  expansion on upload, NULL-data render targets for V4),
  \`TexParameter[i/f/fv]\` (wrap/filter state → recreated samplers),
  \`GenerateMipmap\` (blit chain over the full mip pyramid), GL texture
  units → per-draw descriptor updates (\`Uniform1i\` sampler assignments
  recorded per program; only changed bindings rewritten). Demo scene
  now renders textured on Vulkan; zero validation complaints.
- **V2 — Vulkan buffers, shaders, pipelines, first geometry (2026-08-04):**
  scene geometry now renders through \`lsVulkan\`. Buffer/VAO/shader/
  program/uniform/draw API surface implemented: host-visible coherent
  \`VkBuffer\`s, VAO attribute-layout capture, GLSL→SPIR-V at link time
  via glslang (loose GL uniforms source-transformed into a shared
  std140 block at set=0/binding=0 with identical offsets in both
  stages; samplers auto-bound at set=0/binding=1+K, sampler arrays
  supported), per-draw dynamic-UBO upload ring, cached \`VkPipeline\`s
  keyed by (program, VAO, topology, GL state) with depth test/cull/
  blend baked from the GL-style state cache. Window render pass gained
  a depth attachment; Y-flip via negative viewport height. Samplers
  bind a dummy 1×1 white texture until V3. Requires \`glslang-dev\` +
  \`spirv-tools-dev\` (apt). GL fallback unchanged; suite green; zero
  validation-layer complaints on RADV.
- **V1 — Vulkan windowed rendering core (2026-08-04):** `lsVulkan`
  is now the PRIMARY renderer when available: instance (+debug
  validation in Debug builds), GLFW surface, device/queues, swapchain
  with recreation on resize/out-of-date, clear-colour render pass,
  command buffers, 2-frames-in-flight sync, `Clear`/`Present`.
  `vortxGE::PreferredRendererType()` probes Vulkan before window
  creation so apps create the right window type (GLFW_NO_API vs GL
  context); `ForceRenderer`/`SetWindow` added; `lsRenderConstants.h`
  now uses numeric literals (99 constants) — no GL header leaks into
  the Vulkan TU. GLFW demo runs on Vulkan by default
  (`VORTX_RENDERER=opengl` selects GL), GtkGLArea demo forces OpenGL.
  Offscreen/shadow passes fail gracefully (V2–V4 stubs).
- **V0 — renderer interface refactor for dual backends (2026-08-04):**
  `lsRender` gains frame-lifecycle hooks (`SetWindow`, `BeginFrame`,
  `EndFrame`, `Present`, `Resize`) and `Init()` now returns
  `bool` for fallback signalling; `lsOpenGL` implements them as
  no-op shims (GL context/swap stay app-owned); new `lsVulkan`
  skeleton (80 stubs) compiles in-tree; `vortxGE` selects the backend
  via `VORTX_RENDERER=vulkan|opengl` (default: try Vulkan, fall back
  to OpenGL); `vortxGE::Render` wraps passes with the frame hooks.
- **Vulkan primary renderer — plan approved (2026-08-04):** Plan.md
  rewritten; completed history condensed to Part 1. New Part 2: Vulkan
  becomes the main renderer with OpenGL as fallback (user decision),
  phases V0 (interface refactor + backend selection) through V6 (full
  parity). Key design: `lsOpenGL` untouched as stable fallback, `LS_*`
  constants keep GL values (Vulkan translates at its boundary), frame
  lifecycle hooks enter `lsRender`, `VORTX_RENDERER` env override.

## [0.2.0] - 2026-08-03

### Added
- **Phase 5 (partial) — beyond-math test coverage:** new doctest suites
  for `lsEventManager` (queue FIFO, dispatch, ownership semantics) and
  `lsProperty_Float` / `lsProperty_Vector3` (reflective set/get,
  toString/fromString round-trips). `lsEventManager::Poll_Event`
  ownership contract now explicitly documented (manager deletes the
  previously polled event; callers must not).
- This CHANGELOG.md (Keep a Changelog format).
- **Legacy .vbm format removed (user decision):** deleted
  `lsMesh::LoadVBM` (~750 lines), the vbm dispatch branch in
  `lsAsset_Mesh::AllocAsset` (glTF/glb is now the only mesh path), and
  the `obj2vbm` tool. Verified: full build clean, 52 test cases /
  1737 assertions green, 60 s demo smoke with all 9 `.glb` meshes
  loading. glTF is the sole mesh pipeline going forward.
- **Phase 7 M5 — async parse→upload pipeline:** streaming loads no
  longer hitch the GL thread. `lsAsyncFileLoader::Queue` (generic worker
  tasks); `lsMesh::UploadGLTF` extracted from LoadGLTF (GL upload from
  parsed data); `lsAsset::AllocAsset_Async`/`AsyncPoll` virtuals (sync
  fallback by default); `lsAsset_Mesh` async impl (worker: LoadFile +
  ParseGLTF → GL thread: UploadGLTF); `lsStreamingManager` LOADING
  state + `setLoader` (polls AsyncPoll in update; no double-queue;
  mid-flight entries never evicted — an out-of-range completion evicts
  in the same frame, pinned). DemoScene runs a 2-worker pool. 1 new
  test case (12 assertions: async lifecycle, mid-flight protection,
  sync fallback unchanged). Verified: 63 cases / 1874 assertions green,
  60 s demo smoke shows the full async cycle live.
- **Phase 7 M4 — demo streaming integration:** DemoScene walks the
  loaded scene graph and registers every mesh-bearing lsObject (skybox
  excluded) with an `lsStreamingManager` (load < 50, evict > 80, budget
  16). The manager logs INFO lines on load/evict. Verified live in the
  60 s smoke: 9 objects registered; Cube.003 (moving physics object)
  cycles `Stream load`/`Stream evict` as it crosses the radii — real
  distance-driven streaming, hysteresis stable, no crashes. Phase 7
  (M1–M4) complete.
- **Phase 7 M3 — node-driven asset residency:** new
  `lsStreamingManager` (assets/) — registers (lsNode, lsAsset) pairs and
  drives residency per frame from `lsScene::Optimize` (camera position
  via the ModelMatrix translation column, same pattern as the light
  manager). Hysteresis (load < loadRadius / evict > unloadRadius),
  budget cap with farthest-first eviction, and ref-aware protection
  (assets with users above their registration baseline are never
  evicted). Eviction is FreeAsset() in place — nodes keep their
  reference and Draw's getData() null-check simply skips evicted
  objects. `lsScene::setStreamingManager` hook (nullptr = disabled,
  zero overhead). 5 new test cases (42 assertions) with mocked
  assets/nodes: registration, distance load/evict, hysteresis
  stability, ref-aware block/release, budget ordering. Verified: 62
  cases / 1854 assertions green, 60 s demo smoke clean.
- **Phase 7 M2 — glTF CPU-parse/GL-upload split:** new
  `lsGLTFParser.{h,cpp}` (datablocks/) — GL-free
  `ParseGLTF(bytes, isGlb, baseDir) -> expected<lsGLTFMeshData, string>`
  via tinygltf from memory. Extracts POSITION/NORMAL/TEXCOORD_0/TANGENT,
  u8/u16/u32 indices, PBR factors (albedo/roughness/metallic), texture
  refs (URI → urldecoded baseDir-joined path; embedded → decoded pixels +
  layout) and the mesh AABB. Replicates + test-pins the legacy **1-based
  index quirk** (+1 per index) so the upload side feeds `Add_Index`
  directly. `lsMesh::LoadGLTF` rewired onto it (251 → 153 lines; reads
  bytes via `lsAsyncFileLoader::LoadFile`, keeps GL-side batch/texture/
  shader/bounding-sphere logic; dead urldecode helper removed).
  5 new test cases (74 assertions): all 8 demo .glb parse, Suzanne
  sanity, exact ASCII-triangle data, URI/urldecode pinning, error paths.
  Verified: 57 cases / 1812 assertions green, 60 s demo smoke — all 9
  `.glb` meshes load through the new split.
- **Phase 7 M1 — async file loader:** new `lsAsyncFileLoader`
  (assets/): worker-thread pool, `std::future` +
  `std::expected<std::vector<std::byte>, std::string>` results,
  drain-on-destruction. First C++23 `std::expected` adoption (Phase 6
  step 5). 4 test cases (52 cases / 1737 assertions total). Docs:
  `docs/assets/AsyncFileLoader.md`; `docs/index.md` created as the
  documentation catalog.
- **Deferred test-suite findings fixed (48 cases / 1710 assertions):**
  `lsVector3::normalize()` now has a zero-length guard (was div-by-zero
  → NaN), matching lsVector4/lsQuaternion; `lsQuaternion::inverse()` is
  now the true inverse (conjugate/|q|²) — correct for non-unit
  quaternions; `lsRandom::SetSeed()` added for reproducible sequences
  (documents global rand() state caveat); stale `lsNode` ctor doc
  comment corrected (no auto-registration). Regression tests added for
  each.
- **C++23 migration (Plan.md Phase 6):** `CMAKE_CXX_STANDARD` 20 → 23
  (GCC 15.2). Full rebuild clean, **47 test cases / 1692 assertions
  green**, 60 s GLFW demo smoke passed. Fixed the only new C++23
  warnings: deprecated literal-operator space in vendored
  `tinygltf/json.hpp` (upstream-style `operator""_json` fix). Feature
  adoption (std::expected, std::print, …) proceeds incrementally.
- **Beyond-math tests (lsNode):** first non-math suite
  (`tests/src/scene/lsNode_tests.cpp`, 7 cases / 1692 total assertions):
  ctor defaults, TRS setters, active/deletion flags, parent-child
  hierarchy, and `RotateWorld` (degrees; parent-relative path exercises
  the transpose fix). **Findings:** lsNode ctor doc comment claims
  auto-registration with parent but doesn't do it (stale docs);
  `Move/Optimize/Draw`/`ModelMatrix` composition untestable without a
  scene/render pass (documented scope limit). tests target now links
  OpenAL (lsListener dependency).
- **Math tests Phase 4 (remaining math — PLAN COMPLETE):** 11 more
  test cases (**40 cases / 1655 assertions total**): `lsMatrix3x3`
  (transpose/inverse/extract from 4x4), `lsPlane` (signed distance,
  3 construction paths), `lsFrustum` (public API — finding: no public
  point-in-frustum query; `distance()` is radial from camera),
  `lsBoundingSphere`, `lsMath` constants, `lsRandom` (ranges; finding:
  no seed API, non-deterministic default seeding). All 11 math classes
  covered.
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

### Fixed
- **`lsProperty_Float`/`lsProperty_Vector3::fromString` silently
  discarded toString-formatted input:** the parsers only accepted bare
  numbers, so a `toString()` → `fromString()` round-trip (the
  serialization contract) was a no-op. Both now strip `( ) ,`
  decoration before parsing; bare formats still accepted.
- **Shutdown crash (use-after-free):** `~lsScene` deleted the Bullet
  world before its child nodes were destroyed (base-class member
  destruction runs after the derived-class body), so `lsBulletObject`
  destructors called `removeRigidBody()` on a dead world — segfault on
  every clean exit. `~lsScene` now clears its child nodes first.
- **Streaming load/evict flapping:** the M3 residency hook ran inside
  `lsScene::Optimize()`, which passes invoke per-pass with the active
  cull camera — including shadow-pass light cameras — so the streaming
  manager saw alternating camera positions each frame and flapped
  load/evict decisions. Residency is now driven by the app once per
  frame from the main thread with the viewer camera
  (`DemoScene::Update`); the `lsScene::setStreamingManager` hook was
  removed. Exposed by testing with a 10/20 load/evict radius — the
  shipped 50/80 config had kept light-camera distances inside the
  hysteresis dead zone, hiding the bug.

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

[Unreleased]: https://gitlab.com/leestripp/vortxGE/-/compare/0.2.0...main
[0.2.0]: https://gitlab.com/leestripp/vortxGE/-/compare/0.1.0...0.2.0
[0.1.0]: https://gitlab.com/leestripp/vortxGE/-/tags/0.1.0

## 2026-08-15 — Vegetation system repair + lsFlyCam

### Fixed
- **"One patch" vegetation bug (two root causes):**
  1. `classifyBiome` hard-coded sea level 0.30 — the all-land demo map's
     walkable lowlands classified as Ocean/Beach and every veg mask
     excluded them. `classifyBiome` gained a `seaLevel` parameter
     (default 0.30, tests unchanged); the demo passes 0.08. Biome
     histogram now: Grassland 46%, Forest 45%, Taiga 5%.
  2. Per-chunk cap used prefix `resize()` — kept scan-order prefix, so
     capped instances clustered in one corner of every chunk. Replaced
     with deterministic hash-thinning (spatially uniform).
- Veg altitude rules widened (0.05 min) — biome mask gates placement.
- Grass invisible against terrain: brighter albedo (0.42,0.66,0.20) +
  1.6x clump scale; bush/tree caps raised (400/250) + brighter albedo.

### Added
- **lsFlyCam** (`vortxGE/src/scene/lsFlyCam.{h,cpp}`): free-fly survey
  camera — WASD+Space/Ctrl flight (sprint x4), mouse look, scripted
  paths for automated captures: `VORTX_FLYCAM=1`,
  `VORTX_FLYPATH=orbit:cx,cy,cz,radius,period[,pitch]` or
  `tour:x0,y0,z0;x1,y1,z1;...`, `VORTX_FLYSPEED` m/s.
- **Vegetation LOD**: per-chunk instance thinning by camera distance
  (grass 100/50/25% across its 90 m range via `setInstanceDrawCount`),
  on top of the existing per-chunk frustum culling + maxDrawDist
  (grass 90 m, bush 300 m, tree 800 m).
- Debug: `VORTX_VEG_DEBUG=1` per-chunk scatter dump, biome histogram
  at startup, `VORTX_GRASS_SCALE` test hook.

Verified: ground-level captures show grass + bushes + trees across the
whole map (was: one foothill patch), 59.6 fps at vsync cap, ctest 100%.
Known polish item: bush shade sides read nearly black (dark albedo +
low ambient) — lift ambient or albedo if it bothers in play.

## 2026-08-15 (2) — Vegetation asset scale/orientation + flycam toggle

### Fixed
- **Veg meshes were lying flat**: grass_clump/bush/broadleaf OBJs were
  authored growing along -Z (Blender export without Y-up conversion) —
  tree trunk ran 0 -> -3.2 in Z, so "trees" were 3.2 m horizontal
  patches on the ground. Meshes are now rotated +90 deg about X at
  load (positions, normals, AABB) — trees stand upright with trunks.
- **Asset scales corrected to 1 unit = 1 m**: per-species `scaleMul`
  (target world size / mesh size) in the demo veg table — grass
  0.34-0.50 m tufts, bushes 1.1-1.7 m, trees 7-11 m tall (was:
  waist-high 1.4 m grass, 2 m trees). Per-instance jitter (0.8-1.2)
  on top. Trees thinned (9 m spacing, cap 200) so canopies don't
  close into darkness.
- **lsFirstPersonRig no longer hijacked**: flycam input routing is now
  keyed on an explicit `m_flyCamActive` flag instead of mere flycam
  existence; default OFF, rig path untouched. Fixed the static
  `s_flyCam` that survived scene reloads (dangling on 2nd run).

### Changed
- **Flycam is now a runtime toggle**: F6 switches rig <-> flycam live
  (noclip-style: starts at the current camera position, WASD + Space/
  Ctrl + Shift sprint, mouse look; F6 again returns to the rig).
  VORTX_FLYCAM=1 still forces it on at startup for scripted captures
  (same VORTX_CAM_POS/YAW/PITCH, VORTX_FLYPATH orbit/tour envs).
  gtkmm frontend: F6 binding not added yet (glfw3 only).

Verified: normal mode rig camera at spawn responding to input path;
flycam mode at exact requested pose; orbit path moves; ctest 100%.
