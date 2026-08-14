#pragma once

// ============================================================================
// Noise — deterministic seeded 2D gradient noise (Perlin-style), fBm and
// ridged multifractal. Hash-driven gradients (no permutation tables) so a
// seed reproduces the same field on any platform (Plan.md §5 step 1).
// ============================================================================

#include <cstdint>

namespace terrain {

class Noise {
public:
    explicit Noise(std::uint64_t seed);

    float noise(float x, float y) const; // [-1, 1]
    float fbm(float x, float y, int octaves, float lacunarity = 2.0f,
              float gain = 0.5f) const; // ~[-1, 1]
    float ridged(float x, float y, int octaves, float lacunarity = 2.0f,
                 float gain = 0.5f) const; // [0, 1]

private:
    std::uint64_t seed_;
    float cornerGradient(int ix, int iy, float fx, float fy) const;
};

} // namespace terrain
