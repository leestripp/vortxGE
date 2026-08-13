// Tests for lsFitSpotDepthRange (Plan.md L3.1): tight near/far fit of
// a spot shadow frustum to scene content in light view space.
#include "doctest/doctest.h"
#include "scene/lsLight_Spot.h"
using doctest::Approx;

TEST_CASE( "lsFitSpotDepthRange: empty input keeps caller defaults" )
{
	lsMatrix4x4 view; // identity: light at origin, looking down -Z
	float n = 1.0f, f = 50.0f;
	CHECK( lsFitSpotDepthRange( view, {}, {}, n, f ) == false );
	CHECK( n == Approx( 1.0 ) );
	CHECK( f == Approx( 50.0 ) );
}

TEST_CASE( "lsFitSpotDepthRange: brackets content depths with radii" )
{
	lsMatrix4x4 view; // identity
	std::vector<lsVector3> pos = { lsVector3( 0, 0, -10 ),
								   lsVector3( 3, 1, -20 ) };
	std::vector<float> rad = { 1.0f, 2.0f };
	float n, f;
	REQUIRE( lsFitSpotDepthRange( view, pos, rad, n, f ) );
	CHECK( n == Approx( 9.0 ) );  // 10 - 1
	CHECK( f == Approx( 22.0 ) ); // 20 + 2
}

TEST_CASE( "lsFitSpotDepthRange: clamps near and keeps far > near" )
{
	lsMatrix4x4 view;
	// object sitting on the light plane / slightly behind it
	std::vector<lsVector3> pos = { lsVector3( 0, 0, 0.5 ) };
	std::vector<float> rad = { 1.0f };
	float n, f;
	REQUIRE( lsFitSpotDepthRange( view, pos, rad, n, f ) );
	CHECK( n == Approx( 0.05 ) );        // clamped positive
	CHECK( f >= Approx( n + 0.1 ) );     // strictly beyond near
}
