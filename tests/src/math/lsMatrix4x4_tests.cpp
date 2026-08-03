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

    SUBCASE( "self-aliasing: m.transpose(m) — regression for engine bug" )
    {
        // transpose() used to corrupt the matrix when source aliased the
        // destination (found in Phase 2 via getLocalTransformFromParent).
        lsMatrix4x4 self_t;
        for( int i = 0; i < 16; ++i )
            self_t.matrix[i] = (float)i;
        self_t.transpose( self_t );              // alias!
        lsMatrix4x4 ref, expect;
        for( int i = 0; i < 16; ++i )
            ref.matrix[i] = (float)i;
        expect.transpose( ref );                 // non-aliased reference
        CHECK( self_t.isEqual( expect ) );
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

// ===========================================================================
// Phase 2 — full lsMatrix4x4 coverage
// Semantics pinned from lsMatrix4x4.cpp:
//   - RotateMatrix / PerspectiveProjection (via Cotangent) take DEGREES
//     (inconsistent with lsVector3::RotateAbout* which take radians —
//     recorded as an API finding)
//   - Translate/Rotate/ScaleMatrix POST-multiply:  this = this * M
//   - operator*(vec3): 3x3 + translation column, NO perspective divide
// ===========================================================================

#include "math/lsQuaternion.h"
#include "math/lsVector3.h"

TEST_CASE( "lsMatrix4x4: TranslateMatrix post-multiplies (Phase 2)" )
{
    lsMatrix4x4 m;                       // identity
    m.TranslateMatrix( 5.0f, -2.0f, 3.0f );

    CHECK( m.matrix[12] == Approx( 5.0f ) );
    CHECK( m.matrix[13] == Approx( -2.0f ) );
    CHECK( m.matrix[14] == Approx( 3.0f ) );
    CHECK( m.matrix[15] == Approx( 1.0f ) );

    SUBCASE( "transform moves a point by the translation" )
    {
        lsVector3 p = m * lsVector3( 1.0f, 1.0f, 1.0f );
        CHECK( p.x == Approx( 6.0f ) );
        CHECK( p.y == Approx( -1.0f ) );
        CHECK( p.z == Approx( 4.0f ) );
    }

    SUBCASE( "two translations accumulate" )
    {
        m.TranslateMatrix( 1.0f, 1.0f, 1.0f );
        CHECK( m.matrix[12] == Approx( 6.0f ) );
        CHECK( m.matrix[13] == Approx( -1.0f ) );
        CHECK( m.matrix[14] == Approx( 4.0f ) );
    }
}

TEST_CASE( "lsMatrix4x4: RotateMatrix takes DEGREES, post-multiplies (Phase 2)" )
{
    lsMatrix4x4 m;
    m.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );   // 90° about +Z

    lsVector3 r = m * lsVector3( 1.0f, 0.0f, 0.0f );
    CHECK( r.x == Approx( 0.0f ).epsilon( 1e-5 ) );
    CHECK( r.y == Approx( 1.0f ).epsilon( 1e-5 ) );
    CHECK( r.z == Approx( 0.0f ).epsilon( 1e-5 ) );

    SUBCASE( "axis is normalised internally" )
    {
        lsMatrix4x4 a, b;
        a.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );
        b.RotateMatrix( 90.0f, 0.0f, 0.0f, 7.0f );   // same direction
        CHECK( a.isEqual( b ) );
    }

    SUBCASE( "composition order: rotate-then-translate vs translate-then-rotate differ" )
    {
        lsMatrix4x4 rt, tr;
        rt.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );
        rt.TranslateMatrix( 1.0f, 0.0f, 0.0f );       // rt = R * T
        tr.TranslateMatrix( 1.0f, 0.0f, 0.0f );
        tr.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );   // tr = T * R
        CHECK_FALSE( rt.isEqual( tr ) );

        // rt * x̂ = R(T(x̂)) = R(x̂ + (1,0,0)) = R(2,0,0) = (0,2,0)
        lsVector3 p = rt * lsVector3( 1.0f, 0.0f, 0.0f );
        CHECK( p.x == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( p.y == Approx( 2.0f ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsMatrix4x4: ScaleMatrix post-multiplies (Phase 2)" )
{
    lsMatrix4x4 m;
    m.ScaleMatrix( 2.0f, 3.0f, 4.0f );

    lsVector3 r = m * lsVector3( 1.0f, 1.0f, 1.0f );
    CHECK( r.x == Approx( 2.0f ) );
    CHECK( r.y == Approx( 3.0f ) );
    CHECK( r.z == Approx( 4.0f ) );
}

TEST_CASE( "lsMatrix4x4: PerspectiveProjection (Phase 2)" )
{
    lsMatrix4x4 m;
    m.PerspectiveProjection( 90.0f, 2.0f, 1.0f, 10.0f );  // fovy=90° aspect=2 n=1 f=10

    SUBCASE( "focal scale: cot(45°)=1 (fovy in DEGREES)" )
    {
        CHECK( m.matrix[5] == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( m.matrix[0] == Approx( 0.5f ).epsilon( 1e-5 ) );  // f/aspect
    }

    SUBCASE( "standard GL form" )
    {
        CHECK( m.matrix[11] == Approx( -1.0f ) );
        CHECK( m.matrix[15] == Approx( 0.0f ) );
    }

    SUBCASE( "near plane maps to NDC z=-1, far to z=+1" )
    {
        // manual project: z' = m[10]*z + m[14], w' = -z
        const float n = 1.0f, f = 10.0f;
        float zp = m.matrix[10] * ( -n ) + m.matrix[14];
        float wp = n;
        CHECK( zp / wp == Approx( -1.0f ).epsilon( 1e-5 ) );

        zp = m.matrix[10] * ( -f ) + m.matrix[14];
        wp = f;
        CHECK( zp / wp == Approx( 1.0f ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsMatrix4x4: OrthographicProjection (Phase 2)" )
{
    lsMatrix4x4 m;
    m.OrthographicProjection( -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 10.0f ); // near/far as distances

    SUBCASE( "x endpoints map to NDC ±1" )
    {
        lsVector3 lo = m * lsVector3( -1.0f, 0.0f, 0.0f );
        lsVector3 hi = m * lsVector3( 1.0f, 0.0f, 0.0f );
        CHECK( lo.x == Approx( -1.0f ) );
        CHECK( hi.x == Approx( 1.0f ) );
    }

    SUBCASE( "near/far distances map to NDC z=-1/+1 (view looks down -Z)" )
    {
        lsVector3 near_p = m * lsVector3( 0.0f, 0.0f, -1.0f );
        lsVector3 far_p  = m * lsVector3( 0.0f, 0.0f, -10.0f );
        CHECK( near_p.z == Approx( -1.0f ).epsilon( 1e-5 ) );
        CHECK( far_p.z  == Approx( 1.0f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "ortho keeps w=1 in last row" )
    {
        CHECK( m.matrix[15] == Approx( 1.0f ) );
        CHECK( m.matrix[11] == Approx( 0.0f ) );
    }
}

TEST_CASE( "lsMatrix4x4: LookAt (Phase 2)" )
{
    lsMatrix4x4 view;
    view.LookAt( lsVector3( 0.0f, 0.0f, 5.0f ),    // eye
                 lsVector3( 0.0f, 0.0f, 0.0f ),    // look at origin
                 lsVector3( 0.0f, 1.0f, 0.0f ) );  // up

    SUBCASE( "eye maps to origin" )
    {
        lsVector3 p = view * lsVector3( 0.0f, 0.0f, 5.0f );
        CHECK( p.x == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( p.y == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( p.z == Approx( 0.0f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "look-at point sits on -Z at distance 5" )
    {
        lsVector3 p = view * lsVector3( 0.0f, 0.0f, 0.0f );
        CHECK( p.x == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( p.y == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( p.z == Approx( -5.0f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "basis is orthonormal (rotation part)" )
    {
        lsMatrix4x4 rot = view.extractRotation();
        lsMatrix4x4 rotT, ident;
        rotT.transpose( rot );
        lsMatrix4x4 prod = rot * rotT;            // R·Rᵀ == I iff orthonormal
        CHECK( prod.isEqual( ident ) );
    }
}

TEST_CASE( "lsMatrix4x4: extractRotation clears translation, keeps 3x3 (Phase 2)" )
{
    lsMatrix4x4 m;
    m.RotateMatrix( 45.0f, 0.0f, 1.0f, 0.0f );
    m.TranslateMatrix( 9.0f, 8.0f, 7.0f );

    lsMatrix4x4 r = m.extractRotation();
    CHECK( r.matrix[12] == Approx( 0.0f ) );
    CHECK( r.matrix[13] == Approx( 0.0f ) );
    CHECK( r.matrix[14] == Approx( 0.0f ) );
    for( int i = 0; i < 12; ++i )                // top 3 columns preserved
        CHECK( r.matrix[i] == Approx( m.matrix[i] ) );
}

TEST_CASE( "lsMatrix4x4: ToEulerZYX recovers pure-Z yaw in DEGREES (Phase 2)" )
{
    // FOUND: ToEulerZYX returns DEGREES (impl converts atan2 output with
    // RAD2ANG) — consistent with RotateMatrix taking degrees.
    lsMatrix4x4 m;
    m.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );   // 90° about Z

    lsVector3 e = lsMatrix4x4::ToEulerZYX( m );
    CHECK( e.x == Approx( 0.0f ).epsilon( 1e-3 ) );    // roll  (degrees)
    CHECK( e.y == Approx( 0.0f ).epsilon( 1e-3 ) );    // pitch (degrees)
    CHECK( e.z == Approx( 90.0f ).epsilon( 1e-3 ) );   // yaw   (degrees)
}

TEST_CASE( "lsMatrix4x4: Set(quaternion) matches quaternion toMatrix4x4 (Phase 2)" )
{
    lsVector3 axis( 0.0f, 0.0f, 1.0f );
    // FOUND: lsQuaternion(axis, angle) takes DEGREES (passing π/2 as
    // radians produced sin(1.57°)≈0.0274 — caught by this test).
    // Phase 3 pins the full quaternion API.
    lsQuaternion q( axis, 90.0f );                // 90° about Z

    lsMatrix4x4 via_set;
    via_set.Set( q, 5.0f, 6.0f, 7.0f );

    lsMatrix4x4 via_quat;
    q.toMatrix4x4( via_quat );

    SUBCASE( "rotation 3x3 identical" )
    {
        for( int i = 0; i < 12; ++i )
            CHECK( via_set.matrix[i] == Approx( via_quat.matrix[i] ).epsilon( 1e-4 ) );
    }

    SUBCASE( "translation column set from args" )
    {
        CHECK( via_set.matrix[12] == Approx( 5.0f ) );
        CHECK( via_set.matrix[13] == Approx( 6.0f ) );
        CHECK( via_set.matrix[14] == Approx( 7.0f ) );
    }

    SUBCASE( "rotates x̂ to ŷ" )
    {
        lsVector3 r = via_quat * lsVector3( 1.0f, 0.0f, 0.0f );
        CHECK( r.x == Approx( 0.0f ).epsilon( 1e-4 ) );
        CHECK( r.y == Approx( 1.0f ).epsilon( 1e-4 ) );
    }
}

TEST_CASE( "lsMatrix4x4: Rotate(vec3) uses 3x3 only, no translation (Phase 2)" )
{
    lsMatrix4x4 m;
    m.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );
    m.TranslateMatrix( 100.0f, 100.0f, 100.0f );

    lsVector3 r = m.Rotate( lsVector3( 1.0f, 0.0f, 0.0f ) );
    CHECK( r.x == Approx( 0.0f ).epsilon( 1e-5 ) );
    CHECK( r.y == Approx( 1.0f ).epsilon( 1e-5 ) );   // rotated, NOT translated
    CHECK( r.z == Approx( 0.0f ).epsilon( 1e-5 ) );
}

TEST_CASE( "lsMatrix4x4: getLocalTransformFromParent (Phase 2)" )
{
    SUBCASE( "pure translation parent" )
    {
        lsMatrix4x4 parent, world;
        parent.TranslateMatrix( 5.0f, 0.0f, 0.0f );
        world.TranslateMatrix( 7.0f, 0.0f, 0.0f );

        lsMatrix4x4 local = world.getLocalTransformFromParent( parent );
        CHECK( local.matrix[12] == Approx( 2.0f ).epsilon( 1e-4 ) );
    }

    SUBCASE( "round-trip: world = parent * local recovers local (rotation+translation)" )
    {
        // Engine convention: rigid transform M = [R|t] is built as T·R
        // (TranslateMatrix THEN RotateMatrix). The impl's parent inverse
        // [Rᵀ | −Rᵀt] is exactly correct for that form (no scale).
        lsMatrix4x4 parent;
        parent.TranslateMatrix( 5.0f, 5.0f, 0.0f );
        parent.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );

        lsMatrix4x4 local_expected;
        local_expected.TranslateMatrix( 1.0f, 0.0f, 0.0f );

        lsMatrix4x4 world = parent * local_expected;
        lsMatrix4x4 local = world.getLocalTransformFromParent( parent );
        CHECK( local.isEqual( local_expected ) );
    }
}

TEST_CASE( "lsMatrix4x4: float[16] assignment and multiply operators (Phase 2)" )
{
    float raw[16];
    for( int i = 0; i < 16; ++i ) raw[i] = (float)i;

    lsMatrix4x4 m;
    m = raw;                                       // operator=(const float[16])
    for( int i = 0; i < 16; ++i )
        CHECK( m.matrix[i] == Approx( (float)i ) );

    SUBCASE( "operator*(float[16]) matches operator*(lsMatrix4x4)" )
    {
        float identity_raw[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
        lsMatrix4x4 via_raw = m * identity_raw;
        CHECK( via_raw.isEqual( m ) );
    }
}
