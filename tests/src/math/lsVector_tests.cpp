// Tests for vortxGE/src/math/lsVector2/3/4.{h,cpp}  (Plan.md Phase 1)
//
// Semantics verified against the .cpp implementations:
//   - Cross: standard right-handed formula
//   - Rotations: radians, counter-clockwise (RotateAboutZ: nx=cx-sy, ny=sx+cy)
//   - RotateAboutAxis: Rodrigues' formula, radians
//   - lsVector3::operator*(vec) is intentionally the CROSS product
//     (documented in lsVector3.cpp) — same result as Cross()
//   - lsVector4 is HOMOGENEOUS: all ops act on xyz and preserve w;
//     length()/distance() are 3D; normalize() has a zero guard
//   - FIXED 2026-08-03: lsVector3::normalize() now has a zero-length
//     guard (was div-by-zero → NaN/Inf), matching lsVector4/lsQuaternion

#include "doctest/doctest.h"

#include "math/lsVector2.h"
#include "math/lsVector3.h"
#include "math/lsVector4.h"

#include <cmath>

using doctest::Approx;

static constexpr float PI_HALF = 1.57079632679489661923f;

// ---------------------------------------------------------------------------
TEST_CASE( "lsVector2: construction and arithmetic" )
{
    lsVector2 a;                       // default
    CHECK( a.x == Approx( 0.0f ) );
    CHECK( a.y == Approx( 0.0f ) );

    lsVector2 b( 3.0f, 4.0f );
    lsVector2 c( 1.0f, -2.0f );

    lsVector2 sum = b + c;
    CHECK( sum.x == Approx( 4.0f ) );
    CHECK( sum.y == Approx( 2.0f ) );

    lsVector2 diff = b - c;
    CHECK( diff.x == Approx( 2.0f ) );
    CHECK( diff.y == Approx( 6.0f ) );

    lsVector2 comp = b * c;            // component-wise
    CHECK( comp.x == Approx( 3.0f ) );
    CHECK( comp.y == Approx( -8.0f ) );

    lsVector2 scaled = b * 2.0f;
    CHECK( scaled.x == Approx( 6.0f ) );
    CHECK( scaled.y == Approx( 8.0f ) );

    lsVector2 div = b / 2.0f;
    CHECK( div.x == Approx( 1.5f ) );
    CHECK( div.y == Approx( 2.0f ) );
}

// ---------------------------------------------------------------------------
TEST_CASE( "lsVector3: construction, accessors, scalar/vector arithmetic" )
{
    lsVector3 a;
    CHECK( a.x == Approx( 0.0f ) );
    CHECK( a.y == Approx( 0.0f ) );
    CHECK( a.z == Approx( 0.0f ) );

    lsVector3 b( 1.0f, 2.0f, 3.0f );
    CHECK( b.getX() == Approx( 1.0f ) );
    CHECK( b.getY() == Approx( 2.0f ) );
    CHECK( b.getZ() == Approx( 3.0f ) );

    b.setX( 5.0f );  b.setY( 6.0f );  b.setZ( 7.0f );
    CHECK( b.x == Approx( 5.0f ) );
    CHECK( b.y == Approx( 6.0f ) );
    CHECK( b.z == Approx( 7.0f ) );

    lsVector3 c( 1.0f, 2.0f, 3.0f );

    SUBCASE( "operator+ scalar and vector" )
    {
        lsVector3 s = c + 1.0f;
        CHECK( s.x == Approx( 2.0f ) );
        CHECK( s.y == Approx( 3.0f ) );
        CHECK( s.z == Approx( 4.0f ) );

        lsVector3 v = c + lsVector3( 10.0f, 20.0f, 30.0f );
        CHECK( v.x == Approx( 11.0f ) );
        CHECK( v.y == Approx( 22.0f ) );
        CHECK( v.z == Approx( 33.0f ) );
    }

    SUBCASE( "operator- scalar, vector, unary" )
    {
        lsVector3 s = c - 1.0f;
        CHECK( s.x == Approx( 0.0f ) );
        CHECK( s.y == Approx( 1.0f ) );
        CHECK( s.z == Approx( 2.0f ) );

        lsVector3 v = c - lsVector3( 1.0f, 1.0f, 1.0f );
        CHECK( v.x == Approx( 0.0f ) );
        CHECK( v.y == Approx( 1.0f ) );
        CHECK( v.z == Approx( 2.0f ) );

        lsVector3 n = -c;
        CHECK( n.x == Approx( -1.0f ) );
        CHECK( n.y == Approx( -2.0f ) );
        CHECK( n.z == Approx( -3.0f ) );
    }

    SUBCASE( "operator* scalar, operator/(scalar), operator*(vec) = CROSS" )
    {
        lsVector3 s = c * 2.0f;
        CHECK( s.x == Approx( 2.0f ) );
        CHECK( s.y == Approx( 4.0f ) );
        CHECK( s.z == Approx( 6.0f ) );

        lsVector3 d = c / 2.0f;
        CHECK( d.x == Approx( 0.5f ) );
        CHECK( d.y == Approx( 1.0f ) );
        CHECK( d.z == Approx( 1.5f ) );

        // lsVector3::operator*(const lsVector3&) is intentionally the
        // cross product (documented in lsVector3.cpp) — same as Cross().
        lsVector3 via_op = c * lsVector3( 2.0f, 3.0f, 4.0f );
        lsVector3 via_fn = c.Cross( lsVector3( 2.0f, 3.0f, 4.0f ) );
        CHECK( via_op.x == Approx( via_fn.x ) );
        CHECK( via_op.y == Approx( via_fn.y ) );
        CHECK( via_op.z == Approx( via_fn.z ) );
        CHECK( via_op.x == Approx( -1.0f ) );   // hand-computed cross
        CHECK( via_op.y == Approx( 2.0f ) );
        CHECK( via_op.z == Approx( -1.0f ) );
    }

    SUBCASE( "operator+= and operator-=" )
    {
        lsVector3 m( 1.0f, 1.0f, 1.0f );
        m += lsVector3( 2.0f, 3.0f, 4.0f );
        CHECK( m.x == Approx( 3.0f ) );
        CHECK( m.y == Approx( 4.0f ) );
        CHECK( m.z == Approx( 5.0f ) );

        m -= lsVector3( 1.0f, 1.0f, 1.0f );
        CHECK( m.x == Approx( 2.0f ) );
        CHECK( m.y == Approx( 3.0f ) );
        CHECK( m.z == Approx( 4.0f ) );
    }
}

TEST_CASE( "lsVector3: dot and cross products" )
{
    lsVector3 x( 1.0f, 0.0f, 0.0f );
    lsVector3 y( 0.0f, 1.0f, 0.0f );
    lsVector3 z( 0.0f, 0.0f, 1.0f );

    SUBCASE( "dot: basis vectors" )
    {
        CHECK( x.Dot( x ) == Approx( 1.0f ) );
        CHECK( x.Dot( y ) == Approx( 0.0f ) );   // perpendicular
        CHECK( y.Dot( z ) == Approx( 0.0f ) );
    }

    SUBCASE( "dot: general value (1,2,3)·(4,5,6) = 32" )
    {
        lsVector3 a( 1.0f, 2.0f, 3.0f ), b( 4.0f, 5.0f, 6.0f );
        CHECK( a.Dot( b ) == Approx( 32.0f ) );
        CHECK( b.Dot( a ) == Approx( 32.0f ) );  // commutative
    }

    SUBCASE( "cross: right-handed basis x̂×ŷ=ẑ" )
    {
        lsVector3 r = x.Cross( y );
        CHECK( r.x == Approx( 0.0f ) );
        CHECK( r.y == Approx( 0.0f ) );
        CHECK( r.z == Approx( 1.0f ) );

        // anti-commutative
        lsVector3 r2 = y.Cross( x );
        CHECK( r2.z == Approx( -1.0f ) );
    }

    SUBCASE( "cross: parallel vectors give zero" )
    {
        lsVector3 a( 1.0f, 2.0f, 3.0f );
        lsVector3 b( 2.0f, 4.0f, 6.0f );         // b = 2a
        lsVector3 r = a.Cross( b );
        CHECK( r.x == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( r.y == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( r.z == Approx( 0.0f ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsVector3: length, normalize, distance" )
{
    SUBCASE( "length: 3-4-5 triangle and 3D diagonal" )
    {
        lsVector3 v( 3.0f, 4.0f, 0.0f );
        CHECK( v.length() == Approx( 5.0f ) );

        lsVector3 d( 1.0f, 2.0f, 2.0f );         // sqrt(1+4+4)=3
        CHECK( d.length() == Approx( 3.0f ) );
    }

    SUBCASE( "normalize gives unit length, preserves direction" )
    {
        lsVector3 v( 0.0f, 3.0f, 4.0f );
        v.normalize();
        CHECK( v.length() == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( v.x == Approx( 0.0f ) );
        CHECK( v.y == Approx( 0.6f ).epsilon( 1e-5 ) );
        CHECK( v.z == Approx( 0.8f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "distance" )
    {
        lsVector3 a( 0.0f, 0.0f, 0.0f ), b( 3.0f, 4.0f, 0.0f );
        CHECK( a.distance( b ) == Approx( 5.0f ) );
    }

    SUBCASE( "normalize of zero vector is guarded (no NaN) — regression" )
    {
        lsVector3 z( 0.0f, 0.0f, 0.0f );
        z.normalize();
        CHECK( z.x == Approx( 0.0f ) );
        CHECK( z.y == Approx( 0.0f ) );
        CHECK( z.z == Approx( 0.0f ) );
    }
}

TEST_CASE( "lsVector3: rotations are radians, counter-clockwise" )
{
    SUBCASE( "RotateAboutZ: (1,0,0) by π/2 → (0,1,0)" )
    {
        lsVector3 v( 1.0f, 0.0f, 0.0f );
        v.RotateAboutZ( PI_HALF );
        CHECK( v.x == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( v.y == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( v.z == Approx( 0.0f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "RotateAboutX: (0,1,0) by π/2 → (0,0,1)" )
    {
        lsVector3 v( 0.0f, 1.0f, 0.0f );
        v.RotateAboutX( PI_HALF );
        CHECK( v.x == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( v.y == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( v.z == Approx( 1.0f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "RotateAboutY: (0,0,1) by π/2 → (1,0,0)" )
    {
        lsVector3 v( 0.0f, 0.0f, 1.0f );
        v.RotateAboutY( PI_HALF );
        CHECK( v.x == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( v.y == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( v.z == Approx( 0.0f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "RotateAboutAxis about ẑ matches RotateAboutZ (Rodrigues)" )
    {
        lsVector3 axis( 0.0f, 0.0f, 1.0f );
        lsVector3 v1( 1.0f, 2.0f, 3.0f ), v2( 1.0f, 2.0f, 3.0f );
        v1.RotateAboutZ( 0.7f );
        v2.RotateAboutAxis( 0.7f, axis );
        CHECK( v1.x == Approx( v2.x ).epsilon( 1e-4 ) );
        CHECK( v1.y == Approx( v2.y ).epsilon( 1e-4 ) );
        CHECK( v1.z == Approx( v2.z ).epsilon( 1e-4 ) );
    }

    SUBCASE( "full turn returns to start" )
    {
        lsVector3 v( 1.0f, 2.0f, 3.0f );
        v.RotateAboutZ( 6.28318530717958647692f );
        CHECK( v.x == Approx( 1.0f ).epsilon( 1e-4 ) );
        CHECK( v.y == Approx( 2.0f ).epsilon( 1e-4 ) );
        CHECK( v.z == Approx( 3.0f ).epsilon( 1e-4 ) );
    }
}

// ---------------------------------------------------------------------------
TEST_CASE( "lsVector4: homogeneous semantics — xyz ops preserve w" )
{
    // lsVector4 is a HOMOGENEOUS 3D vector (position/direction + w), not
    // a general 4D vector: every arithmetic operator acts on xyz only and
    // carries w through unchanged; length()/distance() are 3D (xyz);
    // normalize() normalizes xyz (and unlike lsVector3, HAS a zero guard).

    lsVector4 a;
    CHECK( a.x == Approx( 0.0f ) );
    CHECK( a.getW() == Approx( 0.0f ) );

    lsVector4 b( 1.0f, 2.0f, 3.0f, 4.0f );
    CHECK( b.getX() == Approx( 1.0f ) );
    CHECK( b.getW() == Approx( 4.0f ) );

    SUBCASE( "arithmetic acts on xyz, preserves w" )
    {
        lsVector4 s = b + 1.0f;
        CHECK( s.x == Approx( 2.0f ) );
        CHECK( s.y == Approx( 3.0f ) );
        CHECK( s.z == Approx( 4.0f ) );
        CHECK( s.getW() == Approx( 4.0f ) );   // w untouched

        lsVector4 v = b + lsVector4( 1.0f, 1.0f, 1.0f, 99.0f );
        CHECK( v.x == Approx( 2.0f ) );
        CHECK( v.z == Approx( 4.0f ) );
        CHECK( v.getW() == Approx( 4.0f ) );   // lhs w wins, rhs w ignored

        lsVector4 d = b - lsVector4( 1.0f, 2.0f, 3.0f, 4.0f );
        CHECK( d.x == Approx( 0.0f ) );
        CHECK( d.z == Approx( 0.0f ) );
        CHECK( d.getW() == Approx( 4.0f ) );

        lsVector4 m = b * 2.0f;
        CHECK( m.x == Approx( 2.0f ) );
        CHECK( m.z == Approx( 6.0f ) );
        CHECK( m.getW() == Approx( 4.0f ) );

        lsVector4 q = b / 2.0f;
        CHECK( q.x == Approx( 0.5f ) );
        CHECK( q.z == Approx( 1.5f ) );
        CHECK( q.getW() == Approx( 4.0f ) );

        lsVector4 n = -b;
        CHECK( n.x == Approx( -1.0f ) );
        CHECK( n.z == Approx( -3.0f ) );
        CHECK( n.getW() == Approx( 4.0f ) );
    }

    SUBCASE( "length is 3D (xyz only), normalize acts on xyz, w preserved" )
    {
        lsVector4 v( 0.0f, 0.0f, 3.0f, 4.0f );
        CHECK( v.length() == Approx( 3.0f ) );   // NOT sqrt(9+16)
        v.normalize();
        CHECK( v.length() == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( v.z == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( v.getW() == Approx( 4.0f ) );     // w untouched
    }

    SUBCASE( "normalize of zero vector is guarded (no NaN)" )
    {
        lsVector4 z( 0.0f, 0.0f, 0.0f, 7.0f );
        z.normalize();                            // if(len) guard in impl
        CHECK( z.x == Approx( 0.0f ) );
        CHECK( z.getW() == Approx( 7.0f ) );
    }

    SUBCASE( "distance is 3D (xyz only)" )
    {
        lsVector4 p( 0.0f, 0.0f, 0.0f, 0.0f ), q( 0.0f, 0.0f, 3.0f, 4.0f );
        CHECK( p.distance( q ) == Approx( 3.0f ) );
    }
}
