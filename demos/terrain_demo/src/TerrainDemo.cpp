// ============================================================================
// TerrainDemo — end-to-end exercise of the terrain module (Plan.md §5):
//   noise → erosion → biomes → masks → vegetation scatter → LOD selection
// Dumps PGM/PPM visualizations to tools/out/ for eyeballing.
// ============================================================================

#include "terrain/Biome.h"
#include "terrain/Erosion.h"
#include "terrain/Heightfield.h"
#include "terrain/Masks.h"
#include "terrain/Noise.h"
#include "terrain/lod/ChunkLod.h"
#include "terrain/veg/Scatterer.h"
#include "terrain/Mesh.h"
#include "TerrainDemo.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <vector>

namespace {

constexpr int kRes = 256;          // heightfield resolution
constexpr float kWorld = 1024.0f;  // world edge length (m)
constexpr float kHeight = 180.0f;  // max elevation (m)

void writePgm(const char* path, const terrain::Heightfield& f) {
    FILE* fp = std::fopen(path, "wb");
    if (!fp) return;
    std::fprintf(fp, "P5\n%zu %zu\n255\n", f.width(), f.height());
    for (std::size_t y = 0; y < f.height(); ++y)
        for (std::size_t x = 0; x < f.width(); ++x) {
            const float h = f(x, y);
            const auto c = static_cast<unsigned char>(
                std::clamp(h, 0.0f, 1.0f) * 255.0f);
            std::fputc(c, fp);
        }
    std::fclose(fp);
}

void writeBiomePpm(const char* path, const std::vector<terrain::Biome>& b, int res) {
    FILE* fp = std::fopen(path, "wb");
    if (!fp) return;
    std::fprintf(fp, "P6\n%d %d\n255\n", res, res);
    // Palette indexed by Biome enum value.
    static const unsigned char kPal[][3] = {
        {30, 60, 160},   // Ocean
        {220, 205, 150}, // Beach
        {230, 200, 90},  // Desert
        {190, 180, 80},  // Savanna
        {110, 170, 70},  // Grassland
        {40, 120, 45},   // Forest
        {35, 95, 70},    // Taiga
        {170, 175, 165}, // Tundra
        {130, 125, 120}, // Rock
        {245, 245, 250}, // Snow
    };
    for (const auto biome : b) {
        const auto& c = kPal[static_cast<int>(biome)];
        std::fputc(c[0], fp);
        std::fputc(c[1], fp);
        std::fputc(c[2], fp);
    }
    std::fclose(fp);
}

} // namespace

namespace terrain {

int run_terrain_demo(const char* out_dir, const char* resource_root) {
    namespace fs = std::filesystem;
    const std::string out = out_dir ? out_dir : "terrain_out";
    const std::string res =
        resource_root ? resource_root : "data/vegetation";
    fs::create_directories(out);

    // -- 1. Base heightfield: fBm + ridged mix -----------------------------
    terrain::Noise noise(1337);
    terrain::Heightfield hf(kRes, kRes);
    for (int y = 0; y < kRes; ++y)
        for (int x = 0; x < kRes; ++x) {
            const float u = static_cast<float>(x) / (kRes - 1);
            const float v = static_cast<float>(y) / (kRes - 1);
            const float base = noise.fbm(u * 4.0f, v * 4.0f, 6) * 0.5f + 0.5f;
            const float ridge = noise.ridged(u * 3.0f + 7.3f, v * 3.0f + 2.1f, 5);
            hf(x, y) = base * 0.65f + ridge * 0.45f;
        }
    hf.normalize();
    std::printf("1. base heightfield %dx%d, range [%.3f, %.3f]\n", kRes, kRes,
                hf.minValue(), hf.maxValue());

    // -- 2. Hydraulic erosion ----------------------------------------------
    terrain::ErosionParams ep;
    ep.seed = 42;
    ep.droplets = 60000;
    terrain::erode(hf, ep);
    hf.normalize();
    std::printf("2. erosion done (%d droplets), range [%.3f, %.3f]\n", ep.droplets,
                hf.minValue(), hf.maxValue());
    writePgm((out + "/height.pgm").c_str(), hf);

    // -- 3. Biomes: temp falls with altitude, moisture from noise ----------
    terrain::Noise climate(9001);
    std::vector<terrain::Biome> biomes(static_cast<std::size_t>(kRes) * kRes);
    int biomeCount[static_cast<int>(terrain::Biome::Count)] = {};
    for (int y = 0; y < kRes; ++y)
        for (int x = 0; x < kRes; ++x) {
            const float u = static_cast<float>(x) / (kRes - 1);
            const float v = static_cast<float>(y) / (kRes - 1);
            const float h = hf(x, y);
            const float temp = std::clamp(
                0.75f - h * 0.55f + climate.fbm(u * 2.0f, v * 2.0f, 3) * 0.08f, 0.0f,
                1.0f);
            const float moist = std::clamp(
                0.5f + climate.fbm(u * 3.0f + 11.0f, v * 3.0f + 5.0f, 4) * 0.6f, 0.0f,
                1.0f);
            const auto b = terrain::classifyBiome(h, temp, moist);
            biomes[static_cast<std::size_t>(y) * kRes + x] = b;
            ++biomeCount[static_cast<int>(b)];
        }
    std::printf("3. biomes:");
    for (int i = 0; i < static_cast<int>(terrain::Biome::Count); ++i)
        if (biomeCount[i] > 0)
            std::printf(" %s=%d", terrain::biomeName(static_cast<terrain::Biome>(i)),
                        biomeCount[i]);
    std::printf("\n");
    writeBiomePpm((out + "/biomes.ppm").c_str(), biomes, kRes);

    // -- 4. Vegetation scatter (3 layers, 8x8 chunks) ----------------------
    terrain::veg::Scatterer sc(hf, biomes, kWorld, kHeight);

    terrain::veg::VegetationLayer grass;
    grass.id = 1;
    grass.name = "grass";
    grass.meshPath = res + "/grass_clump.obj";
    grass.rules.minAltitude = 0.31f;
    grass.rules.maxAltitude = 0.80f;
    grass.rules.maxSlopeDeg = 32.0f;
    grass.rules.minSpacing = 1.5f;
    grass.rules.density = 0.85f;
    grass.rules.biomeMask = terrain::biomeBit(terrain::Biome::Grassland) |
                            terrain::biomeBit(terrain::Biome::Forest) |
                            terrain::biomeBit(terrain::Biome::Savanna);

    terrain::veg::VegetationLayer bush;
    bush.id = 2;
    bush.name = "bush";
    bush.meshPath = res + "/bush.obj";
    bush.rules.minAltitude = 0.31f;
    bush.rules.maxAltitude = 0.75f;
    bush.rules.maxSlopeDeg = 28.0f;
    bush.rules.minSpacing = 4.0f;
    bush.rules.density = 0.5f;
    bush.rules.biomeMask = terrain::biomeBit(terrain::Biome::Grassland) |
                           terrain::biomeBit(terrain::Biome::Forest) |
                           terrain::biomeBit(terrain::Biome::Taiga);

    terrain::veg::VegetationLayer tree;
    tree.id = 3;
    tree.name = "tree";
    tree.meshPath = res + "/broadleaf.obj";
    tree.rules.minAltitude = 0.32f;
    tree.rules.maxAltitude = 0.82f;
    tree.rules.maxSlopeDeg = 30.0f;
    tree.rules.minSpacing = 8.0f;
    tree.rules.density = 0.6f;
    tree.rules.minScale = 0.9f;
    tree.rules.maxScale = 1.6f;
    tree.rules.biomeMask = terrain::biomeBit(terrain::Biome::Forest) |
                           terrain::biomeBit(terrain::Biome::Taiga);

    // Load each layer's example mesh (generated by
    // blender_scripts/generate_vegetation.py via Blender headless).
    for (auto* layer : {&grass, &bush, &tree}) {
        auto mesh = terrain::loadObj(layer->meshPath);
        if (!mesh) {
            std::printf("4. ERROR loading %s: %s\n", layer->meshPath.c_str(),
                        mesh.error().c_str());
            return 1;
        }
        std::printf("4. mesh %-16s %5zu verts %4zu tris  height %.2f m\n",
                    mesh->name.c_str(), mesh->vertices.size(), mesh->triangleCount(),
                    mesh->height());
    }

    constexpr int kChunks = 8;
    for (const auto* layer : {&grass, &bush, &tree}) {
        std::size_t total = 0;
        for (int cy = 0; cy < kChunks; ++cy)
            for (int cx = 0; cx < kChunks; ++cx)
                total += sc.scatter(*layer, cx, cy, kChunks, /*globalSeed=*/7).size();
        std::printf("4. layer %-6s → %zu instances over %dx%d chunks\n",
                    layer->name.c_str(), total, kChunks, kChunks);
    }

    // -- 5. LOD selection from a camera near the middle --------------------
    terrain::lod::ChunkLod lod(kWorld, /*maxLevel=*/5);
    const auto sel = lod.select(kWorld * 0.5f, kWorld * 0.5f);
    int lvlHist[8] = {};
    for (const auto& n : sel.nodes)
        ++lvlHist[n.level];
    std::printf("5. LOD: %zu nodes (levels:", sel.nodes.size());
    for (int i = 0; i <= sel.maxLevel; ++i)
        std::printf(" L%d=%d", i, lvlHist[i]);
    std::printf(")\n");

    std::printf("DONE — images in %s/\n", out.c_str());
    return 0;
}

} // namespace terrain
