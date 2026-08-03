// Tests for vortxGE/src/math/lsQuaternion.{h,cpp}  (Plan.md Phase 3)
//
// Semantics pinned from lsQuaternion.cpp:
//   - fromAxisAngle / axis-angle ctor take DEGREES (halfAngle * ANG2RAD)
//   - inverse() == conjugate() — correct only for UNIT quaternions
//   - normalize() has a zero guard; toMatrix4x4() normalises first
//   - lsQuaternion(axis,angle) degrees matches lsMatrix4x4::RotateMatrix
//     degrees — consistent within the matrix/quaternion API, but NOT
//     with lsVector3::RotateAbout* (radians) — API finding

#include "doctest/doctest.h"

#include "math/lsQuaternion.h"
#include "math/lsMatrix4x4.h"
#include "math/lsVector3.h"

using doctest::Approx;

static const float SQRT2_2 = 0.70710678118654752440f;   // √2/2

TEST_CASE( "lsQuaternion: axis-angle construction (DEGREES)" )
{
    lsVector3 zaxis( 0.0f, 0.0f, 1.0f );
    lsQuaternion q( zaxis, 90.0f );

    CHECK( q.w == Approx( SQRT2_2 ).epsilon( 1e-5 ) );   // cos(45°)
    CHECK( q.x == Approx( 0.0f ).epsilon( 1e-5 ) );
    CHECK( q.y == Approx( 0.0f ).epsilon( 1e-5 ) );
    CHECK( q.z == Approx( SQRT2_2 ).epsilon( 1e-5 ) );   // sin(45°)

    SUBCASE( "fromAxisAngle matches ctor; axis normalised internally" )
    {
        lsQuaternion a, b;
        a.fromAxisAngle( lsVector3( 0.0f, 0.0f, 1.0f ), 90.0f );
        b.fromAxisAngle( lsVector3( 0.0f, 0.0f, 9.0f ), 90.0f );
        CHECK( a.w == Approx( b.w ).epsilon( 1e-5 ) );
        CHECK( a.z == Approx( b.z ).epsilon( 1e-5 ) );
    }

    SUBCASE( "zero rotation is identity" )
    {
        lsQuaternion id( zaxis, 0.0f );
        CHECK( id.w == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( id.z == Approx( 0.0f ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsQuaternion: magnitude and normalize" )
{
    SUBCASE( "axis-angle quats are unit" )
    {
        lsQuaternion q( lsVector3( 1.0f, 2.0f, 3.0f ), 73.0f );
        CHECK( q.magnitude() == Approx( 1.0f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "normalize scales to unit; zero-guard present" )
    {
        lsQuaternion q( 2.0f, 0.0f, 0.0f, 0.0f );   // w,x,y,z ctor
        CHECK( q.magnitude() == Approx( 2.0f ) );
        q.normalize();
        CHECK( q.magnitude() == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( q.w == Approx( 1.0f ).epsilon( 1e-5 ) );

        lsQuaternion zero( 0.0f, 0.0f, 0.0f, 0.0f );
        zero.normalize();                            // must not produce NaN
        CHECK( zero.magnitude() == Approx( 0.0f ) );
    }
}

TEST_CASE( "lsQuaternion: conjugate and inverse (unit quats)" )
{
    lsQuaternion q( lsVector3( 0.0f, 1.0f, 0.0f ), 35.0f );

    SUBCASE( "conjugate negates vector part" )
    {
        lsQuaternion c = q.conjugate();
        CHECK( c.w == Approx( q.w ) );
        CHECK( c.x == Approx( -q.x ) );
        CHECK( c.y == Approx( -q.y ) );
        CHECK( c.z == Approx( -q.z ) );
    }

    SUBCASE( "q * q.inverse() == identity" )
    {
        lsQuaternion id = q * q.inverse();
        CHECK( id.w == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( id.x == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( id.y == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( id.z == Approx( 0.0f ).epsilon( 1e-5 ) );
    }

    SUBCASE( "inverse of NON-unit quaternion also gives identity — regression" )
    {
        // inverse() used to be plain conjugate() — wrong when |q| != 1.
        lsQuaternion big = q;
        big.w *= 3.0f;  big.x *= 3.0f;  big.y *= 3.0f;  big.z *= 3.0f;
        lsQuaternion id = big * big.inverse();
        CHECK( id.w == Approx( 1.0f ).epsilon( 1e-5 ) );
        CHECK( id.x == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( id.y == Approx( 0.0f ).epsilon( 1e-5 ) );
        CHECK( id.z == Approx( 0.0f ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsQuaternion: multiplication composes rotations, non-commutative" )
{
    lsVector3 zaxis( 0.0f, 0.0f, 1.0f );
    lsQuaternion z90a( zaxis, 90.0f ), z90b( zaxis, 90.0f );

    SUBCASE( "90°+90° about Z = 180° about Z" )
    {
        lsQuaternion z180 = z90a * z90b;
        CHECK( z180.w == Approx( 0.0f ).epsilon( 1e-5 ) );  // cos(90°)
        CHECK( z180.z == Approx( 1.0f ).epsilon( 1e-5 ) );  // sin(90°)
    }

    SUBCASE( "non-commutative for differing axes" )
    {
        lsQuaternion x90( lsVector3( 1.0f, 0.0f, 0.0f ), 90.0f );
        lsQuaternion y90( lsVector3( 0.0f, 1.0f, 0.0f ), 90.0f );
        lsQuaternion xy = x90 * y90;
        lsQuaternion yx = y90 * x90;
        bool differs = fabs( xy.w - yx.w ) > 1e-4f ||
                       fabs( xy.x - yx.x ) > 1e-4f ||
                       fabs( xy.y - yx.y ) > 1e-4f ||
                       fabs( xy.z - yx.z ) > 1e-4f;
        CHECK( differs );
    }

    SUBCASE( "operator*= matches operator*" )
    {
        lsQuaternion a( lsVector3( 0.0f, 0.0f, 1.0f ), 30.0f );
        lsQuaternion b( lsVector3( 0.0f, 1.0f, 0.0f ), 45.0f );
        lsQuaternion via_star = a * b;
        a *= b;
        CHECK( a.w == Approx( via_star.w ).epsilon( 1e-5 ) );
        CHECK( a.z == Approx( via_star.z ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsQuaternion: Slerp endpoints and halfway (angles in degrees)" )
{
    lsQuaternion from( lsVector3( 0.0f, 0.0f, 1.0f ), 0.0f );
    lsQuaternion to( lsVector3( 0.0f, 0.0f, 1.0f ), 90.0f );

    SUBCASE( "t=0 gives from, t=1 gives to" )
    {
        lsQuaternion s0 = from;  s0.Slerp( to, 0.0f );
        CHECK( s0.w == Approx( from.w ).epsilon( 1e-4 ) );
        CHECK( s0.z == Approx( from.z ).epsilon( 1e-4 ) );

        lsQuaternion s1 = from;  s1.Slerp( to, 1.0f );
        CHECK( s1.w == Approx( to.w ).epsilon( 1e-4 ) );
        CHECK( s1.z == Approx( to.z ).epsilon( 1e-4 ) );
    }

    SUBCASE( "t=0.5 is the 45° rotation about Z" )
    {
        lsQuaternion half = from;  half.Slerp( to, 0.5f );
        CHECK( half.w == Approx( 0.92387953251f ).epsilon( 1e-4 ) );  // cos(22.5°)
        CHECK( half.z == Approx( 0.38268343236f ).epsilon( 1e-4 ) );  // sin(22.5°)
    }

    SUBCASE( "static two-arg form matches" )
    {
        lsQuaternion s;
        s.Slerp( from, to, 0.5f );
        CHECK( s.w == Approx( 0.92387953251f ).epsilon( 1e-4 ) );
    }
}

TEST_CASE( "lsQuaternion: Lerp midpoint and dot" )
{
    lsQuaternion a( 1.0f, 0.0f, 0.0f, 0.0f );
    lsQuaternion b( 0.0f, 1.0f, 0.0f, 0.0f );

    SUBCASE( "lerp t=0.5 gives component midpoint" )
    {
        lsQuaternion l = a;  l.Lerp( b, 0.5f );
        CHECK( l.w == Approx( 0.5f ).epsilon( 1e-4 ) );
        CHECK( l.x == Approx( 0.5f ).epsilon( 1e-4 ) );
    }

    SUBCASE( "dot of identical unit quats is 1" )
    {
        lsQuaternion q( lsVector3( 0.0f, 0.0f, 1.0f ), 50.0f );
        CHECK( q.dot( q ) == Approx( 1.0f ).epsilon( 1e-5 ) );
    }
}

TEST_CASE( "lsQuaternion: toMatrix4x4 orthonormal and matches RotateMatrix" )
{
    lsQuaternion q( lsVector3( 0.0f, 0.0f, 1.0f ), 90.0f );   // 90° Z, degrees

    lsMatrix4x4 mq;
    q.toMatrix4x4( mq );

    SUBCASE( "rotation is orthonormal (R·Rᵀ == I)" )
    {
        lsMatrix4x4 t, ident;
        t.transpose( mq );
        CHECK( ( mq * t ).isEqual( ident ) );
    }

    SUBCASE( "matches lsMatrix4x4::RotateMatrix (both degrees)" )
    {
        lsMatrix4x4 mm;
        mm.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );
        for( int i = 0; i < 12; ++i )
            CHECK( mq.matrix[i] == Approx( mm.matrix[i] ).epsilon( 1e-4 ) );
    }

    SUBCASE( "toMatrix4x4 normalises first (non-unit input OK)" )
    {
        lsQuaternion big( lsVector3( 0.0f, 0.0f, 1.0f ), 90.0f );
        big.w *= 3.0f;  big.x *= 3.0f;  big.y *= 3.0f;  big.z *= 3.0f;
        lsMatrix4x4 mb;
        big.toMatrix4x4( mb );
        for( int i = 0; i < 12; ++i )
            CHECK( mb.matrix[i] == Approx( mq.matrix[i] ).epsilon( 1e-4 ) );
    }
}
