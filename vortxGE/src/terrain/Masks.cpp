#include "Masks.h"

#include <algorithm>
#include <cmath>

namespace terrain {

namespace {

constexpr float kPi = 3.14159265358979323846f;

float smoothstep(float e0, float e1, float x) {
    const float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // namespace

float slopeDegAt(const Heightfield& f, float u, float v, float worldSize,
                 float heightScale) {
    const Vec2 g = f.gradient(u, v, worldSize);
    const float dzdx = g.x * heightScale;
    const float dzdy = g.y * heightScale;
    return std::atan(std::sqrt(dzdx * dzdx + dzdy * dzdy)) * (180.0f / kPi);
}

float slopeMaskAt(const Heightfield& f, float u, float v, float worldSize,
                  float heightScale, float maxDeg) {
    return std::clamp(slopeDegAt(f, u, v, worldSize, heightScale) / maxDeg, 0.0f, 1.0f);
}

float altitudeBand(float h, float lo, float hi, float feather) {
    return smoothstep(lo - feather, lo + feather, h) *
           (1.0f - smoothstep(hi - feather, hi + feather, h));
}

} // namespace terrain
