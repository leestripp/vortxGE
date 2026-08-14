#pragma once

// ============================================================================
// Mesh — minimal indexed triangle mesh + Wavefront OBJ loader.
// Built into the engine (no asset plugins). Used by vegetation layers for
// their example/species meshes; a future GPU renderer consumes the same
// vertex/index buffers directly.
//
// Convention: OBJ assets are Y-up, metres, origin at the base of the plant
// (matches resources/vegetation/generate_vegetation.py output).
// ============================================================================

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace terrain {

struct MeshVertex {
    float px = 0.0f, py = 0.0f, pz = 0.0f; // position
    float nx = 0.0f, ny = 1.0f, nz = 0.0f; // normal (defaults up)
};

struct Mesh {
    std::string name;
    std::vector<MeshVertex> vertices;
    std::vector<std::uint32_t> indices; // 3 per triangle

    float aabbMin[3] = {0, 0, 0};
    float aabbMax[3] = {0, 0, 0};

    std::size_t triangleCount() const { return indices.size() / 3; }
    float height() const { return aabbMax[1] - aabbMin[1]; }
};

// Loads v/vn/f records. Faces may be v, v/vt, v//vn or v/vt/vn, 1-based or
// negative (relative); polygons are fan-triangulated. Vertices are deduped
// on the (position, normal) pair.
std::expected<Mesh, std::string> loadObj(const std::string& path);

} // namespace terrain
