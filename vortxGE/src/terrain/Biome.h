#pragma once

// ============================================================================
// Biome — Whittaker-style biome classification from altitude / temperature /
// moisture (Plan.md §5 step 3).
// ============================================================================

#include <cstdint>

namespace terrain {

enum class Biome : std::uint8_t {
    Ocean = 0,
    Beach,
    Desert,
    Savanna,
    Grassland,
    Forest,
    Taiga,
    Tundra,
    Rock,
    Snow,
    Count,
};

constexpr std::uint32_t biomeBit(Biome b) { return 1u << static_cast<std::uint8_t>(b); }

// altitude, temperature, moisture all normalized [0,1]
// (temperature: 0 = coldest, moisture: 0 = driest).
Biome classifyBiome(float altitude, float temperature, float moisture,
                    float seaLevel = 0.30f);

const char* biomeName(Biome b);

} // namespace terrain
