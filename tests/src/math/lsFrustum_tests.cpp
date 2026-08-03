// Tests for lsFrustum (Plan.md Phase 4) — public API only.
// All state (pl[6], camerapos, nearplane, …) is private; the only
// observable query is distance(V1) = |camerapos - V1|, where camerapos
// is anchored by Build_CullPlanes (verified in lsFrustum.cpp).
// Note: no public point-in-frustum query exists (cull planes are
// private) — recorded as an API observation.
#include "doctest/doctest.h"
#include "math/lsFrustum.h"
using doctest::Approx;

TEST_CASE( "lsFrustum: distance is radial from Build_CullPlanes camera position" )
{
    lsFrustum fr;
    fr.Set_Perspective( 90.0f, 1.0f, 1.0f, 100.0f );

    fr.Build_CullPlanes( lsVector3( 0.0f, 0.0f, 5.0f ),
                         lsVector3( 0.0f, 0.0f, -1.0f ),
                         lsVector3( 0.0f, 1.0f, 0.0f ) );

    CHECK( fr.distance( lsVector3( 0.0f, 0.0f, 5.0f ) ) == Approx( 0.0f ).epsilon( 1e-5 ) );
    CHECK( fr.distance( lsVector3( 0.0f, 0.0f, 0.0f ) ) == Approx( 5.0f ).epsilon( 1e-5 ) );
    CHECK( fr.distance( lsVector3( 3.0f, 4.0f, 5.0f ) ) == Approx( 5.0f ).epsilon( 1e-5 ) );

    SUBCASE( "re-anchoring moves the reference point" )
    {
        fr.Build_CullPlanes( lsVector3( 0.0f, 0.0f, 0.0f ),
                             lsVector3( 0.0f, 0.0f, -1.0f ),
                             lsVector3( 0.0f, 1.0f, 0.0f ) );
        CHECK( fr.distance( lsVector3( 0.0f, 0.0f, 5.0f ) ) == Approx( 5.0f ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsFrustum: orthographic setup path is callable" )
{
    lsFrustum fr;
    fr.Set_Orthographic( -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 10.0f );
    fr.Build_CullPlanes( lsVector3( 0.0f, 0.0f, 0.0f ),
                         lsVector3( 0.0f, 0.0f, -1.0f ),
                         lsVector3( 0.0f, 1.0f, 0.0f ) );
    CHECK( fr.distance( lsVector3( 0.0f, 0.0f, -7.0f ) ) == Approx( 7.0f ).epsilon( 1e-5 ) );
}
