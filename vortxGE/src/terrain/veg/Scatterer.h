#pragma once

// ============================================================================
// Scatterer — deterministic per-chunk vegetation placement (Plan.md §4.2).
//
// Candidates come from a jittered grid with cell size = minSpacing/sqrt(2),
// which guarantees the Poisson min-distance property by construction. Each
// candidate is validated against the stacked rules (altitude, slope, biome,
// optional density mask) and accepted with probability = density.
//
// Determinism: the RNG is seeded from hash(chunk coords, layer id, global
// seed), so a chunk re-scatters identically every time it streams in — no
// instance data needs to be persisted for procedural areas.
// ============================================================================

#include "../Biome.h"
#include "../Heightfield.h"
#include "VegetationLayer.h"

#include <cstdint>
#include <vector>

namespace terrain::veg {

struct Instance {
    float x = 0.0f;      // world X
    float y = 0.0f;      // world Z (ground plane)
    float z = 0.0f;      // world Y = terrain height (elevation)
    float rotation = 0.0f; // yaw, radians
    float scale = 1.0f;
    std::uint32_t layerId = 0;
};

class Scatterer {
public:
    // height: normalized [0,1] heightfield. biomes: per-cell biome map with
    // the same dimensions. worldSize: edge length of the square world in
    // world units. heightScale: world-unit height of a 1.0 field value.
    Scatterer(const Heightfield& height, const std::vector<Biome>& biomes,
              float worldSize, float heightScale);

    // Scatter one layer over one chunk. chunksAcross = number of chunks per
    // world edge (chunk world size = worldSize / chunksAcross).
    // densityMask (optional): extra [0,1] multiplier, e.g. hand-painted.
    std::vector<Instance> scatter(const VegetationLayer& layer, int chunkX, int chunkY,
                                  int chunksAcross, std::uint64_t globalSeed = 0,
                                  const Heightfield* densityMask = nullptr) const;

private:
    bool validate(const ScatterRules& rules, float u, float v,
                  const Heightfield* densityMask) const;

    const Heightfield& height_;
    const std::vector<Biome>& biomes_;
    float worldSize_;
    float heightScale_;
};

} // namespace terrain::veg
