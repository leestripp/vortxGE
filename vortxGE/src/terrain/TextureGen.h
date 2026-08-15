#pragma once

// ============================================================================
// TextureGen — procedural tiling terrain splat layers (Plan.md T2).
//
// Generates 512x512 RGBA8 textures at startup from terrain::Noise fBm:
//   RGB = albedo, A = per-texture height/depth for height-based splat
//   blending (Plan.md T3, Mishkinis "Advanced Terrain Texture Splatting").
//
// Layers: 0 grass, 1 dirt, 2 rock, 3 sand. All textures tile seamlessly
// (noise sampled on a torus: x and y wrap by construction via periodic
// lattice hashing — we simply sample the noise field with coordinates
// scaled so the lattice period divides the texture size).
// ============================================================================

#include <cstdint>
#include <vector>

namespace terrain {

struct SplatLayer {
    int size = 0;                    // width == height
    std::vector<std::uint8_t> rgba;  // size*size*4
};

/// Generate one layer. kind: 0 grass, 1 dirt, 2 rock, 3 sand.
SplatLayer generateSplatLayer( int kind, int size = 512 );

} // namespace terrain
