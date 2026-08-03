// Tests for lsRandom (Plan.md Phase 4): range bounds + default-seed determinism.
#include "doctest/doctest.h"
#include "math/lsRandom.h"
using doctest::Approx;

TEST_CASE( "lsRandom: output ranges" )
{
    lsRandom rng;
    for( int i = 0; i < 200; ++i )
    {
        int r = rng.Range( 10 );
        CHECK( r >= 0 );
        CHECK( r < 10 );

        float f = rng.Rand_Float();
        CHECK( f >= 0.0f );
        CHECK( f <= 1.0f );

        float g = rng.Rand_FloatRange( 2.0f, 5.0f );
        CHECK( g >= 2.0f );
        CHECK( g <= 5.0f );
    }
}

TEST_CASE( "lsRandom: default-constructed instances are non-deterministically seeded" )
{
    // FOUND: there is no seed API on lsRandom (header exposes only
    // Range/Rand_Float/Rand_FloatRange); two default instances produce
    // DIFFERENT sequences — seeded from a non-deterministic source.
    // Plan.md §5 asked for pinned seeds, but no such API exists.
    lsRandom a, b;
    bool identical = true;
    for( int i = 0; i < 10; ++i )
        if( a.Rand_Float() != b.Rand_Float() ) identical = false;
    CHECK_FALSE( identical );
}
