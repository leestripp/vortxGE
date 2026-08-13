# Plan.md — vortxGE Lighting & Shadow System Refactor

> **Active plan:** full lighting/shadow rewrite (L-series).
> **Status:** D0–D2 debug tooling done (2026-08-12). Next: validate
> fixes in real game scenes, then L1 — Shadow core rewrite.
> Previous roadmap (renderer abstraction, Vulkan backend, pipeline
> stages R1–R4) is complete — see `CHANGELOG.md` and git history.

---

## ⏭️ Session handoff — resume here 2026-08-12

**State at end of 2026-08-11: the "Vulkan dark floor" saga is CLOSED.**
Vulkan now matches OpenGL visually in the demo (fully lit floor,
shaded objects, soft shadows, correct startup size). ctest 100%.

Five commits (newest first):

| Commit | What |
|---|---|
| `cfb92a7` | Startup black bands: self-healing framebuffer-size poll in glfwApp (Wayland applies scale only at window mapping, after Init's size query; no callback fires — poll heals it) |
| `2c5d033` | Shadow-map V-mirror: VK offscreen FBOs render V-flipped (negative-height viewport) — y-flipped shadow bias on `RenderType::Vulkan` in `lsLight::buildShadowmap_matrix` (same precedent as blit `flipY`) |
| `b528b5f` | THE dark-floor fix: (1) `lsShadowStage` no longer gates on `getActive()` (flags refresh AFTER shadow stages run → spot maps never rendered, shading sampled zeroed maps); (2) `lsShader` shadow binding shares `resolveLightList` + active-filter + slot compaction with `Setup_Lights` (latent GL bug too). New hooks: `VORTX_PASS_TRACE`, `VORTX_SHADOW_TRACE` |
| `e719282` | GL↔VK depth-range translation at the backend boundary (`lsVulkan::DepthRangef` stores GL (n,f), viewport emits `min=(n+f)/2, max=min+(f−n)/2`) — all VK depth buffers GL-convention. New hook: `VORTX_DEPTH_SHOT` (D32 depth readback, auto-contrast) |
| `8fc03f4` | D1 debug tools: `VORTX_ONLY_LIGHT` / `VORTX_LIGHTS_OFF` / `VORTX_DEBUG_VIEW` (lightcount, shadow, normals, depth false-color) |

Every fix site carries a `FIX 2026-08-11` comment cross-referencing
this section. Root-cause write-ups are in the D1 section below and in
AGENTS.md (coordinate conventions, V-mirror rule, debug env table).

**Debug tooling now available** (all gated behind `VORTX_DEBUG_TOOLS`,
zero Release footprint): `VORTX_SHOT`/`F12` screenshots, FBO colour +
depth readback, per-light isolation, false-color debug views, pass/
shadow tracing. Full table in AGENTS.md §debug env vars.

**Priorities, in order (updated 2026-08-12 after D2):**
1. ~~D2 — JSON frame dump + GL/VK diff script~~ **DONE 2026-08-12**
   (see §D2 below; GL/VK frame-120 dump pair diffs CLEAN).
2. **Validate all fixes in real game scenes** (not just the demo).
3. Then resume the L-series at **L1 — Shadow core rewrite** (§L1):
   the shadow path is now well-instrumented and the fixes above are
   the foundation L1 builds on (texel snapping, cascades, point-light
   shadows remain open by design).
4. Backlog (non-blocking): VK z_ndc<0 clip gap
   (`VK_EXT_depth_clip_control`), sun-map depth compression
   (deep ortho frusta → consider tighter fitting in L1/L2),
   emitter/object lifetimes are wall-clock (`lsTimer`) not sim-time —
   deterministic capture tooling works around it (`VORTX_FIXED_DT`
   disables demo spawns), but a sim-time migration would make the
   whole engine replay-deterministic; intermittent VK teardown
   segfault observed once on SIGTERM shutdown (not reproducible,
   not D2-related — no D2 code runs at exit).

---

---

## Why rewrite (current state, honest assessment)

- Forward renderer with **3 lights hard-coded** in shaders
  (`vxLightPos[3]`, `vxLightType[3]`, …); `lsLightManager` picks the
  nearest ≤3 within 50 m. No path to more lights.
- Per-light 2D shadow map FBOs (`lsShadowStage`), depth-only pass,
  basic depth-compare in the PBR shader. Known weaknesses: hard
  aliased edges, hand-tuned bias, no texel snapping (swimming), no
  cascade support, spot maps waste resolution, **no point-light
  shadows at all**.
- **Dynamic/emitted objects get no lighting** (user-reported
  2026-08-04): objects spawned by `lsObjectEmitter` after scene light
  linking never enter the light lists.
- Backend lessons already learned (do not re-learn): Vulkan needs
  array-element uniform decoding (`name[i]` → member + elem<<16),
  FBO textures are Y-flipped vs GL (blit `flipY` auto), swapchain
  recreate must no-op when extent is unchanged.

## What games actually use in 2025–2026 (research summary)

| Technique | Used by | Takeaway for vortxGE |
|---|---|---|
| **Cascaded Shadow Maps (CSM)** | every outdoor game, still the sun-shadow baseline | S2 — 3–4 stable cascades for the sun |
| **Virtual Shadow Maps (VSM)** | UE5 (Nanite pipeline); cached 16K pages, dirty-page re-render | aspirational; too heavy for our dual GL/VK shim — skip for now |
| **PCF + improvements** | universal baseline filtering | S1 — 3×3→5×5 PCF w/ rotation/poisson taps |
| **PCSS / contact-hardening** | NVIDIA hybrid pipelines, many AA titles | S4 — blocker-distance penumbra over CSM |
| **Screen-space contact shadows** | most engines as gap-filler | S4 — cheap ray march in depth |
| **Hybrid RT shadows (DXR/VK_RT)** | AAA on RT hardware; RX 7800 XT (RDNA3) has RT cores | L6 optional Vulkan-only path |
| **Clustered/Forward+ lighting** | industry standard for many lights (AMD Forward+, idTech, etc.) | L5 — replaces the 3-light limit |
| **Deferred/visibility buffers** | AAA heavy pipelines | out of scope — stay forward |
| **ReSTIR, Lumen/SDF GI, Nanite** | UE5/CDPR tier | non-goals (see below) |

Non-goals: deferred renderer rewrite, virtualized geometry, GI/Lumen,
ReSTIR, VSM page caching. The engine stays a **forward renderer with
modern shadow quality + clustered light assignment**.

## Decision record — forward vs deferred (2026-08-04, agreed)

**Decision: stay forward; do NOT build a deferred renderer first.**

- Shadow work (L1–L4) is renderer-agnostic: maps are rendered from the
  light's POV and merely sampled by the shading pass. Deferred would
  delay shadows without improving them.
- Deferred costs this engine specifically: MRT G-buffer through
  `lsRender` on BOTH backends (new parity surface), a rewrite of every
  fragment shader + a material-model constraint, a hybrid forward pass
  for transparency anyway, and 3–5 full-res targets at 3840×2160.
- Many lights do not require deferred — clustered forward (L5) is the
  industry answer (AMD Forward+, Doom 2016).
- 2025-26 trend for new engines is clustered forward / visibility
  buffers, not classic deferred.
- **Where deferred genuinely wins** (recorded for future review):
  SSAO/SSR and especially screen-space decals. Mitigation chosen: a
  **depth+normal prepass** added in L4 — SSAO/SSR/contact-shadows all
  work on depth+normals. Revisit deferred only if decals become a
  hard requirement.

---

## Invariants (hold after every phase)

1. **GL 3.3 and Vulkan produce matching output** — verify with
   `VORTX_SHOT` captures on both backends after each phase.
2. Pipeline architecture (R-series) is the render path: new work
   lands as stages/pass tokens, not control classes.
3. No per-frame swapchain recreation; no per-frame driver waits.
4. `VORTX_PIPELINE=1` demo stays the test bed; legacy path keeps
   working until L-series is done.

---

## L0 — Foundations & bug fixes — L0.1 done 2026-08-04

- L0.1 **Dynamic object lighting** ✅ done: emitter-spawned objects must
  register with `lsLightManager` (or per-object light evaluation)
  so emitted cubes are lit. Root-cause: light linking is a
  scene-load-time step only. **Fixed: scene light-manager fallback
  in lsShader::Setup_Lights when the per-object list is empty.**
- L0.2 **Shadow-map orientation audit on Vulkan**: FBO-rendered
  depth maps are Y-flipped vs GL (same convention issue as the
  blit). Verify shadow UV sampling matches on both backends;
  compensate in one place (shadow matrix or stage), not per shader.
- L0.3 **Shadow bias/uniform plumbing check**: array-element
  locations for shadow matrices/samplers on Vulkan.

**Done when:** emitted cubes lit on GL+VK; existing shadows identical
or better on both backends.

## L1 — Shadow core rewrite

**Execution order (agreed 2026-08-12, next session starts here):**
1. **L1.3 comparison samplers** — highest value, self-contained.
   Engine: add `LS_TEXTURE_COMPARE_MODE` / `LS_TEXTURE_COMPARE_FUNC` /
   `LS_COMPARE_REF_TO_TEXTURE` constants to `lsRenderConstants.h`;
   GL: no-op beyond constants (glTexParameteri already reachable via
   `TexParameteri`); VK: `createTextureViewAndSampler` in lsVulkan.cpp
   must set `VkSamplerCreateInfo.compareEnable/compareOp` when the
   texture is a shadow map (mark via TexImage2D with LS_DEPTH_COMPONENT
   internalformat). Shaders (all 4 light_tex_*): `sampler2D
   tex_shadowmaps[3]` -> `sampler2DShadow`, PCF taps become
   `texture(shadowMap, vec3(offset.xy, projCoords.z))` single fetches
   (HW bilinear comparison = free 4-tap PCF per fetch).
2. **L1.4b normal-offset bias** — needs world-space normal in the frag
   shader (vert already has nMatrix*vxNormal view-space; add a world
   normal varying), offset shadowCoord along world normal by
   ~1-2 texels before projecting. Kills acne + peter-panning; lets
   L1.2 shrink the polygon offset.
3. **L1.5 texel snapping** — in `lsLight` ortho projection for the
   sun: round the light-space origin to the shadow-map texel grid
   (texel = orthoWidth/mapSize) to stop swimming.
4. **L1.1/L1.6 lsShadowMap + presets** — refactor lsShadowStage's
   lsFBO_Depth ownership into a per-light subsystem class; quality
   presets low/med/high select map size (1024/2048/4096) + PCF tap
   count via a `vxShadowQuality` uniform; stage becomes a thin driver.
5. **L1.2** front-face culling option on the depth pass + depth-bias
   via pipeline state on VK (VkPipelineRasterizationStateCreateInfo
   depthBias fields — needs pipeline-key extension) replacing
   glPolygonOffset semantics; retire the shader bias to ~0.
Verify each step: VORTX_SHOT before/after on BOTH backends +
frame-dump diff CLEAN (128 draws, deterministic capture recipe in
AGENTS.md) + ctest 100%.

**L1 COMPLETE 2026-08-12:** L1.1 lsShadowMap + L1.6 presets (253d3ff),
L1.2 VK pipeline-state depth bias (db67ecc), L1.3 comparison samplers
(708dfc4), L1.4 PCF+normal-offset (eb70c00/5638039), L1.5 texel
snapping (7132407). Front-face-cull depth-pass option deferred
(normal-offset bias covers acne). Presets: VORTX_SHADOW_QUALITY.

**Progress 2026-08-12 (2):** L1.3 landed — comparison samplers on
both backends (lsFBO_Depth: LINEAR + COMPARE_REF_TO_TEXTURE/LEQUAL;
VK sampler compareEnable=LESS_OR_EQUAL; tex_shadowmaps now
sampler2DShadow; every Poisson tap is a HW bilinear comparison =
16x4 effective taps). Verified: GL/VK VORTX_SHOT parity, dump diff
CLEAN (172 draws), ctest 100%.

**Progress 2026-08-12:** L1.4 core landed — `calculateShadow` in all
four `light_tex_*` fragment shaders replaced the pseudo-9x9 grid
(it sampled a ±0.4-texel sub-area — supersampling, not real PCF)
with a 16-tap rotated Poisson-disk PCF (2.5-texel spread, per-pixel
random rotation). Verified: GL/VK `VORTX_SHOT` parity, no acne,
softer edges, frame-dump diff CLEAN (128 draws), ctest 100%.
Remaining L1 items unchanged below.

Single shadow subsystem owning all shadow rendering/sampling:

- L1.1 `lsShadowMap` (new, engine-level): per-light depth target,
  atlas allocation option, resolution per light type/config.
- L1.2 Depth-only pipeline: dedicated fast depth shader path
  (no material/fragment cost), front-face culling option,
  polygon offset/depth-bias via pipeline state (Vulkan) and
  `glPolygonOffset` (GL) — kill hand-tuned shader bias.
- L1.3 Comparison samplers: GL `GL_COMPARE_REF_TO_TEXTURE` /
  Vulkan `VK_COMPARE_OP_*` sampler so PCF taps are single
  texture fetches (`sampler2DShadow` / `texture2DShadow` in
  shaders on both backends).
- L1.4 Filtering: 3×3 PCF → 5×5 rotated-poisson PCF; normal-offset
  bias to kill acne+peter-panning together.
- L1.5 Directional stability: texel snapping (round light-space
  origin to texel grid) — no more swimming while moving.
- L1.6 `lsShadowStage` becomes a thin driver over the subsystem;
  shadow quality presets (low/med/high) in config.

**Done when:** sun + both spots cast stable, acne-free shadows on
GL+VK at 60 fps; quality preset visibly scales.

## L2 — CSM for the sun

- L2.1 3–4 cascades, practical split scheme (lambda blend
  uniform/log), per-cascade stable ortho fitting to camera
  frustum slice bounds.
- L2.2 Per-cascade texel snapping + cascade overlap blend band.
- L2.3 Atlas: one depth target, 2×2 viewport quadrants (GL/VK
  identical semantics — scissored viewports both support).
- L2.4 Shader: cascade select by view depth, blend across seam;
  keep single-cascade fallback path for low preset.

**Done when:** shadows usable from 0.5 m to 300+ m from camera with
no visible seams while moving/rotating.

**L2 COMPLETE 2026-08-13:** L2.1a lsCsm split/fit math + doctests
(414cb19), L2.1b+2.3 lsCascadedShadowMap atlas + lsCsmStage (e68b39c),
L2.2 world-anchored per-cascade texel snapping (in 414cb19, tested),
L2.4 shader cascade select + 10% blend band (33c2d4a). Sun renders
via CSM when VORTX_CSM=1 (dev flag in demoscene); single-map sun
remains the default until soak testing. Verified GL+VK parity via
VORTX_SHOT captures, ctest 100%.

## L3 — Full light-type coverage

- L3.1 Spot: tighten frustum fit to cone, perspective-correct
  filtering.
- L3.2 **Point lights: omnidirectional shadows** — cube depth
  target (GL `GL_TEXTURE_CUBE_MAP`, VK cube image view layers,
  both behind `lsRender`) with 6-face pass; dual-paraboloid as
  GL-only fallback if cube proves painful.
- L3.3 Shadow-casting flag per light + per-object (caster mask).

**Done when:** a point light in the demo scene casts correct
shadows from all directions on both backends.

**L3 COMPLETE 2026-08-13:** L3.1 spot frustum tightening —
cone-synced FOV + content-fitted near/far (5a297d4, doctested).
L3.2 point-light omnidirectional shadows: lsFBO_DepthCube +
lsPointShadowStage (6-face 90-deg pass) + samplerCubeShadow path
with major-axis reference-depth reconstruction (GL 76cfe74, VK
26ce816; VK renders cube faces unflipped). Demo: VORTX_POINTLIGHT=1
dev light. Known limits: one point shadow caster at a time (shared
cube unit 7), point stage always renders all 6 faces (no per-face
culling refresh skip), L3.1 fit is position+radius based (no mesh
bounds yet). L3.3 caster flags (1cc00f4): per-light cast_shadows
gates stages + shading; per-object casts_shadows pre-existing,
now doctested.

## L4 — Soft shadows

- L4.1 PCSS: blocker search + variable penumbra from light size
  (sun angular size, spot/point radius) over L1/L2 maps.
- L4.1b **Depth+normal prepass** (forward-compatible): small 2-target
  pass before the main scene pass; enables contact shadows now and
  SSAO/SSR later without a deferred rewrite (see decision record).
- L4.2 Screen-space contact shadows (short depth ray march) for
  fine detail the maps miss.
- L4.3 Preset gating: PCSS on high only.

**Done when:** penumbra grows with occluder distance; contact
hardening visible near occluders.

## L5 — Clustered forward lighting (kill the 3-light limit)

- L5.1 Screen-tile/cluster light grid (16 px tiles × depth slices);
  CPU build first (engine already culls lights by distance),
  compute-shader build as Vulkan-only upgrade later.
- L5.2 Light data → SSBO/UBO array (no fixed 3); shaders index per
  cluster. GL 3.3: UBO-sized light cap (~64–128 visible lights);
  Vulkan: SSBO.
- L5.3 Shadow-casting lights stay few (atlas budget); non-casting
  lights become nearly free.
- L5.4 Demo: stress scene with 30–100 small lights.

**Done when:** 50-light scene runs interactively on both backends;
nearest-3 popping gone.

## L6 — (Optional, Vulkan-only) RT shadow path

- VK_KHR_ray_tracing_pipeline on RDNA3: hard RT shadows for sun +
  SVGF-style temporal denoise; hybrid with L2 CSM (RT near field).
- Strictly opt-in (`VORTX_RT_SHADOWS=1`); GL path untouched.

---

## D-series — Built-in debug & profiler tooling (agreed 2026-08-11)

Motivation: RenderDoc has no Wayland support (upstream issue #853), the
XWayland path in our container falls back to llvmpipe (no DRI3 on the
:99 display, RADV can't present) and produces no capture; no other
Wayland-capable frame debugger exists (GFXReconstruct is capture/replay
only, Nsight is NVIDIA-only, RGP is a profiler not a pixel debugger).
So the diagnostic capability we need — "which lights reach which pixels",
per-stage timings — gets built into the engine, engine-native and
GL<->VK comparable.

**Release-bloat rule (hard requirement):** everything in the D-series is
compiled out in Release builds. Mechanism: CMake option
`VORTX_DEBUG_TOOLS` (default ON for Debug, OFF for Release), adds
`VORTX_DEBUG_TOOLS=1` compile definition. All D-series code lives behind
`#if VORTX_DEBUG_TOOLS` / `lsDebug.h` macros that vanish in Release.
Env-var checks must not exist in Release binaries at all.
The builtin screenshot feature (`lsRender::Screenshot`, F12) is NOT
debug tooling — it is a real feature and stays always-on.

### D0 — Gating infrastructure — done 2026-08-11

- D0.1 CMake option `VORTX_DEBUG_TOOLS` with build-type default;
  compile definition on the vortxGE target (PUBLIC so demos see it).
- D0.2 `vortxGE/src/debug/lsDebug.h`: `VORTX_DEBUG_ONLY(x)` macro
  (compiles to nothing in Release), shared env-var helpers.
- D0.3 Migrate existing diagnostics behind the switch: `VORTX_SHOT`
  frame-30 auto-capture, `VORTX_FBO_SHOT`, `VORTX_VK_DEBUG`,
  `VORTX_VK_NOCULL`/`VORTX_VK_CCW`, `VORTX_DISABLE_SHADOWS`,
  `VORTX_RDC` (RenderDoc trigger — keep for the day RenderDoc works).
- **Done when:** Debug build behaves exactly as today; Release build
  contains none of the env-var strings (`strings` check) and the demo
  runs identically.

### D1 — Light isolation & debug views (dark-floor attack) — done 2026-08-11

**RESULT — dark-floor root cause found:** Vulkan shadow map sampling
returns ~0 everywhere (shadow debug view: whole scene black on VK,
correct lit/shadow shapes on GL) -> every light's contribution is
multiplied to zero -> "ambient-only" floor. With
`VORTX_DISABLE_SHADOWS=1` the VK scene is fully lit, identical to GL.
Lights/UBOs/materials/geometry are all FINE — the bug is isolated to
the shadow-map path (shadow FBO depth content or sampler binding).
Fix belongs to L1 (shadow core rewrite) or a targeted patch before it.
Side observation: VK viewport renders into a subregion of the
swapchain (black band right/bottom in 2400x1350 captures) — separate
viewport/extent mismatch to fix.

**UPDATE (same day):** GL-vs-VK depth-range mismatch fixed at the
backend boundary (lsVulkan::DepthRangef now translates GL(n,f) ->
viewport min/max, all VK depth buffers GL-convention). Necessary but
NOT sufficient: VORTX_DEPTH_SHOT (new depth-readback diagnostic)
proves the scene FBO depth is written correctly but the shadow FBOs
are uniform 0.0 — the shadow render pass NEVER executes on Vulkan
(no clear, no geometry writes). Next: find why lsShadowStage's draws
never begin the depth-only render pass (draw skip, pass-token path,
or FBO bind before ensurePassActive).

**RESOLVED (same day, two commits):** the actual mechanism was a
*gating mismatch*, found via the new VORTX_PASS_TRACE /
VORTX_SHADOW_TRACE hooks: lsShadowStage::execute early-outed on
light->getActive(), but the active flags are refreshed by the scene
stage's Optimize AFTER the shadow stages run — so the spot stages read
stale-false flags and never rendered, while shading counted all 3
lights and sampled their zeroed maps. Legacy GL passes render all maps
unconditionally, which is why only Vulkan was dark. Fixes: (1) shadow
stage no longer gates on getActive (matches GL invariant: any light
that can shade must have a valid map); (2) UseShader's shadow-map
binding now uses the SAME active-filter + slot compaction as
Setup_Lights (resolveLightList helper — light->slot mapping can never
disagree again; this misalignment was also latent on GL). Remaining
shadow-quality items (separate): VK swapchain subregion viewport
mismatch. RESOLVED since: mid-floor dark band was the shadow-map
V-mirror (VK offscreen FBOs render V-flipped; fixed by y-flipped
shadow bias on RenderType::Vulkan in lsLight::buildShadowmap_matrix,
same precedent as blit flipY); "sun map no silhouettes" was an 8-bit
readback artifact — auto-contrast readback shows all 3 maps have
correct content (objects + floor gradient), depths merely compressed
by the deep light frusta.

- D1.1 `VORTX_ONLY_LIGHT=i`: process only light i in the shader light
  loop (others skipped); `VORTX_LIGHTS_OFF=1`: ambient only.
  Per-light `VORTX_SHOT` runs on both backends -> exactly which light
  is missing/dim on Vulkan's floor.
- D1.2 `VORTX_DEBUG_VIEW=lightcount|shadow|normals|depth`: lighting
  shader outputs false-color debug channels instead of shaded color
  (uniform switch, one uber-shader branch).
- **Done when:** per-light and debug-view PNGs exist for GL and VK,
  dark-floor cause identified or narrowed to a single light/uniform.

### D2 — Frame state dump ("mini-RenderDoc") — done 2026-08-12

**RESULT — both done-when criteria met.** GL/VK frame-120 dump pair
(128 draws, pipeline mode, `VORTX_FIXED_DT` + `VORTX_RAND_SEED` for
determinism) diffs CLEAN apart from two BY-DESIGN differences:
`sMatrices[5]` (VK y-flipped shadow bias, commit 2c5d033) and
`vxMatAlbedo` (dead-code-eliminated by the GL driver, present only in
the VK dump). The script flags a deliberately-broken dump (uniform
value, depth-state bit, draw count) with exact draw/field locations,
exit code 1. Release build contains zero D2 strings (strings check on
binary + libvortxGE.a).

- D2.1 ✅ `VORTX_FRAME_DUMP=N:path.json` (`debug/lsFrameDump.{h,cpp}`,
  gated by `VORTX_DEBUG_TOOLS`, empty TU in Release). Every draw of
  frame N: target FBO, program, VAO, mode/count/first, viewport,
  depth/cull/blend state, texture-unit bindings, float + int uniforms.
  Deliberately asymmetric data sources: GL records state tracked at
  the lsRender API boundary; Vulkan snapshots INTERNAL state at draw
  time (uboStaging at std140 member offsets, sampler-unit maps) so
  staging/layout bugs are visible in the diff. Cross-backend identity
  via signatures (programs = FNV-1a of shader sources, textures =
  w/h/fmt/content-hash); FBO/VAO ids canonicalised by the script.
  New hooks: `VORTX_FIXED_DT` (glfw3 demo fixed timestep + demo
  emitter-spawn disable), `VORTX_RAND_SEED` (deterministic lsRandom).
- D2.2 ✅ `tools/diff_frames.py`: aligns draws by sequence, normalises
  `name[i]` uniform keys vs VK base-name arrays, matches programs/
  textures by signature, per-field compare with float tolerance,
  `--skip-uniform`, `--tol`, `--max`; exit 0 clean / 1 mismatches.
- Lesson recorded: the demo scene is NOT cross-run deterministic by
  default (emitter spawns + object lifetimes are wall-clock, RNG is
  time-seeded) — frame-N dumps of animated scenes only align under
  `VORTX_FIXED_DT` + `VORTX_RAND_SEED`.

### D3 — CPU/GPU profiler

- D3.1 `lsProfiler` scoped zones (`lsProfileZone` RAII) over frame
  phases: Update, Shadow pass, Scene pass, Blit/present.
- D3.2 GPU timings: Vulkan timestamp queries around each pipeline
  stage (pool of query pairs, 1-frame latency); GL: `GL_EXT_timer_query`
  where available, CPU-only otherwise.
- D3.3 Output: rolling 120-frame summary to console every N frames
  (`VORTX_PROF=1`) or CSV (`VORTX_PROF=path.csv`); per-stage avg/max ms.
- **Done when:** per-stage GPU+CPU ms table on both backends; Release
  binary has no profiler code (size/strings check); profiler overhead
  <1% frame time when enabled, 0 when compiled out.

### D4 — (optional, deferred) on-screen overlay

Needs text rendering; defer until a real need exists. Console/CSV
output of D3 covers diagnostics for now.

## Working agreements

- One phase per working session; commit per phase with
  `VORTX_SHOT` before/after evidence on **both** backends.
- Update this file's phase checkboxes as work lands; move finished
  phases to `CHANGELOG.md` when the L-series completes.
- D-series only: every phase must prove Release-build exclusion
  (`strings` + binary-size check) before being marked done.
