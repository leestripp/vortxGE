#pragma once

// ============================================================================
// TerrainMesh — bridges the CPU terrain module to engine datablocks.
// generateGridMesh(): pure geometry (unit-tested, no GL).
// uploadGridMesh()/uploadVegetationMesh(): lsMesh construction for rendering.
// ============================================================================

#include <cstdint>
#include <vector>

class lsMesh;
class lsRender;
class lsShader;

namespace terrain {

class Heightfield; // Heightfield.h uses `class Heightfield`-style; adjust if struct
struct Mesh;

struct GridMesh {
    std::vector<float> positions;       // xyz per vertex, centred on origin
    std::vector<float> normals;         // xyz per vertex (unit)
    std::vector<float> uvs;             // uv per vertex, [0,1] over the field
    std::vector<std::uint32_t> indices; // 0-based, 3 per triangle
    float worldSize = 0.0f;
    float maxHeight = 0.0f;
};

/// Build a render grid from a heightfield. The grid is CENTRED on the
/// origin (x/z in [-worldSize/2, +worldSize/2]) so it lines up with
/// btHeightfieldTerrainShape, which is centre-origin too.
GridMesh generateGridMesh( const Heightfield& hf, float worldSize,
                           float maxHeight );

/// Upload a grid to an lsMesh (one lsBatch_Mesh, GL buffers copied).
lsMesh* uploadGridMesh( const GridMesh& grid, lsRender* render,
                        lsShader* shader = nullptr );

/// Upload a terrain::Mesh (OBJ vegetation) to an lsMesh.
lsMesh* uploadVegetationMesh( const Mesh& mesh, lsRender* render,
                              lsShader* shader = nullptr );

} // namespace terrain
