#include "Scatterer.h"

#include "../Masks.h"

#include <algorithm>
#include <cmath>

namespace terrain::veg {

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kSqrt2 = 1.41421356237309504880f;

std::uint64_t mix64(std::uint64_t z) {
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

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

Scatterer::Scatterer(const Heightfield& height, const std::vector<Biome>& biomes,
                     float worldSize, float heightScale)
    : height_(height), biomes_(biomes), worldSize_(worldSize), heightScale_(heightScale) {}

bool Scatterer::validate(const ScatterRules& rules, float u, float v,
                         const Heightfield* densityMask) const {
    const float h = height_.sampleBilinear(u, v);
    if (h < rules.minAltitude || h > rules.maxAltitude)
        return false;

    if (rules.maxSlopeDeg < 90.0f &&
        slopeDegAt(height_, u, v, worldSize_, heightScale_) > rules.maxSlopeDeg)
        return false;

    if (rules.biomeMask != 0xFFFFFFFFu && !biomes_.empty()) {
        const std::size_t bx = std::min(
            static_cast<std::size_t>(u * static_cast<float>(height_.width())),
            height_.width() - 1);
        const std::size_t by = std::min(
            static_cast<std::size_t>(v * static_cast<float>(height_.height())),
            height_.height() - 1);
        const Biome b = biomes_[by * height_.width() + bx];
        if ((rules.biomeMask & biomeBit(b)) == 0)
            return false;
    }

    if (densityMask && densityMask->sampleBilinear(u, v) <= 0.0f)
        return false;

    return true;
}

std::vector<Instance> Scatterer::scatter(const VegetationLayer& layer, int chunkX,
                                         int chunkY, int chunksAcross,
                                         std::uint64_t globalSeed,
                                         const Heightfield* densityMask) const {
    const ScatterRules& rules = layer.rules;
    const float chunkSize = worldSize_ / static_cast<float>(chunksAcross);
    const float originX = static_cast<float>(chunkX) * chunkSize;
    const float originY = static_cast<float>(chunkY) * chunkSize;

    // Jittered grid: cell diagonal == minSpacing → min distance guaranteed.
    const float cell = rules.minSpacing / kSqrt2;
    const int cellsPerEdge = std::max(1, static_cast<int>(std::ceil(chunkSize / cell)));

    Rng rng(mix64(globalSeed ^ mix64(static_cast<std::uint64_t>(chunkX) * 0x9E3779B1ULL ^
                                     mix64(static_cast<std::uint64_t>(chunkY) * 0x85EBCA77ULL ^
                                           mix64(layer.id)))));

    std::vector<Instance> out;
    out.reserve(static_cast<std::size_t>(cellsPerEdge) *
                static_cast<std::size_t>(cellsPerEdge) / 4);

    for (int cy = 0; cy < cellsPerEdge; ++cy) {
        for (int cx = 0; cx < cellsPerEdge; ++cx) {
            const float jx = rng.uniform();
            const float jy = rng.uniform();
            const float rollDensity = rng.uniform();
            const float rollRot = rng.uniform();
            const float rollScale = rng.uniform();

            const float wx = originX + (static_cast<float>(cx) + jx) * cell;
            const float wy = originY + (static_cast<float>(cy) + jy) * cell;
            // Clip to the chunk: the jittered grid can overshoot the far
            // edge by up to one cell, which would double-place instances
            // when the neighbour chunk scatters the same world position.
            if (wx >= originX + chunkSize || wy >= originY + chunkSize)
                continue;

            const float u = wx / worldSize_;
            const float v = wy / worldSize_;
            if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
                continue;

            if (!validate(rules, u, v, densityMask))
                continue;

            // Density: base probability, modulated by the optional mask.
            float p = std::clamp(rules.density, 0.0f, 1.0f);
            if (densityMask)
                p *= std::clamp(densityMask->sampleBilinear(u, v), 0.0f, 1.0f);
            if (rollDensity > p)
                continue;

            Instance inst;
            inst.x = wx;
            inst.y = wy;
            inst.z = height_.sampleBilinear(u, v) * heightScale_;
            inst.rotation = rollRot * 2.0f * kPi;
            inst.scale = std::lerp(rules.minScale, rules.maxScale, rollScale);
            inst.layerId = layer.id;
            out.push_back(inst);
        }
    }
    return out;
}

} // namespace terrain::veg
