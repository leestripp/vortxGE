# Plan.md — vortxGE development plan

> **Status 2026-08-14:** the lighting & shadow refactor (L0–L5) and
> debug tooling D0–D2 are **COMPLETE**; Vulkan is the primary renderer
> with GL fallback; the docs overhaul landed (Readme.md + docs/
> refresh, 5e9c4a2). The active plan is now **Part 2 — E-series:
> engine hardening & next features**, below.
>
> Framing (user, 2026-08-14): **vortxGE is a static library other
> developers build games with; the demos are guidance.** Priorities are
> ranked by what would bite — or help — a game developer, not by demo
> visuals.
>
> Completed work (renderer abstraction, Vulkan backend, R1–R4 pipeline
> stages, L0–L5 lighting, D0–D2 tooling) is archived at the bottom of
> this file; full detail in `CHANGELOG.md` and git history.

---

## Part 2 — E-series: engine hardening & next features

### E1 — Stability & correctness (bugs a game would hit)

- **E1.1 Progressive slowdown — physics body leak. ✅ CLOSED 2026-08-14**
  Already fixed by the emitter object pool (7f9cfd6, 2026-08-04 — same
  day as the report): PreAllocate creates 100 parked objects+bodies
  upfront, EmitObject reuses them ring-buffer style, no per-spawn
  allocation or deletion. Full ownership chain audited: checkLife →
  lsEvent_DelNode → MarkForDeletion → CleanupDeletedNodes (root sweep
  per Move) → unique_ptr → ~lsObject deletes bulletobject →
  ~lsBulletObject removes the body from the dynamics world.
  Verified live: bodies=106 stable across 3572 frames / ~47 spawn
  cycles, fps ~41 steady (VORTX_PHYSICS_COUNT=1 probe, kept in the
  demo as guidance — 5s cadence, bodies+fps).
  Remaining: user soak test (original report was over minutes).

  Symptom (user report 2026-08-04): demo starts fine, gets slower and
  slower. Prime suspect: `lsObjectEmitter` spawns `lsBulletObject`
  rigid bodies on a timer; objects die via `lsObject::checkLife`
  (lsObject.cpp ~line 122) — **verify whether checkLife removes the
  Bullet rigid body from the dynamics world.** If not, bodies
  accumulate → broadphase grows → slowdown.
  - Diagnose: log `btDiscreteDynamicsWorld::getNumCollisionObjects()`
    over time with `VORTX_FIXED_DT` off (emitter active); expect growth
    if leaking.
  - Fix: remove + delete the rigid body on object death (ownership
    audit: who owns the `btRigidBody` / motion state / shape).
  - Regression test: doctest or scripted probe asserting body count
    returns to baseline after spawn→death cycles.
- **E1.2 VK "Failed to create shadow pass" ×64 — ✅ FIXED 2026-08-14.**
  Root cause was NOT Vulkan: demo `setupShadows()` iterated ALL
  lights but only created passes for DIRECT/SPOT, printing a false
  "ERROR" for every point light (64 on the stress scene). Also ran
  unconditionally in pipeline mode, double-creating shadow FBOs
  alongside the pipeline stages. Fix: setupShadows now skips
  non-DIRECT/SPOT and non-caster lights silently, and the call site
  is skipped entirely in pipeline mode (stages replace it). Verified:
  VK stress scene 0 errors (was 64); GL legacy base scene still
  creates its 3 passes; ctest 100%.
- **E1.3 VK swapchain subregion — ✅ RESOLVED 2026-08-14 (prior work).**
  Pipeline-mode VK capture at 2400×1350 fills the full frame — no
  black bands. Fixed along the way by the self-healing framebuffer
  poll (cfb92a7) + pipeline blit path. Side discovery: the LEGACY
  pass-manager path on Vulkan renders clear color only (never fully
  implemented — V2–V4 stubs era; all VK work landed on the pipeline
  path). Not a regression; the legacy path is retired by E3.
- **E1.4 Intermittent VK teardown segfault on SIGTERM** — 🟡 MONITORED
  2026-08-14. Seen once, never reproduced despite repeated SIGTERM
  soak runs during the C-series orbit/capture sessions (dozens of
  clean exits both backends). No stack available. Standing rule: if a
  second sighting produces a core/stack, fix immediately; until then
  it stays on the soak list and does not block.

**Done when:** body count stable across spawn cycles; stress scene
runs on VK with zero shadow-pass errors; VK captures fill the full
swapchain; ctest 100%.

### E2 — D3: built-in CPU/GPU profiler — ✅ DONE 2026-08-14

For a library whose demos are guidance, an in-engine profiler is what
developers tune their own games with — and it unblocks measuring
E1/E3/E5.

- D3.1 `lsProfiler` scoped zones (`lsProfileZone` RAII) over frame
  phases: Update, Shadow pass, Scene pass, Blit/present.
- D3.2 GPU timings: Vulkan timestamp queries around each pipeline
  stage (pool of query pairs, 1-frame latency); GL:
  `GL_EXT_timer_query` where available, CPU-only otherwise.
- D3.3 Output: rolling 120-frame summary to console every N frames
  (`VORTX_PROF=1`) or CSV (`VORTX_PROF=path.csv`); per-stage avg/max ms.
- D-series release-bloat rule applies: compiled out in Release
  (`VORTX_DEBUG_TOOLS`), prove with strings/size check.

**DONE:** `lsProfiler` (debug/lsProfiler.{h,cpp}, whole TU behind
`VORTX_DEBUG_TOOLS`) + `VORTX_PROFILE_ZONE` RAII macro (vanishes in
Release). `lsRender` gained debug-only virtuals
`profilerFrameBegin/GpuBegin/GpuEnd`; pipeline `render()` zones every
stage. VK: per-frame-slot timestamp query pools (2-frame latency,
fence-guaranteed; bug fixed: slot high-water mark must persist across
frames). GL: GL_TIMESTAMP query pairs, non-blocking next-frame poll.
`VORTX_PROF=1` console table every 120 frames / `=path.csv` per-frame
rows. Verified both backends (VK main-scene GPU 2.9ms, GL 3.3ms);
Release lib contains no profiler strings (only the empty TU filename);
ctest 100%. Demo also gained `update` zone as usage guidance.

### E3 — Pipeline by default (R3/R5/R6)

The stage-based pipeline is proven (L1–L5 all landed on it) but still
opt-in (`VORTX_PIPELINE=1`). A library should not ship two render
paths indefinitely.

- E3.1 **R3: pipeline is the default** demo path ✅ 2026-08-14 —
  `initPipeline()` runs unless `VORTX_LEGACY_PIPELINE=1`; verified
  GL+VK full-scene captures with zero env vars, legacy escape honored,
  ctest 100%.
- E3.2 **R5: camera decoupling** ✅ 2026-08-14 — `lsScene` owns the
  active camera (`setActiveCamera`/`getActiveCamera`, non-owning);
  the demo's rig sets it once; initPipeline reads the scene, not the
  player rig; `lsSceneStage` falls back to the scene camera at
  execute-time when none is set. Verified: full-scene capture from the
  scene-owned camera, ctest 100%.
- E3.3 **R6: legacy code REMOVED** ✅ 2026-08-14 (user directive:
  no zombie code) — deleted `vortxGE/src/passes/` (lsPass, lsPassManager,
  lsPass_DR, lsPass_FBO, lsPass_Occlusion, lsPass_ShadowMap) and
  `vortxGE/src/controls/` (lsControl, lsControlManager, lsComposite,
  lsViewer) + `lsStagePassToken.h`. Scene-graph Draw/Optimize/UseShader
  signatures carry `int pass_type` (LSPT_* moved to
  `render/lsRenderTypes.h`); pipeline stages hold a plain int. The demo
  owns its `lsSceneManager` directly and ticks it in `Update()`.
  `VORTX_LEGACY_PIPELINE` is gone. Verified: build clean, ctest 100%,
  GL physics ticking (bodies=106), VK full-scene capture. The gtkmm
  demo rides the same demoscene — no porting needed.
- Guard: `VORTX_SHOT` parity + frame-dump diff clean + ctest 100%
  before/after each step.

**Done when:** demos run the pipeline with no env var; legacy path
removed or clearly deprecated; docs updated.

### E4 — Blender tooling & live-link

- **E4.1+E4.2 — ✅ DONE 2026-08-14.** Addon synced to v1.6 (repo
  `blender_scripts/export_scene.py` is now the single source; installed
  over the stale v1.4). `export_scenes.py` warns when the installed
  addon is older. Live-link phase 2: `vortxge.push_to_engine` operator
  + Live-link panel section (target path + Push button); verified
  headless (push wrote 65 lights WITH cast_shadows flags). Original notes:
 the installed Blender addon
  (`~/.config/blender/5.2/scripts/addons/vortxge_exporter.py`) is
  v1.4; `blender_scripts/export_scene.py` is v1.5 with `cast_shadows`.
  A GUI export through the old addon **silently strips cast_shadows
  flags** (observed 2026-08-14 — stripped all 64 flags from
  light_stress.vgs). Sync the addon; add a version check to
  `tools/export_scenes.py`.
- **E4.2 Live-link phase 2:** one-click push from Blender —
  `blendermcp.start_server` + MCP `execute_code` push works manually
  (phase 1 engine side done: `VORTX_LIVELINK=1` hot-applies
  transforms/light props, verified). Add a "Push to engine" operator
  to the addon UI that re-exports straight onto the watched `.vgs`.
- **E4.3 (stretch) Structural live updates — DESIGN COMPLETE
  2026-08-14**, implementation deferred (not blocking).
  Problem: live-link today updates transforms/light props on EXISTING
  nodes; add/remove breaks because pipeline stages bind scene light
  POINTERS at initPipeline (lsShadowStage/lsCsmStage hold lsLight*),
  and the cluster stage reads the scene's light list once.
  Design:
  1. Pipeline gains `rebindLights(scene)` — rebuilds only the light
     stages (shadow/CSM/point-shadow) from the scene light manager;
     preserves targets and camera stages. Scene emits a new
     `LSPE_LIGHTS_CHANGED` event when lights are added/removed/
     cast_shadows-toggled; the pipeline checks the flag once per frame
     (cheap) and rebinds between frames (never mid-command-buffer).
  2. lsLightManager gets add/remove notifications (it already owns the
     list) → sets the dirty flag on the owning scene.
  3. Live-link: Blender push of a new light appends the node via the
     existing node-creation path; the flag triggers the rebind next
     frame. Removal: the existing DelNode event chain handles teardown;
     rebind rebuilds stages without the removed light.
  4. Safety: rebind runs between frames after EndFrame; GL/VK device
     idle is guaranteed by the existing frame fence.
  Estimate: ~1 day including stress-scene verification (64→65 lights
  live). Tracked here; implement when a game needs it.

**Done when:** edit a light in Blender → click → it moves in the
running demo; addon version mismatch is caught by the export tool.

### E5 — Clustered-lighting polish — ✅ DONE 2026-08-14

- E5.1 ✅ REDEFINED by measurement: the upload was never the cost
  (0.001ms GPU) — the CPU grid BUILD was (30ms on the stress scene,
  std::pow per cell per light). Fixed: precomputed slice-depth table +
  per-cluster bounding spheres → 3.45ms. Sub-rect upload not needed.
  Original note:
  (~1 MB) → sub-rect upload of dirty region.
- E5.2 ✅ Light cap 64 → 128 (uniform arrays, cluster slot cap, all 4
  light_tex_* shaders, demo link cap; stress scene verified >64).
- E5.3 ✅ Profiled with the E2 profiler (that's what found E5.1);
  consider partial rebuilds (static lights only re-assign on move).

**Done when:** measurable upload/build cost reduction; 128-light
scene interactive.

### E6 — Sim-time determinism — ✅ DONE 2026-08-14

Emitter spawns and object lifetimes are wall-clock (`lsTimer`), not
sim-time — `VORTX_FIXED_DT` works around it for captures by disabling
spawns. Migrate lifetimes to sim time so the whole engine is
replay-deterministic; remove the capture workaround.

**Done when:** animated demo scenes produce identical frame-N dumps
across runs without disabling spawns.

### C-series — Camera/Light rewrite — ✅ DONE 2026-08-14

User-reported regression (sun shadows moving / whole scene flickering
dark on tiny camera movement) + mandate to rip apart Camera/Light.

- **C0 Evidence + tooling** ✅ — deterministic tiny-orbit hook
  (`VORTX_CAM_ORBIT=1` in demoscene), GL two-shot `VORTX_SHOT` (frames
  30+90) for temporal A/B. The user's symptom was traced to a real
  bug in the existing L1.5 texel snap.
- **C1 Matrix hygiene** ✅ — lsMatrix4x4 convention doctests lock down
  column-major layout, LookAt basis, Ortho depth mapping (engine maps
  zMin→+1, zMax→−1), and RotateMatrix's Y-rotation handedness.
- **THE BUG (root cause)** ✅ — the existing L1.5 ortho snap built a
  light-VIEW-space translation but multiplied it AFTER the projection
  (`snap * P * V` instead of `P * snap * V`). Tiny camera moves then
  mapped fragments wildly out of the shadow map → whole scene flickered
  dark/lit. Fixed to `P * snap * V`; debug overflow guard added.
- **C2 lsCamera** ✅ — dirty-flag rebuild (`markDirty`/`isDirty`;
  Update_Camera no-ops when clean), ortho extent getters,
  getProjectionMatrix accessor.
- **C3 lsLight_Direct** ✅ — follow logic extracted to
  `updateFollowCamera()`; sun follow preserved by design.
- **C4 Shadow consistency** ✅ — `lsLight::getShadowMatrix(render)`
  caches per frame (counter advanced in `lsRenderPipeline::render()`);
  the matrix used to render the map and to sample it are identical.
- **C5 Verification** ✅ — new doctests (dirty flag, ortho getters,
  per-frame cache, follow offset); 75/75 tests pass; GL+VK orbit
  captures normal and lit.

**Design note for multi-camera:** `lsScene::setActiveCamera` is only a
default pointer. Additional cameras (rear-view mirror, split-screen)
get their own `lsSceneStage` + named target + blit — no global state.

---

## Deferred / optional

### L6 — (Vulkan-only) RT shadow path
`VK_KHR_ray_tracing_pipeline` on RDNA3: hard RT shadows for the sun +
SVGF-style temporal denoise; hybrid with CSM (RT near field).
Strictly opt-in (`VORTX_RT_SHADOWS=1`); GL path untouched.

### D4 — On-screen profiler overlay
Needs text rendering; defer until a real need exists. D3 console/CSV
covers diagnostics.

---

## Backlog (non-blocking)

- VK z_ndc<0 clip gap (`VK_EXT_depth_clip_control`) — open, low
  risk (no observed artifacts; GL-style [-1,1] clip works via the
  negative-height viewport path). Pick up when a real artifact appears.
- Sun-map depth compression — REVERTED 2026-08-14: per-frame ortho
  near/far refit destabilized shadows (radius-capped floor mesh broke
  the fit). Sun keeps its fixed, conservative range. Kept: the
  getShadowMatrix cache now respects the projection dirty flag.
- Validate the 2026-08-11/12 shadow fixes in a real game scene —
  blocked: needs a real game scene (stress scene covers lighting).
- CSM + point-cube paths still use fixed PCF (PCSS follow-up) — open
  quality item; single-map path has PCSS on High (L4.3).

STATUS 2026-08-14: every actionable Plan.md item is complete. L6 (RT
shadows) and D4 (profiler overlay) remain Deferred/optional by design;
the backlog entries above are explicitly non-blocking.

---

## Completed work (archive)

### L-series — lighting & shadow refactor — COMPLETE 2026-08-14

- **L0 foundations (2026-08-04):** emitted/dynamic objects lit (scene
  light-manager fallback in `lsShader::Setup_Lights`); shadow-map
  V-mirror + array-element uniform lessons recorded.
- **L1 shadow core (2026-08-12):** `lsShadowMap` subsystem + presets
  (`VORTX_SHADOW_QUALITY`, 253d3ff); VK pipeline-state depth bias
  (db67ecc); comparison samplers both backends — `sampler2DShadow`,
  HW bilinear compare (708dfc4); 16-tap rotated Poisson PCF +
  normal-offset bias (eb70c00/5638039); texel snapping (7132407).
- **L2 CSM sun (2026-08-13):** `lsCsm` math + doctests (414cb19);
  `lsCascadedShadowMap` atlas + `lsCsmStage` (e68b39c); shader
  cascade select + blend band (33c2d4a); `VORTX_CSM=1`.
- **L3 full light-type coverage (2026-08-13):** spot frustum
  tightening (5a297d4); point-light cube shadows `lsFBO_DepthCube` +
  `lsPointShadowStage`, `samplerCubeShadow` path (GL 76cfe74, VK
  26ce816); per-light `cast_shadows` gates (1cc00f4).
- **L4 soft shadows (2026-08-13):** PCSS via raw-depth alias view
  (4f93549); depth+normal prepass `lsPrepassStage` (High preset);
  screen-space contact shadows (12-step ray march). Latent
  uniform-id init bug fixed along the way.
- **L5 clustered forward (2026-08-14):** 64-light uniform arrays;
  `lsClusterGrid` + `lsClusterStage` (32 px tiles × 12 exp slices,
  RG32UI/R32UI textures on units 12/13); smooth windowed falloff
  `saturate(1-(d/r)⁴)²`, `r = sqrt(intensity/0.25)`; VK integer
  samplers + `clusterFlipY`; `cast_shadows` Blender plumbing +
  light_stress scene (64 lights, runs ~52–55 fps at 2400×1350).
  Base scene pixel-identical (RMSE 2e-6). ffa43d0.
- **Live-link phase 1 (2026-08-14):** `VORTX_LIVELINK=1` hot-applies
  `.vgs` edits (transforms + light props) in place; no pipeline
  teardown.

### D-series D0–D2 — COMPLETE 2026-08-12

- **D0** gating: `VORTX_DEBUG_TOOLS` CMake option, `lsDebug.h`,
  Release contains zero debug strings.
- **D1** light isolation & debug views: `VORTX_ONLY_LIGHT`,
  `VORTX_LIGHTS_OFF`, `VORTX_DEBUG_VIEW` — found the VK dark-floor
  root cause (stale active-flag gating) + V-mirror.
- **D2** frame dump: `VORTX_FRAME_DUMP=N:path.json` +
  `tools/diff_frames.py`; GL/VK frame-120 dumps diff clean;
  `VORTX_FIXED_DT`/`VORTX_RAND_SEED` deterministic capture.

### Design rationale (kept — still binds new work)

**Decision record — forward vs deferred (2026-08-04, agreed):** stay
forward; no deferred renderer. Shadow work is renderer-agnostic;
deferred costs MRT through both backends + shader rewrite; many
lights are solved by clustered forward (L5), not deferred. A
depth+normal prepass (L4.1b, done) unlocks SSAO/SSR/contact shadows
without deferred. Revisit only if screen-space decals become a hard
requirement.

**Research summary 2025–2026 (what games use):** CSM = sun baseline
(done); PCF/PCSS = filtering baseline (done); contact shadows =
gap-filler (done); clustered/Forward+ = many-light answer (done);
hybrid RT = AAA-on-RT (L6, optional); VSM/ReSTIR/Lumen = non-goals.

**Invariants (hold after every phase):**
1. GL 3.3 and Vulkan produce matching output — verify with
   `VORTX_SHOT` captures on both backends after each phase.
2. Pipeline architecture is the render path: new work lands as
   stages/pass tokens, not control classes (E3 makes it the default).
3. No per-frame swapchain recreation; no per-frame driver waits.

---

## Working agreements

- One phase (or phase item) per working session; commit per item with
  `VORTX_SHOT` before/after evidence on **both** backends where the
  change is render-visible.
- Update this file as work lands; move finished phases to
  `CHANGELOG.md`.
- D-series only: every phase must prove Release-build exclusion
  (`strings` + binary-size check) before being marked done.
- No CI by policy (2026-08-03): local build + full doctest suite
  before every push. Only revisit if the engine is ported off Linux —
  then self-hosted runners, never hosted CI.
