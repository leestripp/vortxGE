#include "lsClusterGrid.h"

#include <algorithm>
#include <cmath>
#include <functional>

void lsClusterGrid::setup( int viewportW, int viewportH, float tilePixels,
                           float nearZ, float farZ, int slices,
                           float tanHalfFovX, float tanHalfFovY )
{
    m_tilePixels = tilePixels;
    m_tilesX = std::max( 1, (int)std::ceil( viewportW / tilePixels ) );
    m_tilesY = std::max( 1, (int)std::ceil( viewportH / tilePixels ) );
    m_slices = std::max( 1, slices );
    m_nearZ = std::max( nearZ, 0.001f );
    m_farZ = std::max( farZ, m_nearZ + 0.01f );
    m_tanX = tanHalfFovX;
    m_tanY = tanHalfFovY;

    m_grid.assign( (size_t)clusterCount() * 2, 0u );
    m_indices.clear();
    m_indices.reserve( 4096 );
}

int lsClusterGrid::sliceFromViewDepth( float viewDepth ) const
{
    if( viewDepth <= m_nearZ ) return 0;
    if( viewDepth >= m_farZ ) return m_slices - 1;
    const float t = std::log( viewDepth / m_nearZ ) /
                    std::log( m_farZ / m_nearZ );
    int s = (int)( t * (float)m_slices );
    return std::min( s, m_slices - 1 );
}

void lsClusterGrid::clusterAABB( int tx, int ty, int slice,
                                 lsVector3& outMin, lsVector3& outMax ) const
{
    // slice depth range (positive distances)
    const float zNear = m_nearZ * std::pow( m_farZ / m_nearZ,
                        (float)slice / (float)m_slices );
    const float zFar  = m_nearZ * std::pow( m_farZ / m_nearZ,
                        (float)( slice + 1 ) / (float)m_slices );

    // NDC x/y range of the tile -> view-space x/y at the two z planes.
    // View space: camera at origin looking down -Z, +X right, +Y up.
    const float ndcX0 = 2.0f * (float)tx / (float)m_tilesX - 1.0f;
    const float ndcX1 = 2.0f * (float)( tx + 1 ) / (float)m_tilesX - 1.0f;
    const float ndcY0 = 2.0f * (float)ty / (float)m_tilesY - 1.0f;
    const float ndcY1 = 2.0f * (float)( ty + 1 ) / (float)m_tilesY - 1.0f;

    // x/y grow linearly with depth; evaluate at the far plane (wider)
    const float x0 = ndcX0 * zFar * m_tanX;
    const float x1 = ndcX1 * zFar * m_tanX;
    const float y0 = ndcY0 * zFar * m_tanY;
    const float y1 = ndcY1 * zFar * m_tanY;

    outMin = lsVector3( x0, y0, -zFar );
    outMax = lsVector3( x1, y1, -zNear );
}

// sphere vs AABB (squared distance from sphere centre to box)
static float sphereAABB2( const lsVector3& c, const lsVector3& bmin,
                          const lsVector3& bmax )
{
    float d2 = 0.0f;
    const float cx = c.x, cy = c.y, cz = c.z;
    if( cx < bmin.x ) { const float d = bmin.x - cx; d2 += d * d; }
    if( cx > bmax.x ) { const float d = cx - bmax.x; d2 += d * d; }
    if( cy < bmin.y ) { const float d = bmin.y - cy; d2 += d * d; }
    if( cy > bmax.y ) { const float d = cy - bmax.y; d2 += d * d; }
    if( cz < bmin.z ) { const float d = bmin.z - cz; d2 += d * d; }
    if( cz > bmax.z ) { const float d = cz - bmax.z; d2 += d * d; }
    return d2;
}

// conservative spot-cone vs sphere test (cluster approximated by its
// bounding sphere): true if the cone MIGHT overlap the sphere
static bool coneHitsSphere( const lsVector3& apex, const lsVector3& dir,
                            float cosOuter, const lsVector3& centre,
                            float radius )
{
    const lsVector3 toC = centre - apex;
    const float len2 = toC.Dot( toC );
    if( len2 < radius * radius ) return true; // apex inside sphere

    const float len = std::sqrt( len2 );
    // angle between cone axis and vector to sphere centre
    const float cosA = toC.Dot( dir ) / len;
    if( cosA >= cosOuter ) return true;      // centre inside cone

    // sphere may still intersect the cone's lateral surface; approximate
    // with angular tolerance (conservative: inflate the cone by the
    // sphere's angular size seen from the apex)
    const float sinA = std::sqrt( std::max( 0.0f, 1.0f - cosA * cosA ) );
    const float sinOuter = std::sqrt( std::max( 0.0f, 1.0f - cosOuter * cosOuter ) );
    // angle between axis-to-centre and cone surface
    const float angDiff = std::atan2( sinA, cosA ) - std::atan2( sinOuter, cosOuter );
    const float sphereAng = std::asin( std::min( 1.0f, radius / len ) );
    return angDiff <= sphereAng;
}

void lsClusterGrid::build( const std::vector<lsClusterLightInput>& lights )
{
    std::fill( m_grid.begin(), m_grid.end(), 0u );
    m_indices.clear();

    // per-cluster append counts (offset fixed after prefix pass)
    std::vector<uint32_t> counts( (size_t)clusterCount(), 0u );

    // pass 1: count assignments per cluster
    // pass 2 (after prefix): fill
    // To avoid storing (light,cluster) pairs we do both passes over the
    // lights; each pass is cheap (range-limited loops).

    auto forEachOverlap = [&]( const lsClusterLightInput& L,
                               const std::function<void(int)>& fn )
    {
        const float r = L.radius;
        const lsVector3& p = L.position;      // view space
        const float zc = -p.z;                 // positive depth

        // slice range
        const int s0 = sliceFromViewDepth( std::max( zc - r, m_nearZ ) );
        const int s1 = sliceFromViewDepth( zc + r );

        // screen tile range from the projected bounding sphere
        // (view space -> ndc via tangents; conservative at z->0)
        const float zMin = std::max( zc - r, m_nearZ );
        const float halfW = zMin * m_tanX, halfH = zMin * m_tanY;
        const float ndcX0 = ( p.x - r ) / halfW, ndcX1 = ( p.x + r ) / halfW;
        const float ndcY0 = ( p.y - r ) / halfH, ndcY1 = ( p.y + r ) / halfH;

        int tx0 = std::max( 0, (int)std::floor( ( ndcX0 + 1.0f ) * 0.5f * m_tilesX ) );
        int tx1 = std::min( m_tilesX - 1, (int)std::floor( ( ndcX1 + 1.0f ) * 0.5f * m_tilesX ) );
        int ty0 = std::max( 0, (int)std::floor( ( ndcY0 + 1.0f ) * 0.5f * m_tilesY ) );
        int ty1 = std::min( m_tilesY - 1, (int)std::floor( ( ndcY1 + 1.0f ) * 0.5f * m_tilesY ) );

        // light behind the camera still covers the near slices
        if( p.z > r ) return; // fully behind near plane AND beyond radius

        const float r2 = r * r;
        for( int s = s0; s <= s1; ++s )
            for( int ty = ty0; ty <= ty1; ++ty )
                for( int tx = tx0; tx <= tx1; ++tx )
                {
                    lsVector3 bmin, bmax;
                    clusterAABB( tx, ty, s, bmin, bmax );
                    if( sphereAABB2( p, bmin, bmax ) > r2 ) continue;

                    if( L.type == 2 ) // spot: cone cull vs cluster sphere
                    {
                        const lsVector3 c = ( bmin + bmax ) * 0.5f;
                        const float cr = ( bmax - bmin ).length() * 0.5f;
                        if( !coneHitsSphere( p, L.direction, L.cosOuter,
                                             c, cr ) ) continue;
                    }
                    fn( clusterIndex( tx, ty, s ) );
                }
    };

    for( const auto& L : lights )
    {
        if( L.type == 3 ) continue; // direct lights are global
        forEachOverlap( L, [&]( int ci ) { counts[ci]++; } );
    }

    // prefix sums -> offsets
    uint32_t total = 0;
    for( int c = 0; c < clusterCount(); ++c )
    {
        m_grid[(size_t)c * 2] = total;
        m_grid[(size_t)c * 2 + 1] = counts[c];
        total += counts[c];
    }
    if( total > (uint32_t)m_maxIndices )
    {
        // capacity exceeded: recount with clamping
        total = 0;
        for( int c = 0; c < clusterCount(); ++c )
        {
            const uint32_t room = (uint32_t)m_maxIndices - total;
            const uint32_t n = std::min( counts[c], room );
            m_grid[(size_t)c * 2] = total;
            m_grid[(size_t)c * 2 + 1] = n;
            total += n;
        }
    }
    m_indices.resize( total );

    // pass 2: fill (cursor per cluster)
    std::vector<uint32_t> cursor( (size_t)clusterCount(), 0u );
    for( const auto& L : lights )
    {
        if( L.type == 3 ) continue;
        forEachOverlap( L, [&]( int ci )
        {
            uint32_t& cur = cursor[ci];
            if( cur < m_grid[(size_t)ci * 2 + 1] )
                m_indices[m_grid[(size_t)ci * 2] + cur++] = (uint32_t)L.lightIndex;
        } );
    }
}
