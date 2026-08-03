// Tests for vortxGE/src/math/lsMatrix4x4.{h,cpp}
//
// Storage convention (verified against lsMatrix4x4.cpp):
//   column-major — element (row r, col c) lives at matrix[c*4 + r],
//   matching OpenGL. operator* is the standard column-major product.
//
// Plan.md Phase 0: the first two cases are the old tests/main.cpp
// debug prints (transpose, operator*) converted into real assertions.

#include "doctest/doctest.h"

#include "math/lsMatrix4x4.h"

using doctest::Approx;

TEST_CASE( "lsMatrix4x4: default constructor yields identity" )
{
    lsMatrix4x4 m;

    CHECK( m.isIdentity() );

    lsMatrix4x4 ref;
    ref.LoadIdentity();
    CHECK( m.isEqual( ref ) );

    // Spot-check storage: diagonal 1, off-diagonal 0
    for( int i = 0; i < 16; ++i )
    {
        const bool diagonal = ( i == 0 || i == 5 || i == 10 || i == 15 );
        CHECK( m.matrix[i] == Approx( diagonal ? 1.0 : 0.0 ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsMatrix4x4: transpose" )
{
    // Converted from old tests/main.cpp (was a visual debug print).
    lsMatrix4x4 mat1;
    for( int i = 0; i < 16; ++i )
        mat1.matrix[i] = (float)i;

    lsMatrix4x4 mat2;
    mat2.transpose( mat1 );

    // Column-major storage of the transpose — hand-computed:
    // transposed(r,c) = original(c,r)  →  storage[c*4+r] = original[r*4+c]
    const float expected[16] = {
        0, 4, 8, 12,     // column 0 = original row 0
        1, 5, 9, 13,     // column 1 = original row 1
        2, 6, 10, 14,    // column 2 = original row 2
        3, 7, 11, 15     // column 3 = original row 3
    };
    for( int i = 0; i < 16; ++i )
        CHECK( mat2.matrix[i] == Approx( expected[i] ).epsilon( 1e-5 ) );

    SUBCASE( "transpose is an involution: transpose(transpose(m)) == m" )
    {
        lsMatrix4x4 round_trip;
        round_trip.transpose( mat2 );
        CHECK( round_trip.isEqual( mat1 ) );
    }

    SUBCASE( "transpose of identity is identity" )
    {
        lsMatrix4x4 t;
        t.transpose( lsMatrix4x4() );
        CHECK( t.isIdentity() );
    }
}

TEST_CASE( "lsMatrix4x4: operator* with identity" )
{
    lsMatrix4x4 m;
    for( int i = 0; i < 16; ++i )
        m.matrix[i] = (float)( i * 3 - 7 );   // arbitrary non-symmetric values

    lsMatrix4x4 identity;

    SUBCASE( "M * I == M" )
    {
        lsMatrix4x4 r = m * identity;
        CHECK( r.isEqual( m ) );
    }

    SUBCASE( "I * M == M" )
    {
        lsMatrix4x4 r = identity * m;
        CHECK( r.isEqual( m ) );
    }
}

TEST_CASE( "lsMatrix4x4: operator* matches hand-computed product" )
{
    // Converted from old tests/main.cpp (was a visual debug print).
    // A = storage[i] = i  →  A(r,c) = r + 4c   (column-major)
    // B = transpose(A)    →  B(r,c) = c + 4r
    // P = A * B: P(r,c) = Σ_k (r+4k)(c+4k)
    //                   = 4rc + 24(r+c) + 224        (closed form, exact)
    lsMatrix4x4 a;
    for( int i = 0; i < 16; ++i )
        a.matrix[i] = (float)i;

    lsMatrix4x4 b;
    b.transpose( a );

    lsMatrix4x4 p = a * b;

    for( int c = 0; c < 4; ++c )
        for( int r = 0; r < 4; ++r )
        {
            const double expected = 4.0 * r * c + 24.0 * ( r + c ) + 224.0;
            CAPTURE( r );
            CAPTURE( c );
            CHECK( p.matrix[c * 4 + r] == Approx( expected ).epsilon( 1e-5 ) );
        }

    SUBCASE( "matrix product is non-commutative" )
    {
        // A·Aᵀ vs Aᵀ·A closed forms differ:
        //   (A·Aᵀ)(r,c) =  4rc + 24(r+c) + 224
        //   (Aᵀ·A)(r,c) = 64rc + 24(r+c) +  14   → e.g. (0,0): 224 vs 14
        lsMatrix4x4 forward = a * b;
        lsMatrix4x4 backward = b * a;
        CHECK_FALSE( forward.isEqual( backward ) );
    }
}
