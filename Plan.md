# Plan.md — vortxGE Lighting & Shadow System Refactor

> **Active plan:** full lighting/shadow rewrite (L-series).
> **Status:** L0 planning done (2026-08-04). No code yet.
> Previous roadmap (renderer abstraction, Vulkan backend, pipeline
> stages R1–R4) is complete — see `CHANGELOG.md` and git history.

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

## L0 — Foundations & bug fixes ✅ planned 2026-08-04

- L0.1 **Dynamic object lighting**: emitter-spawned objects must
  register with `lsLightManager` (or per-object light evaluation)
  so emitted cubes are lit. Root-cause: light linking is a
  scene-load-time step only.
- L0.2 **Shadow-map orientation audit on Vulkan**: FBO-rendered
  depth maps are Y-flipped vs GL (same convention issue as the
  blit). Verify shadow UV sampling matches on both backends;
  compensate in one place (shadow matrix or stage), not per shader.
- L0.3 **Shadow bias/uniform plumbing check**: array-element
  locations for shadow matrices/samplers on Vulkan.

**Done when:** emitted cubes lit on GL+VK; existing shadows identical
or better on both backends.

## L1 — Shadow core rewrite

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

## L4 — Soft shadows

- L4.1 PCSS: blocker search + variable penumbra from light size
  (sun angular size, spot/point radius) over L1/L2 maps.
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

## Working agreements

- One phase per working session; commit per phase with
  `VORTX_SHOT` before/after evidence on **both** backends.
- Update this file's phase checkboxes as work lands; move finished
  phases to `CHANGELOG.md` when the L-series completes.
