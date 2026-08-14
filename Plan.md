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

- **E1.1 Progressive slowdown — physics body leak.**
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
- **E1.2 VK "Failed to create shadow pass" ×64 on many-light scenes.**
  Pre-existing (predates L5), non-fatal but alarming console spam a
  developer WOULD report. Reproduce: `VORTX_SCENE=light_stress` on
  Vulkan. Pointshadow-pass related — root-cause and fix or gate.
- **E1.3 VK swapchain subregion viewport mismatch.**
  Content renders into ~80% of the swapchain (black bands
  right/bottom in 2400×1350 captures). Only remaining item from the
  dark-floor saga. Viewport/extent mismatch — inspect the blit/
  present path vs swapchain extent after the Wayland scale heal.
- **E1.4 Intermittent VK teardown segfault on SIGTERM** (seen once,
  not reproducible). Add to the soak list; fix if a second sighting
  gives a stack.

**Done when:** body count stable across spawn cycles; stress scene
runs on VK with zero shadow-pass errors; VK captures fill the full
swapchain; ctest 100%.

### E2 — D3: built-in CPU/GPU profiler (spec unchanged)

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

**Done when:** per-stage GPU+CPU ms table on both backends; Release
binary has no profiler code; overhead <1% frame time when enabled,
0 when compiled out.

### E3 — Pipeline by default (R3/R5/R6)

The stage-based pipeline is proven (L1–L5 all landed on it) but still
opt-in (`VORTX_PIPELINE=1`). A library should not ship two render
paths indefinitely.

- E3.1 **R3: pipeline becomes the default** demo path; legacy
  pass-manager path behind `VORTX_LEGACY_PIPELINE=1` for one release.
- E3.2 **R5: camera decoupling** — the pipeline must take its camera
  from the scene, not from viewer/control classes.
- E3.3 **R6: retire the old control/pass classes** once nothing
  references them; `lsPassManager` docs updated, migration notes in
  the demos doc.
- Guard: `VORTX_SHOT` parity + frame-dump diff clean + ctest 100%
  before/after each step.

**Done when:** demos run the pipeline with no env var; legacy path
removed or clearly deprecated; docs updated.

### E4 — Blender tooling & live-link

- **E4.1 Addon sync (bug):** the installed Blender addon
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
- E4.3 (stretch) Structural live updates: add/remove objects without
  restart (needs pipeline-stage light re-binding — design first).

**Done when:** edit a light in Blender → click → it moves in the
running demo; addon version mismatch is caught by the export tool.

### E5 — Clustered-lighting polish

- E5.1 Cluster grid upload: per-frame full-grid `TexImage2D`
  (~1 MB) → sub-rect upload of dirty region.
- E5.2 Light cap 64 → 128 (bigger UBO / SSBO on Vulkan).
- E5.3 Profile the per-frame CPU grid build (use E2's profiler);
  consider partial rebuilds (static lights only re-assign on move).

**Done when:** measurable upload/build cost reduction; 128-light
scene interactive.

### E6 — Sim-time determinism

Emitter spawns and object lifetimes are wall-clock (`lsTimer`), not
sim-time — `VORTX_FIXED_DT` works around it for captures by disabling
spawns. Migrate lifetimes to sim time so the whole engine is
replay-deterministic; remove the capture workaround.

**Done when:** animated demo scenes produce identical frame-N dumps
across runs without disabling spawns.

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

- VK z_ndc<0 clip gap (`VK_EXT_depth_clip_control`).
- Sun-map depth compression (deep ortho frusta → tighter fitting).
- Validate the 2026-08-11/12 shadow fixes in a real game scene
  (stress scene covers lighting; a real scene would cover layout).
- CSM + point-cube paths still use fixed PCF (PCSS follow-up).

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
