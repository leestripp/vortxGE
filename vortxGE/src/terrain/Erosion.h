#pragma once

// ============================================================================
// Erosion — hydraulic droplet erosion over a Heightfield (Plan.md §5 step 2).
// Classic droplet simulation (Hans Theobald Beyer's implementation of the
// particle-based approach): each droplet descends the gradient, picks up
// sediment while accelerating downhill, and deposits when capacity is
// exceeded or it climbs. Deterministic for a given seed.
// ============================================================================

#include "Heightfield.h"

#include <cstdint>

namespace terrain {

struct ErosionParams {
    std::uint64_t seed = 1;
    int droplets = 50000;       // number of simulated water droplets
    int maxLifetime = 30;       // max steps per droplet
    float inertia = 0.05f;      // direction retention (0 = follows gradient exactly)
    float sedimentCapacity = 4.0f; // capacity factor
    float minSlope = 0.01f;     // capacity floor on flat ground
    float erodeSpeed = 0.3f;    // [0,1] erosion rate
    float depositSpeed = 0.3f;  // [0,1] deposition rate
    float evaporateSpeed = 0.01f; // [0,1] water loss per step
    float gravity = 4.0f;
    int brushRadius = 3;        // erosion brush radius in cells
};

// In-place. Expects height values roughly in [0,1]; call normalize() after.
void erode(Heightfield& field, const ErosionParams& params);

} // namespace terrain
