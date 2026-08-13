// Tests for lsClusterGrid (Plan.md L5.1): exponential slice mapping,
// cluster AABBs, light-to-cluster assignment, capacity clamping.
#include "doctest/doctest.h"
#include "math/lsClusterGrid.h"

using doctest::Approx;

static lsClusterGrid makeGrid()
{
    lsClusterGrid g;
    // 160x90 viewport, 16px tiles -> 10x6 tiles (6 y tiles: 90/16=5.625)
    // near 0.1, far 100, 4 slices, 60deg vfov, 16:9-ish aspect
    const float tanY = 0.57735f;           // tan(30 deg)
    const float tanX = tanY * (160.0f / 90.0f);
    g.setup( 160, 90, 16.0f, 0.1f, 100.0f, 4, tanX, tanY );
    return g;
}

TEST_CASE( "lsClusterGrid: setup computes tile counts" )
{
    auto g = makeGrid();
    CHECK( g.tilesX() == 10 );
    CHECK( g.tilesY() == 6 );
    CHECK( g.slices() == 4 );
    CHECK( g.clusterCount() == 240 );
    CHECK( g.grid().size() == 480 );
}

TEST_CASE( "lsClusterGrid: exponential slice mapping" )
{
    auto g = makeGrid();
    CHECK( g.sliceFromViewDepth( 0.05f ) == 0 );    // before near
    CHECK( g.sliceFromViewDepth( 0.1f ) == 0 );     // at near
    CHECK( g.sliceFromViewDepth( 100.0f ) == 3 );   // at far
    CHECK( g.sliceFromViewDepth( 500.0f ) == 3 );   // beyond far
    // monotonic + exponential: mid ratios land in order
    const int a = g.sliceFromViewDepth( 0.5f );
    const int b = g.sliceFromViewDepth( 5.0f );
    const int c = g.sliceFromViewDepth( 50.0f );
    CHECK( a < b );
    CHECK( b < c );
}

TEST_CASE( "lsClusterGrid: AABB of corner cluster spans its slice" )
{
    auto g = makeGrid();
    lsVector3 mn, mx;
    g.clusterAABB( 0, 0, 0, mn, mx );
    CHECK( mn.x < 0.0f );   // left tile -> negative x
    CHECK( mn.y < 0.0f );   // bottom tile -> negative y
    CHECK( mx.z == Approx( -0.1f ).epsilon( 0.01 ) );  // near plane
    CHECK( mn.z < mx.z );   // far more negative than near
}

TEST_CASE( "lsClusterGrid: point light hits expected clusters only" )
{
    auto g = makeGrid();
    lsClusterLightInput L;
    L.type = 1;                       // point
    L.position = lsVector3( 0, 0, -1.0f );  // centre of view, 1m ahead
    L.radius = 0.5f;
    L.lightIndex = 7;
    g.build( { L } );

    // centre tile (5,3) in the slice containing z=1.0 must contain it
    const int s = g.sliceFromViewDepth( 1.0f );
    const int ci = g.clusterIndex( 5, 3, s );
    const uint32_t off = g.grid()[ci * 2], cnt = g.grid()[ci * 2 + 1];
    REQUIRE( cnt >= 1 );
    bool found = false;
    for( uint32_t k = 0; k < cnt; ++k )
        if( g.indices()[off + k] == 7 ) found = true;
    CHECK( found );

    // far corner tile (0,0) in the LAST slice must NOT contain it
    const int cj = g.clusterIndex( 0, 0, 3 );
    const uint32_t offj = g.grid()[cj * 2], cntj = g.grid()[cj * 2 + 1];
    for( uint32_t k = 0; k < cntj; ++k )
        CHECK( g.indices()[offj + k] != 7 );
}

TEST_CASE( "lsClusterGrid: direct lights are never clustered" )
{
    auto g = makeGrid();
    lsClusterLightInput L;
    L.type = 3;                       // direct
    L.position = lsVector3( 0, 0, -1.0f );
    L.radius = 100.0f;
    L.lightIndex = 0;
    g.build( { L } );
    CHECK( g.indices().empty() );
}

TEST_CASE( "lsClusterGrid: light fully behind camera is skipped" )
{
    auto g = makeGrid();
    lsClusterLightInput L;
    L.type = 1;
    L.position = lsVector3( 0, 0, 50.0f );   // 50m behind
    L.radius = 2.0f;
    g.build( { L } );
    CHECK( g.indices().empty() );
}

TEST_CASE( "lsClusterGrid: spot cone culls clusters behind it" )
{
    auto g = makeGrid();
    lsClusterLightInput L;
    L.type = 2;                       // spot
    L.position = lsVector3( 0, 0, -5.0f );
    L.direction = lsVector3( 0, 0, -1.0f );  // pointing away from camera
    L.cosOuter = 0.7f;                // ~45 deg cone
    L.radius = 3.0f;
    L.lightIndex = 2;
    g.build( { L } );

    // cluster directly BEHIND the spot apex (toward camera, centre tile,
    // near slice) must not contain it: cone points away
    const int ci = g.clusterIndex( 5, 3, 0 );
    const uint32_t off = g.grid()[ci * 2], cnt = g.grid()[ci * 2 + 1];
    for( uint32_t k = 0; k < cnt; ++k )
        CHECK( g.indices()[off + k] != 2 );
}

TEST_CASE( "lsClusterGrid: many lights, index count sane + no overflow" )
{
    auto g = makeGrid();
    std::vector<lsClusterLightInput> lights;
    for( int i = 0; i < 64; ++i )
    {
        lsClusterLightInput L;
        L.type = 1;
        L.position = lsVector3( (float)( i % 8 ) - 3.5f,
                                (float)( i / 8 ) - 3.5f, -10.0f );
        L.radius = 4.0f;
        L.lightIndex = i;
        lights.push_back( L );
    }
    g.build( lights );
    CHECK( !g.indices().empty() );
    CHECK( g.indices().size() < 65536 );
    // grid counts consistent with list
    uint32_t total = 0;
    for( int c = 0; c < g.clusterCount(); ++c )
        total += g.grid()[(size_t)c * 2 + 1];
    CHECK( total == g.indices().size() );
}
