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

/// GPU-side cluster data (texture IDs + grid params) — filled per
/// frame by the pipeline cluster stage, read by lsShader. Plain data,
/// no render-header dependency (lsTextureID is uint32).
struct lsClusterGpuData
{
    uint32_t gridTex = 0;      ///< RG32UI: tilesX x (tilesY*slices), (offset,count)
    uint32_t indexTex = 0;     ///< R32UI: 256 x N light indices
    int      tilesX = 0, tilesY = 0, slices = 0;
    float    nearZ = 0.1f, farZ = 100.0f;
    float    tilePixels = 16.0f;
    int      indexTexWidth = 256;
    bool     enabled = false;  ///< false -> shaders use the legacy loop
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

    /// per-cluster light cap (default 32)
    void setMaxPerCluster( int n ) { m_maxPerCluster = n; }
    /// index list capacity
    void setMaxIndices( int n ) { m_maxIndices = n; }

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
    // E5.1: precomputed slice depth planes (m_slices+1 entries) — the
    // grid build walks every overlapped cluster per light per pass and
    // std::pow per cell was ~30ms on the 65-light stress scene.
    std::vector<float> m_sliceDepths;
    // E5.1b: per-cluster view-space bounding sphere (center xyz +
    // radius), precomputed at setup() — the per-light overlap loop was
    // rebuilding every cluster AABB twice per frame (~3M heavy cells on
    // the stress scene, 30ms). Sphere-vs-sphere is ~8 ops per cell and
    // conservative (the cluster sphere encloses the AABB).
    std::vector<float> m_centers;  // clusterCount() * 3
    std::vector<float> m_radii;    // clusterCount()
    float   m_tanX = 1.0f, m_tanY = 1.0f;
    int     m_maxIndices = 262144;  ///< index list capacity
    int     m_maxPerCluster = 16;   ///< per-cluster light cap

    std::vector<uint32_t>   m_grid;     ///< (offset,count) x clusterCount
    std::vector<uint32_t>   m_indices;  ///< compact light index list
};
