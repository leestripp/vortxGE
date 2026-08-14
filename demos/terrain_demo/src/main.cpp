// ============================================================================
// terrain_demo — console host for the engine's built-in terrain pipeline
// (terrain::run_terrain_demo, vortxGE/src/terrain/TerrainDemo.{h,cpp}).
// Usage: terrain_demo [outdir]   (run from the repo root so
// resources/vegetation/ resolves, or pass VORTXGE_RESOURCE_ROOT)
// ============================================================================

#include "TerrainDemo.h"

#include <cstdlib>

int main(int argc, char* argv[]) {
    const char* out = argc > 1 ? argv[1] : nullptr;
    const char* res = std::getenv("VORTXGE_RESOURCE_ROOT"); // optional override
    return terrain::run_terrain_demo(out, res);
}
