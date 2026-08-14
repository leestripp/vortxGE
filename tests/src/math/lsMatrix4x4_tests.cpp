#include "doctest/doctest.h"
#include "math/lsMatrix4x4.h"
#include "math/lsVector3.h"
#include "math/lsQuaternion.h"
#include <cmath>

TEST_CASE("lsMatrix4x4: identity leaves point unchanged")
{
    lsMatrix4x4 I;
    lsVector3 v(1.0f, 2.0f, 3.0f);
    lsVector3 r = I * v;
    CHECK(r.x == doctest::Approx(1.0f));
    CHECK(r.y == doctest::Approx(2.0f));
    CHECK(r.z == doctest::Approx(3.0f));
}

TEST_CASE("lsMatrix4x4: column-major translation is in elements [12..14]")
{
    lsMatrix4x4 M;
    M.TranslateMatrix(10.0f, 20.0f, 30.0f);
    CHECK(M.matrix[12] == doctest::Approx(10.0f));
    CHECK(M.matrix[13] == doctest::Approx(20.0f));
    CHECK(M.matrix[14] == doctest::Approx(30.0f));
    CHECK(M.matrix[15] == doctest::Approx(1.0f));
}

TEST_CASE("lsMatrix4x4: LookAt basis is orthonormal and right-handed")
{
    lsMatrix4x4 V;
    lsVector3 eye(5.0f, 5.0f, 5.0f);
    lsVector3 target(0.0f, 0.0f, 0.0f);
    lsVector3 up(0.0f, 1.0f, 0.0f);
    V.LookAt(eye, target, up);

    lsVector3 xaxis(V.matrix[0], V.matrix[4], V.matrix[8]);
    lsVector3 yaxis(V.matrix[1], V.matrix[5], V.matrix[9]);
    lsVector3 zaxis(V.matrix[2], V.matrix[6], V.matrix[10]);

    CHECK(xaxis.length() == doctest::Approx(1.0f).epsilon(1e-4));
    CHECK(yaxis.length() == doctest::Approx(1.0f).epsilon(1e-4));
    CHECK(zaxis.length() == doctest::Approx(1.0f).epsilon(1e-4));
    CHECK(xaxis.Dot(yaxis) == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(xaxis.Dot(zaxis) == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(yaxis.Dot(zaxis) == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(xaxis.Cross(yaxis).Dot(zaxis) == doctest::Approx(1.0f).epsilon(1e-4));
}

TEST_CASE("lsMatrix4x4: LookAt maps eye to origin and world origin to translation column")
{
    lsMatrix4x4 V;
    lsVector3 eye(3.0f, 4.0f, 5.0f);
    lsVector3 target(0.0f, 0.0f, 0.0f);
    lsVector3 up(0.0f, 1.0f, 0.0f);
    V.LookAt(eye, target, up);

    lsVector3 zero = V * eye;
    CHECK(zero.x == doctest::Approx(0.0f).epsilon(1e-3));
    CHECK(zero.y == doctest::Approx(0.0f).epsilon(1e-3));
    CHECK(zero.z == doctest::Approx(0.0f).epsilon(1e-3));

    lsVector3 origin = V * lsVector3(0.0f, 0.0f, 0.0f);
    CHECK(origin.x == doctest::Approx(V.matrix[12]).epsilon(1e-4));
    CHECK(origin.y == doctest::Approx(V.matrix[13]).epsilon(1e-4));
    CHECK(origin.z == doctest::Approx(V.matrix[14]).epsilon(1e-4));
}

TEST_CASE("lsMatrix4x4: orthographic projection maps box center to origin")
{
    lsMatrix4x4 P;
    P.OrthographicProjection(-10.0f, 10.0f, -5.0f, 5.0f, 1.0f, 100.0f);
    auto ndc = P * lsVector3(0.0f, 0.0f, -50.5f);
    CHECK(ndc.x == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(ndc.y == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(ndc.z == doctest::Approx(0.0f).epsilon(1e-4));
}

TEST_CASE("lsMatrix4x4: orthographic projection maps box corners to [-1,1]^3")
{
    lsMatrix4x4 P;
    P.OrthographicProjection(-10.0f, 10.0f, -5.0f, 5.0f, 1.0f, 100.0f);
    // Engine convention: matrix[10] = -2/(far-near), so zMin maps to +1 and zMax to -1.
    auto min = P * lsVector3(-10.0f, -5.0f, -100.0f);
    CHECK(min.x == doctest::Approx(-1.0f).epsilon(1e-4));
    CHECK(min.y == doctest::Approx(-1.0f).epsilon(1e-4));
    CHECK(min.z == doctest::Approx( 1.0f).epsilon(1e-4));
    auto max = P * lsVector3(10.0f, 5.0f, -1.0f);
    CHECK(max.x == doctest::Approx(1.0f).epsilon(1e-4));
    CHECK(max.y == doctest::Approx(1.0f).epsilon(1e-4));
    CHECK(max.z == doctest::Approx(-1.0f).epsilon(1e-4));
}

TEST_CASE("lsMatrix4x4: rotation-only leaves translation zero")
{
    lsMatrix4x4 M;
    M.RotateMatrix(30.0f, 1.0f, 0.0f, 0.0f);
    CHECK(M.matrix[12] == doctest::Approx(0.0f));
    CHECK(M.matrix[13] == doctest::Approx(0.0f));
    CHECK(M.matrix[14] == doctest::Approx(0.0f));
}

TEST_CASE("lsMatrix4x4: Rotate extracts rotation only")
{
    lsMatrix4x4 M;
    M.TranslateMatrix(10.0f, 20.0f, 30.0f);
    M.RotateMatrix(45.0f, 0.0f, 1.0f, 0.0f);
    lsVector3 v(1.0f, 0.0f, 0.0f);
    lsVector3 r = M.Rotate(v);
    CHECK(r.x == doctest::Approx(0.70710678f).epsilon(1e-4));
    CHECK(r.y == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(r.z == doctest::Approx(-0.70710678f).epsilon(1e-4));
}

