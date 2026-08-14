#include "Biome.h"

namespace terrain {

Biome classifyBiome(float altitude, float temperature, float moisture) {
    // Water and high-altitude bands first.
    if (altitude < 0.30f)
        return Biome::Ocean;
    if (altitude < 0.33f)
        return Biome::Beach;
    if (altitude > 0.92f)
        return Biome::Snow;
    if (altitude > 0.85f)
        return Biome::Rock;

    // Whittaker-style temperature x moisture table.
    if (temperature < 0.15f)
        return Biome::Tundra;
    if (temperature < 0.35f)
        return moisture > 0.4f ? Biome::Taiga : Biome::Tundra;
    if (temperature < 0.65f)
        return moisture > 0.5f ? Biome::Forest : Biome::Grassland;
    // Hot band.
    if (moisture < 0.33f)
        return Biome::Desert;
    if (moisture < 0.66f)
        return Biome::Savanna;
    return Biome::Forest; // tropical forest
}

const char* biomeName(Biome b) {
    switch (b) {
    case Biome::Ocean: return "Ocean";
    case Biome::Beach: return "Beach";
    case Biome::Desert: return "Desert";
    case Biome::Savanna: return "Savanna";
    case Biome::Grassland: return "Grassland";
    case Biome::Forest: return "Forest";
    case Biome::Taiga: return "Taiga";
    case Biome::Tundra: return "Tundra";
    case Biome::Rock: return "Rock";
    case Biome::Snow: return "Snow";
    case Biome::Count: break;
    }
    return "Unknown";
}

} // namespace terrain
