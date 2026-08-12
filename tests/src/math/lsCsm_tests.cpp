// Tests for lsCsm (Plan.md L2.1/L2.2): practical split scheme,
// per-cascade ortho fitting, world-anchored texel snapping.
#include "doctest/doctest.h"
#include "math/lsCsm.h"
using doctest::Approx;

TEST_CASE( "lsCsmSplitDepths: endpoints, monotonicity, lambda extremes" )
{
	const float nearZ = 0.5f, farZ = 300.0f;

	auto s = lsCsmSplitDepths( nearZ, farZ, 4, 0.5f );
	CHECK( s.size() == 5 );
	CHECK( s.front() == Approx( nearZ ) );
	CHECK( s.back() == Approx( farZ ) );
	for( size_t i = 1; i < s.size(); ++i )
		CHECK( s[i] > s[i - 1] );

	// lambda=0 -> uniform
	auto su = lsCsmSplitDepths( nearZ, farZ, 4, 0.0f );
	CHECK( su[1] == Approx( nearZ + ( farZ - nearZ ) * 0.25f ) );
	// lambda=1 -> log
	auto sl = lsCsmSplitDepths( nearZ, farZ, 4, 1.0f );
	CHECK( sl[1] == Approx( nearZ * powf( farZ / nearZ, 0.25f ) ) );
	// blended sits between the two
	CHECK( s[1] > sl[1] - 0.001f );
	CHECK( s[1] < su[1] + 0.001f );
}

TEST_CASE( "lsCsmCompute: cascade count, contiguous splits, interior mapping" )
{
	lsCsmConfig cfg;
	cfg.cascades = 4;
	cfg.mapSize = 2048;

	// camera at origin looking down -Z (identity view)
	lsMatrix4x4 camView;
	camView.LoadIdentity();

	auto ccs = lsCsmCompute( cfg, camView, 60.0f, 16.0f / 9.0f,
							 0.5f, 300.0f,
							 lsVector3( 0.0f, -1.0f, 0.0f ), false );
	REQUIRE( ccs.size() == 4 );

	// contiguous split coverage
	CHECK( ccs[0].splitNear == Approx( 0.5f ) );
	for( size_t i = 1; i < ccs.size(); ++i )
		CHECK( ccs[i].splitNear == Approx( ccs[i - 1].splitFar ) );
	CHECK( ccs[3].splitFar == Approx( 300.0f ) );

	// a world point at the centre of cascade 0's slice maps into the
	// interior of its shadow map (texture space [0,1])
	const float midDepth = 0.5f * ( ccs[0].splitNear + ccs[0].splitFar );
	const lsVector3 p( 0.0f, 0.0f, -midDepth );
	const lsVector3 t = ccs[0].shadowMatrix * p;
	CHECK( t.x > 0.05f ); CHECK( t.x < 0.95f );
	CHECK( t.y > 0.05f ); CHECK( t.y < 0.95f );
	CHECK( t.z > 0.0f );  CHECK( t.z < 1.0f );
}

TEST_CASE( "lsCsmCompute: yFlip mirrors the bias Y row (VK convention)" )
{
	lsCsmConfig cfg;
	lsMatrix4x4 camView;
	camView.LoadIdentity();

	auto a = lsCsmCompute( cfg, camView, 60.0f, 1.0f, 0.5f, 100.0f,
						   lsVector3( 0.0f, -1.0f, 0.0f ), false );
	auto b = lsCsmCompute( cfg, camView, 60.0f, 1.0f, 0.5f, 100.0f,
						   lsVector3( 0.0f, -1.0f, 0.0f ), true );
	REQUIRE( a.size() == b.size() );
	// the y-scale row of the shadow matrix flips sign
	CHECK( a[0].shadowMatrix.matrix[5] ==
		   Approx( -b[0].shadowMatrix.matrix[5] ) );
}

TEST_CASE( "lsCsmCompute: texel snapping — sub-texel camera moves are stable" )
{
	lsCsmConfig cfg;
	cfg.cascades = 4;
	cfg.mapSize = 2048;

	lsMatrix4x4 v0;
	v0.LoadIdentity();

	auto base = lsCsmCompute( cfg, v0, 60.0f, 16.0f / 9.0f, 0.5f, 300.0f,
							  lsVector3( 0.0f, -1.0f, 0.0f ), false );
	REQUIRE( base.size() == 4 );

	// translate the camera by a small sub-texel amount (+x 0.01)
	// view translation = -R^T * camPos; identity rotation -> -camPos
	lsMatrix4x4 v1;
	v1.LoadIdentity();
	v1.matrix[12] = -0.01f;
	auto moved = lsCsmCompute( cfg, v1, 60.0f, 16.0f / 9.0f, 0.5f, 300.0f,
							   lsVector3( 0.0f, -1.0f, 0.0f ), false );
	REQUIRE( moved.size() == 4 );

	// cascade 0 (smallest texels, most sensitive) must not change
	for( int j = 0; j < 16; ++j )
		CHECK( moved[0].shadowMatrix.matrix[j] ==
			   Approx( base[0].shadowMatrix.matrix[j] ).epsilon( 1e-4 ) );

	// a clearly super-texel move MUST change the matrix
	lsMatrix4x4 v2;
	v2.LoadIdentity();
	v2.matrix[12] = -50.0f;
	auto far = lsCsmCompute( cfg, v2, 60.0f, 16.0f / 9.0f, 0.5f, 300.0f,
							 lsVector3( 0.0f, -1.0f, 0.0f ), false );
	bool differs = false;
	for( int j = 0; j < 16; ++j )
		if( fabsf( far[0].shadowMatrix.matrix[j] -
				   base[0].shadowMatrix.matrix[j] ) > 1e-3f )
			differs = true;
	CHECK( differs );
}
