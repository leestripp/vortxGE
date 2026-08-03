// Tests for lsPlane (Plan.md Phase 4): normal·p + d = 0 form, signed distance.
#include "doctest/doctest.h"
#include "math/lsPlane.h"
#include <cmath>
using doctest::Approx;

TEST_CASE( "lsPlane: setCoefficients and signed distance" )
{
    lsPlane p;
    p.setCoefficients( 0.0f, 0.0f, 1.0f, 0.0f );   // z = 0 plane
    CHECK( std::fabs( p.distance( lsVector3( 0.0f, 0.0f, 5.0f ) ) ) == Approx( 5.0f ).epsilon( 1e-5 ) );
    CHECK( p.distance( lsVector3( 1.0f, 2.0f, 0.0f ) ) == Approx( 0.0f ).epsilon( 1e-5 ) );

    SUBCASE( "opposite sides have opposite signs" )
    {
        float above = p.distance( lsVector3( 0.0f, 0.0f, 3.0f ) );
        float below = p.distance( lsVector3( 0.0f, 0.0f, -3.0f ) );
        CHECK( above * below < 0.0f );
    }
}

TEST_CASE( "lsPlane: setNormalAndPoint and set3Points agree for z=0" )
{
    lsPlane pn;
    pn.setNormalAndPoint( lsVector3( 0.0f, 0.0f, 1.0f ), lsVector3( 0.0f, 0.0f, 0.0f ) );
    CHECK( std::fabs( pn.distance( lsVector3( 0.0f, 0.0f, 4.0f ) ) ) == Approx( 4.0f ).epsilon( 1e-4 ) );

    lsVector3 a( 0.0f, 0.0f, 0.0f ), b( 1.0f, 0.0f, 0.0f ), c( 0.0f, 1.0f, 0.0f );
    lsPlane p3;
    p3.set3Points( a, b, c );
    CHECK( std::fabs( p3.distance( lsVector3( 0.0f, 0.0f, 4.0f ) ) ) == Approx( 4.0f ).epsilon( 1e-4 ) );
    CHECK( p3.distance( lsVector3( 7.0f, -3.0f, 0.0f ) ) == Approx( 0.0f ).epsilon( 1e-4 ) );
}
