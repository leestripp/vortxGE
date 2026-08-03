// Tests for lsBoundingSphere (Plan.md Phase 4).
#include "doctest/doctest.h"
#include "math/lsBoundingSphere.h"
using doctest::Approx;

TEST_CASE( "lsBoundingSphere: center/radius fields and distance ordering" )
{
    lsBoundingSphere s;
    s.center = lsVector3( 0.0f, 0.0f, 0.0f );
    s.radius = 2.0f;

    float d_center = s.distance( lsVector3( 0.0f, 0.0f, 0.0f ) );
    float d_edge   = s.distance( lsVector3( 2.0f, 0.0f, 0.0f ) );
    float d_far    = s.distance( lsVector3( 10.0f, 0.0f, 0.0f ) );

    CHECK( d_center <= d_edge );
    CHECK( d_edge <= d_far );
    CHECK( d_far > 0.0f );
}
