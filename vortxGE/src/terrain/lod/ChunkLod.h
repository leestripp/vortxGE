#pragma once

// ============================================================================
// ChunkLod — CDLOD-style chunked quadtree LOD selection (Plan.md §2).
//
// Pure CPU-side selection: given a camera position, walk the quadtree and
// emit the set of nodes that cover the world at appropriate detail. The GPU
// morph weight for each node is derived from the camera's position inside
// the node's "morph band" (the outer fraction of its visibility range), so
// vertices can blend toward the next-coarser level instead of popping.
// ============================================================================

#include <cstdint>
#include <vector>

namespace terrain::lod {

struct LodNode {
    int level = 0;    // 0 = coarsest (root)
    int x = 0;        // node coords within its level
    int y = 0;
    float morph = 0.0f; // [0,1] — 0 = fully this level, 1 = fully coarser
};

struct LodSelection {
    std::vector<LodNode> nodes;
    int maxLevel = 0;
};

class ChunkLod {
public:
    // worldSize: edge length of the square world. maxLevel: deepest level,
    // giving (2^maxLevel)^2 finest chunks. morphStart: fraction of a node's
    // visibility range (from the far side) where morphing begins, e.g. 0.3.
    ChunkLod(float worldSize, int maxLevel, float morphStart = 0.3f);

    LodSelection select(float camX, float camY) const;

    // Visibility range for a node at `level` (distance from node AABB
    // within which this level is shown before refining).
    float levelRange(int level) const;

    float nodeSize(int level) const;

private:
    void visit(int level, int x, int y, float camX, float camY,
               LodSelection& out) const;

    float worldSize_;
    int maxLevel_;
    float morphStart_;
    float baseRange_; // range of a level-0 node
};

} // namespace terrain::lod
