# Terrain System Plan — Custom C++23 Engine Module

> Research gathered 2026-08-14 via web search. **No plugins / middleware** — everything below is rolled in-house as part of the engine, in C++23.

---

## 1. Goals & Scope

A terrain subsystem for the engine that provides:

- Large-scale, heightmap-based terrain with continuous LOD
- Terrain texturing (splat mapping now, virtual texturing later)
- Vegetation: **grass, plants, bushes, trees** — procedurally scattered + hand-paintable
- GPU-driven vegetation rendering (instancing, impostors)
- Procedural generation pipeline (noise → erosion → biomes) with authored overrides
- Editor-facing painting/scattering tools (engine-integrated, no third-party)

**Non-goals (v1):** voxel/destructible terrain, caves/overhangs (heightmap only), runtime terrain deformation (keep the door open).

---

## 2. Terrain Geometry & LOD

### Research summary
| Technique | Notes | Sources |
|---|---|---|
| **Geometry Clipmaps** (Losasso & Hoppe, 2004) | Nested regular grids centred on camera; heightmap as vertex texture; great for huge terrains, but stitching/skirts and update logic are fiddly | [GPU Gems 2 Ch.2](https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry), [mikejsavage.co.uk writeup](https://mikejsavage.co.uk/geometry-clipmaps/) |
| **CDLOD** (Continuous Distance-Dependent LOD) | Quadtree of chunked grids, morphing between levels in the vertex shader; simpler to get crack-free than clipmaps; used in many shipped titles | [ResearchGate paper](https://www.researchgate.net/publication/220494186_Continuous_Distance-Dependent_Level_of_Detail_for_Rendering_Heightmaps) |
| **Chunked LOD / quadtree** | Classic block-based approach; community consensus is it's the most practical and debuggable | [gamedev.net thread](https://gamedev.net/forums/topic/525491-best-game-terrain-rendering-solution-to-use/), [vterrain.org paper survey](http://vterrain.org/LOD/Papers/) |

### Decision
**Chunked quadtree LOD with GPU vertex morphing (CDLOD-style)** as the primary path. Rationale: simpler streaming granularity (chunks = stream units), easier frustum culling, crack handling via skirts + morphing, and it maps naturally onto job-system parallelism. Keep the clipmap paper as reference if we ever need Google-Earth-scale zoom ranges.

### Implementation notes (C++23)
- World divided into fixed-size **tiles/chunks** (e.g. 64×64 or 128×128 verts), each with its own heightmap texture + AABB
- Quadtree per region; LOD selected by screen-space error or distance bands; **vertex morph** between LODs to avoid popping
- **Skirts** on chunk edges as cheap crack insurance
- Heightmap storage: 16-bit PNG or raw R16; `std::mdspan` (C++23) for clean 2D views over height data
- Async chunk build/stream on the job system; `std::generator` (C++23) is a nice fit for lazy chunk-iteration pipelines
- Frustum + (later) occlusion culling per chunk

---

## 3. Terrain Texturing

### Research summary
- **Splat/weight mapping**: RGBA control texture → blend 4 (or 8 with two maps) detail layers; the standard approach ([kosmonaut terrain overview](https://kosmonautblog.wordpress.com/2017/06/04/terrain-rendering-overview-and-tricks/), [r/GraphicsProgramming thread](https://www.reddit.com/r/GraphicsProgramming/comments/1i33etg/common-techniques-for-terrain-texture-splatting/))
- **Virtual texturing / megatexture** (Carmack's Quake Wars approach): one huge texture, paged in tiles; removes layer-count limits but is a big engineering lift ([2008 VT talk](https://www.youtube.com/watch?v=MejJL87yNgI), [Leadwerks VT terrain](https://www.leadwerks.com/community/blogs/entry/1112-virtual-texture-terrain/), [Frictional tech feature](https://frictionalgames.blogspot.com/2010/11/tech-feature-terrain-textures.html))
- **Rule-based auto-texturing**: derive splat weights from slope/altitude ([IJVR paper](https://ijvr.eu/article/view/2787/8845))

### Decision
**v1: splat mapping**, 2 RGBA control maps = up to 8 layers, height-blended (not plain lerp) for sharp transitions. Auto-generate base weights from slope/altitude rules, then allow hand-painting overrides.
**v2 (stretch):** virtual texturing once the streaming backend is proven.

---

## 4. Vegetation — the core of this plan

### 4.1 Data model
- **Vegetation layer assets**: each species (grass tuft, fern, bush, tree × N) is a layer with:
  - mesh(es) + LOD chain + impostor, material, density, scale/rotation variance
  - **scattering rules**: altitude range, slope range, biome mask, density mask texture, min spacing
- **Instance buffer per chunk**: vegetation instances are *generated per terrain chunk* (deterministic seed from chunk coords → stable across stream-in/out, no saved instance data needed for procedural areas)

### 4.2 Scattering / placement
Research: rule-stacked validation is the industry pattern ([Newhead scattering blog](https://newheadstudio.com/blog/object_scattering.html), [NDunes scattering rules](https://downloads.ndunes.com/download/NDunes/Documentation/en/bk_bi_learning_plants_scattering_rules.html), [Houdini HeightField Scatter](https://www.sidefx.com/docs/houdini/nodes/sop/heightfield_scatter.html)).

Plan:
1. **Poisson-disc sampling** per chunk (dart throwing with radius per layer) for natural, non-clumped spacing — can be jittered-grid for speed ([Poisson + Perlin discussion](https://www.reddit.com/r/proceduralgeneration/comments/fehqma/poisson_disc_sampling_formed_from_perlin_noise/))
2. Validate each candidate: slope, altitude, biome, splat-layer affinity (grass on grass splat, not on rock), density mask
3. Rejection output → instance list (pos, rot, scale, colour variation) uploaded to a per-chunk SSBO/vertex buffer
4. Optional **hand-painted density masks** multiply the procedural density

### 4.3 Grass rendering
Research: [GPU Gems 1 Ch.7 — Countless Blades of Waving Grass](https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-7-rendering-countless-blades-waving-grass), [Cyanilux GPU-instanced grass breakdown](https://www.cyanilux.com/tutorials/gpu-instanced-grass-breakdown/), [GPUOpen mesh-shader procedural grass](https://gpuopen.com/learn/mesh_shaders/mesh_shaders-procedural_grass_rendering/), [Ghost of Tsushima-style compute grass](https://www.reddit.com/r/Unity3D/comments/18yl6oh/i_created_individually_simulated_grass_inspired/), [billboard grass overview](https://www.youtube.com/watch?v=Y0Ko0kvwfgA).

Plan (escalating tiers):
- **Tier 1 (v1): GPU-instanced grass cards** — crossed/billboard quads or low-poly blades, per-instance transform in an instanced draw; wind via vertex-shader sine field + gust noise texture
- **Tier 2:** per-blade geometry instancing near the camera (≤ ~50 m), fading to cards at distance
- **Tier 3 (stretch):** compute-shader culling + expansion of blade instances; mesh/task-shader grass generation where supported
- Distance culling + density falloff per chunk; grass never rendered past its LOD radius

### 4.4 Trees & bushes
Research: impostors are the key distant-LOD trick — [octahedral impostors (ShaderBits)](https://shaderbits.com/blog/octahedral-impostors), [GPU Gems 3 Ch.21 True Impostors](https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-21-true-impostors), [InstaLOD foliage impostors](https://instalod.com/2025/04/07/optimizing-foliage-at-scale-imposters-with-instalod/), [Alan Zucconi billboard impostors](https://www.alanzucconi.com/2018/08/25/shader-showcase-saturday-7/).

Plan:
- **3-4 LOD levels**: full mesh → simplified mesh → **octahedral impostor** (bake our own impostor atlas offline with an engine tool — no SpeedTree) → optional single billboard
- **Wind**: hierarchical vertex animation — trunk sway (low-freq) + branch (mid) + leaf flutter (high-freq, per-vertex weights painted into vertex colours); this is the SpeedTree trick, but we author weights in our own pipeline
- Instancing for identical species+LOD within a chunk; per-instance random phase for wind variation

---

## 5. Procedural Generation Pipeline

Research: [3 ways of generating terrain with erosion](https://github.com/dandrino/terrain-erosion-3-ways), [hydraulic erosion course notes](https://courses.tolstenko.net/artificialintelligence/01-pcg/HydraulicErosion/), [real-time planet-scale erosion](https://gamedev.net/blogs/entry/2275217-real-time-planet-scale-erosion), [biome blending](https://gamedev.stackexchange.com/questions/194421/blending-biomes-in-procedural-terrain-generation).

Pipeline (offline tool → cooked data, plus optional runtime mode):
1. **Base heightfield**: fractal (fBm) / ridged noise, seeded
2. **Hydraulic/thermal erosion** sim passes for realism (droplet erosion is the standard; C++ port runs in ms–seconds per tile)
3. **Biome map**: temperature × moisture (Whittaker-style) → biome id per point; smooth blending between biomes
4. **Derived masks**: slope, altitude, curvature, flow/accumulation → feed splat auto-weights *and* vegetation scattering rules
5. Cook to chunk packages: `{heightmap, splatmaps, biome map, veg instance seeds}`

---

## 6. Architecture (engine integration)

```
engine/
└── terrain/
    ├── TerrainWorld        — owns regions, quadtree, streaming
    ├── TerrainChunk        — height data, mesh LODs, AABB, veg instances
    ├── TerrainRenderer     — draw submission, LOD morph params, skirts
    ├── SplatMap / TerrainMaterial
    ├── veg/
    │   ├── VegetationLayer   — species asset + scatter rules
    │   ├── Scatterer         — Poisson disc + rule validation (jobs)
    │   ├── GrassRenderer     — instanced cards/blades, wind
    │   ├── TreeRenderer      — LOD chain + impostor system
    │   └── ImpostorBaker     — offline octahedral atlas tool
    └── gen/
        ├── Noise, Erosion, BiomeMap, MaskGen   — procedural pipeline
        └── Cooker            — bake to chunk packages
```

C++23 features to lean on: `std::mdspan` (heightfield views), `std::generator` (chunk streaming coroutines), `std::expected` (asset/streaming errors), `constexpr` noise/config where possible, modules if the build supports them.

---

## 7. Milestones

| # | Milestone | Deliverable |
|---|---|---|
| M1 | **Heightfield terrain + chunked LOD** | Load heightmap, quadtree LOD with morphing + skirts, fly camera, no cracks/popping |
| M2 | **Splat texturing** | 8-layer height-blended splatting + auto slope/altitude weights |
| M3 | **Grass v1** | Instanced grass cards on one layer, wind shader, distance cull — *first green terrain* |
| M4 | **Scatter system** | Poisson disc + rule validation, deterministic per-chunk seeds, density masks |
| M5 | **Trees v1** | Mesh LOD chain + billboards, instanced rendering, hierarchical wind |
| M6 | **Impostor baker** | Offline octahedral impostor atlases; trees LOD to impostors at range |
| M7 | **Procedural pipeline** | Noise → erosion → biomes → masks → auto-splat + auto-scatter, cooked chunks |
| M8 | **Polish/stretch** | Compute-shader grass, mesh-shader path, virtual texturing evaluation |

---

## 8. Key References

- GPU Gems 1 Ch.7 — waving grass: https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-7-rendering-countless-blades-waving-grass
- GPU Gems 2 Ch.2 — geometry clipmaps: https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry
- GPU Gems 3 Ch.21 — true impostors: https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-21-true-impostors
- Geometry clipmaps in practice: https://mikejsavage.co.uk/geometry-clipmaps/
- Terrain LOD paper survey: http://vterrain.org/LOD/Papers/
- Octahedral impostors: https://shaderbits.com/blog/octahedral-impostors
- GPUOpen mesh-shader grass: https://gpuopen.com/learn/mesh_shaders/mesh_shaders-procedural_grass_rendering/
- Virtual textures talk (2008): https://www.youtube.com/watch?v=MejJL87yNgI
- Terrain erosion 3 ways: https://github.com/dandrino/terrain-erosion-3-ways
- Procedural object scattering rules: https://newheadstudio.com/blog/object_scattering.html
- Terrain rendering overview & tricks: https://kosmonautblog.wordpress.com/2017/06/04/terrain-rendering-overview-and-tricks/

---

## 9. Implementation Status (2026-08-14)

First pass implemented **built into the engine library** — the terrain
sources under `vortxGE/src/terrain/` compile directly into `libvortxGE`
(GLOB_RECURSE, no standalone library). The demo driver itself
(`TerrainDemo.{h,cpp}`) lives with its host in `demos/terrain_demo/src/` —
the lib stays pure terrain module. Tests live in
`tests/src/terrain/terrain_tests.cpp` and run in the unified `tests`
doctest binary via CTest. CPU-side data + algorithms only — no renderer
yet (see milestones M1–M3 for the GPU path).

| Component | Status | Location |
|---|---|---|
| Heightfield + bilinear/gradient sampling | ✅ | `src/terrain/Heightfield.h` |
| OBJ mesh loader (std::expected, dedupe, AABB) | ✅ | `src/terrain/Mesh.{h,cpp}` |
| Example veg meshes (grass clump, fern, bush, broadleaf, conifer) | ✅ | `demos/data/vegetation/*.obj` (+ Blender generator script) |
| Seeded gradient noise, fBm, ridged | ✅ | `src/terrain/Noise.{h,cpp}` |
| Hydraulic droplet erosion (deterministic) | ✅ | `src/terrain/Erosion.{h,cpp}` |
| Whittaker biome classification | ✅ | `src/terrain/Biome.{h,cpp}` |
| Slope / altitude-band masks | ✅ | `src/terrain/Masks.{h,cpp}` |
| Vegetation layers + scatter rules | ✅ | `src/terrain/veg/VegetationLayer.h` |
| Jittered-grid Poisson scatterer, per-chunk deterministic seeds | ✅ | `src/terrain/veg/Scatterer.{h,cpp}` |
| CDLOD-style quadtree LOD selection + morph weights | ✅ | `src/terrain/lod/ChunkLod.{h,cpp}` |
| Terrain render mesh (heightfield → lsMesh grid) | ✅ | `vortxGE/src/terrain/TerrainMesh.{h,cpp}` |
| Bullet heightfield collision (btHeightfieldTerrainShape) | ✅ | `lsBulletShape::Make_Heightfield` |
| Demo scene in vortxGEdemo_glfw3 (VORTX_TERRAIN=1) | ✅ | `demoscene::setupTerrain` — terrain + collision + trees/bushes + rig spawn |
| Chunked render LOD (8×8 chunks × 4 strides, skirts) | ✅ | `generateChunkGridMesh`, per-frame distance bands w/ hysteresis in `demoscene::Update` |
| Per-chunk frustum culling | ✅ | engine bSphere cull; fixed `lsObject::Optimize` treating INTERSECT as culled (was `== INSIDE`, must be `!= OUTSIDE`) |
| Vegetation bake (per-chunk merged static meshes) | ✅ | 800 lsObjects → ≤5 draw calls; grass needs instancing (M3) |

Perf (2026-08-14, RX 7800 XT 1440p): 5.8 fps monolithic+800 objects →
27 fps chunked (baseline scene alone: 42 fps). `VORTX_TERRAIN_TRACE=1`
prints per-second chunk visibility + LOD histogram using the same
sphere/frustum test as the engine cull path.
| GPU-instanced veg / splat / impostors | ⬜ not started | milestones M2–M6 |
| Impostor baker | ⬜ not started | needs a GPU backend |
| Splat texturing | ⬜ not started | needs a GPU backend |

Deviations from the plan above:

1. **`std::mdspan` → custom `Span2D`** — this toolchain's libstdc++ (GCC 15.2,
   Ubuntu) doesn't ship `<mdspan>` yet. `Span2D` in `Heightfield.h` mirrors
   the API surface we need and can be swapped out later.
2. **Bridson Poisson disc → jittered grid** — cell size `minSpacing/√2` gives
   the min-distance guarantee by construction at a fraction of the cost, and
   keeps per-chunk determinism trivial. One fix landed during testing: the
   grid can overshoot a chunk's far edge by up to one cell, so candidates
   outside the chunk rect are clipped (prevents double-placement when the
   neighbour chunk scatters the same position).

Verified: unified `ctest` green (heightfield sampling, noise
determinism/ranges, erosion determinism + no-negative-heights, biome table,
scatterer determinism + rule compliance, LOD full-coverage area invariant).
`terrain_demo` (console demo) generates a 256² terrain (60k erosion droplets),
classifies 10 biomes, scatters ~382k vegetation instances over 8×8 chunks,
and dumps `height.pgm` + `biomes.ppm` to the given output dir. It resolves
`data/vegetation/` via the build-dir data symlink (same mechanism as the
glfw3/gtkmm demos); override with `VORTXGE_RESOURCE_ROOT`.

**Example assets (2026-08-14):** five low-poly vegetation meshes generated
with Blender 5.2 headless — `grass_clump.obj` (15 tris), `fern.obj` (56),
`bush.obj` (64), `broadleaf.obj` (48), `conifer.obj` (40) — in
`demos/data/vegetation/`. Y-up, metres, origin at plant base. Regenerate or
extend with `blender --background --python
blender_scripts/generate_vegetation.py -- <outdir>`. Vegetation layers
reference them via `VegetationLayer::meshPath`; the demo loads each layer's
mesh through the built-in `terrain::loadObj` and reports vert/tri counts.
