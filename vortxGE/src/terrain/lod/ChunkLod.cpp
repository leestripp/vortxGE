#include "ChunkLod.h"

#include <algorithm>
#include <cmath>

namespace terrain::lod {

ChunkLod::ChunkLod(float worldSize, int maxLevel, float morphStart)
    : worldSize_(worldSize), maxLevel_(maxLevel), morphStart_(morphStart) {
    // A node refines when the camera comes within `baseRange_ * 2^-level`
    // of its AABB. Base range is one node-size of the root.
    baseRange_ = worldSize;
}

float ChunkLod::nodeSize(int level) const {
    return worldSize_ / static_cast<float>(1 << level);
}

float ChunkLod::levelRange(int level) const {
    return baseRange_ / static_cast<float>(1 << level);
}

void ChunkLod::visit(int level, int x, int y, float camX, float camY,
                     LodSelection& out) const {
    const float size = nodeSize(level);
    const float minX = static_cast<float>(x) * size;
    const float minY = static_cast<float>(y) * size;
    const float maxX = minX + size;
    const float maxY = minY + size;

    // Distance from camera to node AABB (0 when inside).
    const float dx = std::max({minX - camX, 0.0f, camX - maxX});
    const float dy = std::max({minY - camY, 0.0f, camY - maxY});
    const float dist = std::sqrt(dx * dx + dy * dy);

    const float range = levelRange(level);

    if (level < maxLevel_ && dist < range * 0.5f) {
        // Too close for this level — refine into 4 children.
        for (int cy = 0; cy < 2; ++cy)
            for (int cx = 0; cx < 2; ++cx)
                visit(level + 1, x * 2 + cx, y * 2 + cy, camX, camY, out);
        return;
    }

    // Leaf for this view. Morph weight: 1 at the *inner* edge of the band
    // (about to refine/coarsen) → 0 at the outer edge. Hysteresis-free
    // single threshold keeps selection and morph consistent.
    LodNode node;
    node.level = level;
    node.x = x;
    node.y = y;
    const float inner = range * 0.5f;
    const float outer = range * (0.5f + morphStart_ * 0.5f);
    node.morph = (level == maxLevel_)
                     ? 0.0f
                     : std::clamp(1.0f - (dist - inner) / std::max(outer - inner, 1e-6f),
                                  0.0f, 1.0f);
    out.nodes.push_back(node);
}

LodSelection ChunkLod::select(float camX, float camY) const {
    LodSelection out;
    out.maxLevel = maxLevel_;
    visit(0, 0, 0, camX, camY, out);
    return out;
}

} // namespace terrain::lod
