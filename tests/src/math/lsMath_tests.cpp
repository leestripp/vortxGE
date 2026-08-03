// Tests for lsMath.h constants (Plan.md Phase 4).
#include "doctest/doctest.h"
#include "math/lsMath.h"
using doctest::Approx;

TEST_CASE( "lsMath: angle conversion constants are mutual inverses" )
{
    CHECK( 90.0 * ANG2RAD == Approx( lsPI / 2.0 ).epsilon( 1e-12 ) );
    CHECK( ( lsPI * RAD2ANG ) == Approx( 180.0 ).epsilon( 1e-10 ) );
    CHECK( ANG2RAD * RAD2ANG == Approx( 1.0 ).epsilon( 1e-10 ) );
    CHECK( lsPI == Approx( 3.14159265358979323846 ).epsilon( 1e-15 ) );
}
