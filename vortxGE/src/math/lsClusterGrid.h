/**
 * @file lsClusterGrid.h
 * @brief Clustered forward light assignment (Plan.md Part 4, L5.1).
 *
 * Pure CPU math, no engine/render dependencies (doctest-friendly):
 * a 3D grid of screen tiles (16 px) x exponential depth slices in view
 * space. Per frame the caller hands in the view-space light list and
 * gets back a (offset,count) grid + light index list for GPU upload
 * (L5.2 storage lives elsewhere).
 *
 * Direct lights are NOT assigned to clusters (they are global); the
 * caller keeps them in a separate small list.
 */
#pragma once

#include <cstdint>
#include <vector>

#include "lsVector3.h"

/// One light prepared for cluster assignment (view space)
struct lsClusterLightInput
{
    lsVector3 position;        ///< view-space position
    lsVector3 direction;       ///< view-space direction (spots)
    float     radius = 1.0f;   ///< effective range (derived from intensity)
    int       type = 0;        ///< LSLT_POINT / LSLT_SPOT (direct excluded)
    float     cosOuter = -1.0f;///< spot outer cone cos (-1 = point)
    int       lightIndex = 0;  ///< index into the GPU light array
};

class lsClusterGrid
{
public:
    /// tilePixels: screen tile edge; slices: exponential depth slices
    void setup( int viewportW, int viewportH, float tilePixels,
                float nearZ, float farZ, int slices,
                float tanHalfFovX, float tanHalfFovY );

    /// Exponential slice for a view-space depth (positive distance)
    int sliceFromViewDepth( float viewDepth ) const;

    /// Assign all point/spot lights to overlapping clusters.
    /// Direct-type lights (type==3) are skipped by design.
    void build( const std::vector<lsClusterLightInput>& lights );

    /// (offset,count) pairs into m_indices, 2 uints per cluster
    const std::vector<uint32_t>& grid() const { return m_grid; }
    /// light indices, clustered
    const std::vector<uint32_t>& indices() const { return m_indices; }

    int tilesX() const { return m_tilesX; }
    int tilesY() const { return m_tilesY; }
    int slices() const { return m_slices; }
    int clusterCount() const { return m_tilesX * m_tilesY * m_slices; }

    /// cluster index from tile + slice
    int clusterIndex( int tx, int ty, int slice ) const
    { return ( slice * m_tilesY + ty ) * m_tilesX + tx; }

    /// view-space AABB of a cluster (out params; camera looks down -Z)
    void clusterAABB( int tx, int ty, int slice,
                      lsVector3& outMin, lsVector3& outMax ) const;

private:
    int     m_tilesX = 0, m_tilesY = 0, m_slices = 0;
    float   m_tilePixels = 16.0f;
    float   m_nearZ = 0.1f, m_farZ = 100.0f;
    float   m_tanX = 1.0f, m_tanY = 1.0f;
    int     m_maxIndices = 65536;   ///< index list capacity

    std::vector<uint32_t>   m_grid;     ///< (offset,count) x clusterCount
    std::vector<uint32_t>   m_indices;  ///< compact light index list
};
