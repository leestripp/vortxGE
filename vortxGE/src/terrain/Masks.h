#pragma once

// ============================================================================
// Masks — derived per-point terrain data (Plan.md §5 step 4): slope,
// normalized altitude, and a fuzzy altitude band selector. These feed both
// splat auto-weights and vegetation scattering rules.
// ============================================================================

#include "Heightfield.h"

namespace terrain {

// Slope angle in degrees at normalized (u,v). heightScale converts the
// field's [0,1] heights to world units so the angle is meaningful.
float slopeDegAt(const Heightfield& f, float u, float v, float worldSize,
                 float heightScale);

// Slope as a 0..1 mask (0 = flat, 1 = at/above maxDeg) — handy for splat
// weights (e.g. rock on steep slopes).
float slopeMaskAt(const Heightfield& f, float u, float v, float worldSize,
                  float heightScale, float maxDeg = 45.0f);

// Smooth fuzzy membership of altitude h in [lo,hi] with edge feathering.
float altitudeBand(float h, float lo, float hi, float feather);

} // namespace terrain
