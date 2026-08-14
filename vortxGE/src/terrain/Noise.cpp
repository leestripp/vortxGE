#include "Noise.h"

#include <algorithm>
#include <cmath>

namespace terrain {

namespace {

std::uint64_t mix64(std::uint64_t z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

float fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

} // namespace

Noise::Noise(std::uint64_t seed) : seed_(seed) {}

float Noise::cornerGradient(int ix, int iy, float fx, float fy) const {
    const std::uint64_t hx = mix64(static_cast<std::uint64_t>(static_cast<std::int64_t>(ix)));
    const std::uint64_t hy = mix64(static_cast<std::uint64_t>(static_cast<std::int64_t>(iy)));
    const std::uint64_t h = mix64(seed_ ^ (hx * 0x9E3779B97F4A7C15ULL + hy));
    static constexpr float kGrad[8][2] = {
        {1, 1}, {-1, 1}, {1, -1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1},
    };
    const auto& g = kGrad[h & 7u];
    return g[0] * fx + g[1] * fy;
}

float Noise::noise(float x, float y) const {
    const int ix = static_cast<int>(std::floor(x));
    const int iy = static_cast<int>(std::floor(y));
    const float fx = x - std::floor(x);
    const float fy = y - std::floor(y);
    const float u = fade(fx);
    const float v = fade(fy);
    const float n00 = cornerGradient(ix, iy, fx, fy);
    const float n10 = cornerGradient(ix + 1, iy, fx - 1.0f, fy);
    const float n01 = cornerGradient(ix, iy + 1, fx, fy - 1.0f);
    const float n11 = cornerGradient(ix + 1, iy + 1, fx - 1.0f, fy - 1.0f);
    const float nx0 = std::lerp(n00, n10, u);
    const float nx1 = std::lerp(n01, n11, u);
    // 1/|grad| worst case for the 8-direction set is ~1/sqrt(2); scale back up.
    return std::clamp(std::lerp(nx0, nx1, v) * 1.4142f, -1.0f, 1.0f);
}

float Noise::fbm(float x, float y, int octaves, float lacunarity, float gain) const {
    float sum = 0.0f;
    float amp = 1.0f;
    float freq = 1.0f;
    float norm = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        sum += noise(x * freq, y * freq) * amp;
        norm += amp;
        amp *= gain;
        freq *= lacunarity;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}

float Noise::ridged(float x, float y, int octaves, float lacunarity, float gain) const {
    float sum = 0.0f;
    float amp = 0.5f;
    float freq = 1.0f;
    float norm = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        const float n = 1.0f - std::fabs(noise(x * freq, y * freq));
        sum += n * n * amp;
        norm += amp;
        amp *= gain;
        freq *= lacunarity;
    }
    return norm > 0.0f ? sum / norm : 0.0f;
}

} // namespace terrain
