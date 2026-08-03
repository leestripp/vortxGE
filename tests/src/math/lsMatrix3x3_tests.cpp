// Tests for lsMatrix3x3 (Plan.md Phase 4). Storage: column-major matrix[9].
#include "doctest/doctest.h"
#include "math/lsMatrix3x3.h"
#include "math/lsMatrix4x4.h"
using doctest::Approx;

TEST_CASE( "lsMatrix3x3: identity, transpose, product" )
{
    lsMatrix3x3 m;
    CHECK( m.isIdentity() );

    SUBCASE( "transpose round-trip" )
    {
        lsMatrix3x3 a;
        for( int i = 0; i < 9; ++i ) a.matrix[i] = (float)i;
        lsMatrix3x3 t1, t2;
        t1.Transpose( a );
        t2.Transpose( t1 );
        CHECK( t2.isEqual( a ) );
        // spot: t1[c*3+r] == a[r*3+c]
        for( int c = 0; c < 3; ++c )
            for( int r = 0; r < 3; ++r )
                CHECK( t1.matrix[c*3+r] == Approx( a.matrix[r*3+c] ) );
    }

    SUBCASE( "product with identity" )
    {
        lsMatrix3x3 a, ident;
        for( int i = 0; i < 9; ++i ) a.matrix[i] = (float)( i + 1 );
        lsMatrix3x3 p = a * ident;
        CHECK( p.isEqual( a ) );
    }
}

TEST_CASE( "lsMatrix3x3: Inverse of rotation is its transpose" )
{
    // 90° about Z, column-major: col0=(0,1,0) col1=(-1,0,0) col2=(0,0,1)
    lsMatrix3x3 r;
    r.matrix[0]=0; r.matrix[1]=1; r.matrix[2]=0;
    r.matrix[3]=-1; r.matrix[4]=0; r.matrix[5]=0;
    r.matrix[6]=0; r.matrix[7]=0; r.matrix[8]=1;

    lsMatrix3x3 inv;
    inv.Inverse( r );
    lsMatrix3x3 prod = r * inv;
    lsMatrix3x3 ident;
    CHECK( prod.isEqual( ident ) );
}

TEST_CASE( "lsMatrix3x3: ExtractRotationMatrix4x4 keeps 3x3, drops translation" )
{
    lsMatrix4x4 m4;
    m4.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );
    m4.TranslateMatrix( 5.0f, 6.0f, 7.0f );

    lsMatrix3x3 r3;
    r3.ExtractRotationMatrix4x4( m4 );
    // 4x4 rotation col c row r at [c*4+r] → 3x3 at [c*3+r]
    for( int c = 0; c < 3; ++c )
        for( int r = 0; r < 3; ++r )
            CHECK( r3.matrix[c*3+r] == Approx( m4.matrix[c*4+r] ).epsilon( 1e-5 ) );
}
