# Terrain Texturing Plan (T-series)

**Goal:** high-detail terrain texturing near the player/camera without
hurting performance. No external texture assets — everything procedural.

**Status (2026-08-15 b):** T5+T6 done — macro-variation anti-tiling
(1 sample at ~97 m period, luminance mod 0.82+0.36) and far-field cheap
path (past 250 m + 60 m band: no texture fetches, weights x
precomputed layer-average colours). Debug toggles verified:
VORTX_NOTEXTURE=1 (flat albedo off-switch), VORTX_SPLAT_DEBUG=1
(weights as R=grass/G=rock/B=sand), VORTX_SPLAT_LAYER=0-3 (isolate).

**Status (2026-08-15):** T1-T4 done — terrain_splat shader on GL+VK,
4 procedural layers (linear RGBA8 upload — sRGB decode double-darkens),
height-based blending (0.2 depth window), near detail (7x grain, 30 m
fade). ~60 fps vsync cap with splat+detail+veg (A/B: 59.8 full / 59.2
detail-off / 59.5 splat-off — texturing is ~free at 2400x1350).

**Baseline (2026-08-15):** terrain = 8×8 chunks (128 m), geometry LOD
strides per distance band, `light_tex_none` shader (flat albedo, no
textures). Mesh has normals + chunk-local UVs. `terrain::Noise` (fbm)
available. ~60 fps vsync with instanced veg.

## Research summary (best practices)

1. **Texture splatting** (industry standard; Unity/Unreal):
   low-res weight map over the whole terrain + a few *tiling* detail
   textures (grass/dirt/rock/sand/snow), blended per pixel. Weights can
   be painted, baked, or computed **procedurally** from altitude/slope/
   noise — we already have exactly those rules in the veg scatterer.
2. **Height-based splat blending** (Mishkinis, "Advanced Terrain
   Texture Splatting"): add a per-texture height/depth map to the splat
   weight before blending and renormalize over a small depth window
   (`ma = max(h1+w1, h2+w2) - d; b = max(h+w-ma, 0)`) — sand fills the
   cracks between stones instead of a soapy lerp. Big quality win, ~free.
3. **Detail texture near camera** (since Quake era, Unity "Detail"
   node): multiply/overlay a high-frequency tiling detail within ~30 m,
   **distance-fade it out** — near pixels get crisp grain, far pixels
   don't pay for it and don't shimmer.
4. **Distance fade to cheap composite** (Unity HDRP BaseMapDist):
   far terrain renders a single low-frequency baked color instead of
   N splat samples. We can do this per-pixel (fade) and per-chunk
   (cheap shader variant for the farthest LOD band).
5. **Anti-tiling**: repetition is the #1 visual tell.
   - cheap: macro-variation (lerp albedo between two very different
     UV scales by large-noise) — 1 extra sample;
   - best: stochastic/hex-tile sampling (Eric Heitz; Unity PST) —
     3 samples + triangle weights, ~3-5× a normal sample, so gate it
     to the near/mid field only.
6. **Slope masks / triplanar**: cliffs (steep faces) get rock via
   `1 - normal.y` mask; triplanar projection avoids stretching on
   near-vertical surfaces (later).
7. **Texture arrays** for layers: one bind, index per layer; avoids
   sampler-count limits when layers grow (later).
8. (Not now) Virtual texturing / RVT — heavy machinery, unneeded at
   1024×1024 m.

## Design decisions

- **Splat weights: procedural in-shader** (altitude, slope from
  `normal.y`, fbm noise) — mirrors the scatterer rules, zero texture
  memory, no baking step. Fallback option: bake per-vertex weights if
  fragment cost shows up in profiles.
- **Textures: procedural at startup** via `terrain::Noise` — 4 layers
  (grass, dirt, rock, sand; snow reserved), 512×512 RGBA8, height in
  alpha for T3 blending. Plain `sampler2D`s first (existing plumbing);
  texture array if we exceed 4-5 layers.
- **Shader:** fork `light_tex_none` → `terrain_splat` (keeps all
  lighting/shadow/cluster code), add samplers + splat logic. Same GLSL
  feeds GL and Vulkan through the existing transform.
- **Perf guards:** every stage is env-toggleable and distance-gated;
  target is to stay at the ~60 fps vsync cap.

## Stages

- [x] **T1 — splat foundation:** `terrain_splat` shader compiling on GL
  + VK, world-space UV, 4 procedural tiling layers, linear splat from
  altitude/slope/noise weights. Verify: biome-following textures in
  captures, fps at vsync.
- [x] **T2 — procedural texture generation:** `terrain::TextureGen`
  producing the 4 RGBA layers (albedo RGB + height A) at startup;
  cached to `demos/data/textures/terrain/` (write once, reuse).
- [x] **T3 — height-based blending** (depth window 0.2) replacing the
  linear lerp. Verify: close-up capture shows granular transitions.
- [x] **T4 — near detail + distance fade:** extra detail octave,
  ~30 m fade-in/out; far pixels skip it entirely.
- [x] **T5 — anti-tiling:** macro-variation (2-scale lerp by fbm);
  optional stochastic hex sampling gated < 150 m.
- [x] **T6 — far-field cheap path:** per-pixel fade to single
  low-freq color; optionally a `terrain_splat_far` variant without
  samplers for the farthest LOD band.
- [ ] **T7 (stretch):** triplanar cliffs, texture-array refactor,
  per-layer normal maps.

## Verification (every stage)

- `ctest` 100%.
- GL + VK captures: ground-level close-up, mid, top-down; check
  biome-following, no seams at chunk borders (world-space UVs),
  no tiling bands at distance.
- `VORTX_PHASE_TIME=1`: render CPU stays ≪ 1 ms; fps at vsync cap.

## Env toggles (debug)

`VORTX_SPLAT_DEBUG=1` (visualize weights as RGB), `VORTX_SPLAT_LAYER=n`
(isolate one layer), `VORTX_NOTEXTURE=1` (revert to flat albedo),
`VORTX_DETAIL_DIST=m` (detail fade range override).
