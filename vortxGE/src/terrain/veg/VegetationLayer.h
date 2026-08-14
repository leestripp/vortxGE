#pragma once

// ============================================================================
// VegetationLayer — species/layer asset + scattering rules (Plan.md §4.1).
// A layer describes one species (grass tuft, fern, bush, tree...) and the
// conditions under which it may grow.
// ============================================================================

#include <cstdint>
#include <string>

namespace terrain::veg {

struct ScatterRules {
    float minAltitude = 0.0f;   // normalized [0,1]
    float maxAltitude = 1.0f;
    float maxSlopeDeg = 45.0f;  // reject candidates on steeper ground
    std::uint32_t biomeMask = 0xFFFFFFFFu; // bit per terrain::Biome
    float minSpacing = 2.0f;    // world units — Poisson radius
    float density = 1.0f;       // [0,1] probability of accepting a valid candidate
    float minScale = 0.8f;      // per-instance uniform scale range
    float maxScale = 1.2f;
};

struct VegetationLayer {
    std::uint32_t id = 0;       // feeds the deterministic chunk seed
    std::string name;
    std::string meshPath;       // OBJ asset, e.g. resources/vegetation/grass_clump.obj
    ScatterRules rules;
};

} // namespace terrain::veg
