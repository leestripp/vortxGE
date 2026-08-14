#include "Erosion.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace terrain {

namespace {

// SplitMix64 — small deterministic PRNG (same family as the Noise hash).
struct Rng {
    std::uint64_t s;
    explicit Rng(std::uint64_t seed) : s(seed) {}
    std::uint64_t next() {
        std::uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
    float uniform() { return static_cast<float>(next() >> 40) * (1.0f / 16777216.0f); }
};

} // namespace

void erode(Heightfield& field, const ErosionParams& p) {
    const int w = static_cast<int>(field.width());
    const int h = static_cast<int>(field.height());
    const int border = p.brushRadius + 1;

    // Precompute the circular erosion brush: offsets + weights within radius.
    std::vector<int> brushOffX;
    std::vector<int> brushOffY;
    std::vector<float> brushW;
    float wsum = 0.0f;
    for (int dy = -p.brushRadius; dy <= p.brushRadius; ++dy) {
        for (int dx = -p.brushRadius; dx <= p.brushRadius; ++dx) {
            const float d = std::sqrt(static_cast<float>(dx * dx + dy * dy));
            if (d <= static_cast<float>(p.brushRadius)) {
                const float wt = static_cast<float>(p.brushRadius) - d;
                brushOffX.push_back(dx);
                brushOffY.push_back(dy);
                brushW.push_back(wt);
                wsum += wt;
            }
        }
    }
    for (float& wt : brushW)
        wt /= wsum;

    auto at = [&](int x, int y) -> float& {
        return field(static_cast<std::size_t>(x), static_cast<std::size_t>(y));
    };

    // Bilinear height + gradient in *cell* space (x,y in [0,w-1]x[0,h-1]).
    auto sample = [&](float x, float y) -> float {
        return field.sampleBilinear(x / static_cast<float>(w - 1),
                                    y / static_cast<float>(h - 1));
    };
    auto grad = [&](float x, float y, float& gx, float& gy) {
        const float e = 1.0f;
        gx = (sample(x + e, y) - sample(x - e, y)) * 0.5f;
        gy = (sample(x, y + e) - sample(x, y - e)) * 0.5f;
    };

    Rng rng(p.seed);

    for (int i = 0; i < p.droplets; ++i) {
        float posX = rng.uniform() * static_cast<float>(w - 1 - 2 * border) + static_cast<float>(border);
        float posY = rng.uniform() * static_cast<float>(h - 1 - 2 * border) + static_cast<float>(border);
        float dirX = 0.0f;
        float dirY = 0.0f;
        float speed = 1.0f;
        float water = 1.0f;
        float sediment = 0.0f;

        for (int lifetime = 0; lifetime < p.maxLifetime; ++lifetime) {
            const int nodeX = static_cast<int>(posX);
            const int nodeY = static_cast<int>(posY);
            if (nodeX < border || nodeY < border || nodeX >= w - border || nodeY >= h - border)
                break;

            float gx = 0.0f;
            float gy = 0.0f;
            grad(posX, posY, gx, gy);
            const float heightOld = sample(posX, posY);

            // Update direction with inertia, then move one step.
            dirX = dirX * p.inertia - gx * (1.0f - p.inertia);
            dirY = dirY * p.inertia - gy * (1.0f - p.inertia);
            const float len = std::sqrt(dirX * dirX + dirY * dirY);
            if (len <= 1e-6f)
                break; // stuck in a pit — evaporate
            dirX /= len;
            dirY /= len;
            posX += dirX;
            posY += dirY;

            if (posX < 1.0f || posY < 1.0f || posX >= static_cast<float>(w - 2) ||
                posY >= static_cast<float>(h - 2))
                break;

            const float heightNew = sample(posX, posY);
            const float deltaH = heightNew - heightOld;

            const float capacity =
                std::max(-deltaH * speed * water * p.sedimentCapacity, p.minSlope);

            if (sediment > capacity || deltaH > 0.0f) {
                // Deposit (when moving uphill, deposit what lifts us out).
                float amount = (deltaH > 0.0f) ? std::min(deltaH, sediment)
                                               : (sediment - capacity) * p.depositSpeed;
                amount = std::max(amount, 0.0f);
                sediment -= amount;
                at(nodeX, nodeY) += amount * 0.25f; // crude 4-tap spread
                at(nodeX + 1, nodeY) += amount * 0.25f;
                at(nodeX, nodeY + 1) += amount * 0.25f;
                at(nodeX + 1, nodeY + 1) += amount * 0.25f;
            } else {
                // Erode with the precomputed brush.
                const float amount =
                    std::min((capacity - sediment) * p.erodeSpeed, -deltaH);
                if (amount > 0.0f) {
                    for (std::size_t b = 0; b < brushW.size(); ++b) {
                        float& cell = at(nodeX + brushOffX[b], nodeY + brushOffY[b]);
                        const float weighed = amount * brushW[b];
                        const float delta = std::min(cell, weighed);
                        cell -= delta;
                        sediment += delta;
                    }
                }
            }

            speed = std::sqrt(std::max(0.0f, speed * speed + (-deltaH) * p.gravity));
            water *= (1.0f - p.evaporateSpeed);
        }
    }
}

} // namespace terrain
