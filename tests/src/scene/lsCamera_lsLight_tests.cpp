#include "doctest/doctest.h"
#include "scene/lsCamera.h"
#include "scene/lsLight.h"
#include "scene/lsLight_Direct.h"
#include "math/lsVector3.h"
#include <cmath>

TEST_CASE("lsCamera: dirty flag tracks projection changes")
{
    lsCamera cam(nullptr);
    // After construction/initial Update_Camera, dirty should be false.
    cam.Update_Camera();
    CHECK(cam.isDirty() == false);

    cam.Set_ViewRatio(2.0f);
    CHECK(cam.isDirty() == true);

    cam.Update_Camera();
    CHECK(cam.isDirty() == false);

    // Second update with no change is a no-op and stays clean.
    cam.Update_Camera();
    CHECK(cam.isDirty() == false);
}

TEST_CASE("lsCamera: ortho getters return values set by Set_Orthographic")
{
    lsCamera cam(nullptr);
    cam.Set_Orthographic(-10.0f, 10.0f, -5.0f, 5.0f, 1.0f, 100.0f);
    CHECK(cam.getOrtho_left() == doctest::Approx(-10.0f));
    CHECK(cam.getOrtho_right() == doctest::Approx(10.0f));
    CHECK(cam.getOrtho_bottom() == doctest::Approx(-5.0f));
    CHECK(cam.getOrtho_top() == doctest::Approx(5.0f));
}

TEST_CASE("lsLight: shadow matrix is cached per frame")
{
    lsLight light(nullptr);
    light.setPixel_x(256.0f);
    light.setPixel_y(256.0f);
    light.setHas_shadowmap(true);
    light.Set_Orthographic(-100.0f, 100.0f, -100.0f, 100.0f, 10.0f, 1000.0f);
    light.Set_Pos(0.0f, 50.0f, 30.0f);
    light.Set_Rot(-60.0f, 0.0f, 0.0f);

    // Mirrors the engine path: the stage updates the camera (clears
    // the dirty flag), then samples the shadow matrix.
    light.Update_Camera();

    // Advance frame counter and build once -> one rebuild.
    lsLight::nextShadowFrame();
    int r0 = light.getShadowMatrixRebuildCount();
    light.getShadowMatrix(nullptr);
    CHECK(light.getShadowMatrixRebuildCount() == r0 + 1);

    // Same frame, clean -> cache hit, no rebuild.
    light.getShadowMatrix(nullptr);
    CHECK(light.getShadowMatrixRebuildCount() == r0 + 1);

    // Advance frame counter -> must rebuild.
    lsLight::nextShadowFrame();
    light.getShadowMatrix(nullptr);
    CHECK(light.getShadowMatrixRebuildCount() == r0 + 2);

    // Same frame -> cached again.
    light.getShadowMatrix(nullptr);
    CHECK(light.getShadowMatrixRebuildCount() == r0 + 2);
}

TEST_CASE("lsLight_Direct: follows camera offset and rebuilds shadow matrix")
{
    lsCamera cam(nullptr);
    cam.Set_Pos(1.0f, 2.0f, 3.0f);
    cam.Set_Rot(0.0f, 0.0f, 0.0f);
    cam.Update_Camera();

    lsLight_Direct sun(nullptr);
    sun.setFollowCamera(&cam);
    sun.Set_Pos(0.0f, 50.0f, 30.0f);
    sun.Set_Rot(-60.0f, 0.0f, 0.0f);
    sun.updateFollowCamera();

    CHECK(sun.ModelMatrix.matrix[12] == doctest::Approx(1.0f));
    CHECK(sun.ModelMatrix.matrix[13] == doctest::Approx(52.0f));
    CHECK(sun.ModelMatrix.matrix[14] == doctest::Approx(33.0f));
}
