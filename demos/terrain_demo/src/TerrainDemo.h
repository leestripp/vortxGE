#pragma once

// ============================================================================
// TerrainDemo — built-in engine entry point for the terrain pipeline.
// Runs the full chain (noise → erosion → biomes → masks → scatter → LOD)
// and dumps visualizations. Hosted by demos/terrain_demo (console demo).
// ============================================================================

namespace terrain {

// Returns a process exit code (0 = success).
// resource_root: directory containing the vegetation OBJs
// (default: "resources/vegetation", relative to CWD).
int run_terrain_demo(const char* out_dir, const char* resource_root = nullptr);

} // namespace terrain
