#include "doctest/doctest.h"
#include "scene/lsSky.h"
#include <cmath>

TEST_CASE("lsSky: time of day wraps to [0, 24)")
{
    lsSky sky;
    sky.setTimeOfDay(10.0f);
    CHECK(sky.getTimeOfDay() == doctest::Approx(10.0f));

    sky.setTimeOfDay(25.5f);
    CHECK(sky.getTimeOfDay() == doctest::Approx(1.5f));

    sky.setTimeOfDay(-1.0f);
    CHECK(sky.getTimeOfDay() == doctest::Approx(23.0f));
}

TEST_CASE("lsSky: frozen by default, update advances with day length")
{
    lsSky sky;
    sky.setTimeOfDay(12.0f);
    sky.update(5.0); // dayLength 0 = frozen (capture determinism)
    CHECK(sky.getTimeOfDay() == doctest::Approx(12.0f));

    sky.setDayLength(24.0f); // 24 real seconds per day -> 1 h/s
    sky.update(1.0);
    CHECK(sky.getTimeOfDay() == doctest::Approx(13.0f));

    // wraps past midnight
    sky.setTimeOfDay(23.5f);
    sky.update(1.0);
    CHECK(sky.getTimeOfDay() == doctest::Approx(0.5f));
}

TEST_CASE("lsSky: sun path — east sunrise, high noon, west sunset")
{
    lsSky sky;

    sky.setTimeOfDay(6.0f); // sunrise: east horizon
    lsSkyParams p = sky.computeParams();
    CHECK(p.sunDir.x == doctest::Approx(1.0f).epsilon(0.01));
    CHECK(p.sunDir.y == doctest::Approx(0.0f).epsilon(0.01));

    sky.setTimeOfDay(12.0f); // noon: near zenith (tilted off by sunTilt)
    p = sky.computeParams();
    float maxEl = std::cos(23.5f * 3.14159265f / 180.0f);
    CHECK(p.sunDir.x == doctest::Approx(0.0f).epsilon(0.01));
    CHECK(p.sunDir.y == doctest::Approx(maxEl).epsilon(0.01));

    sky.setTimeOfDay(18.0f); // sunset: west horizon
    p = sky.computeParams();
    CHECK(p.sunDir.x == doctest::Approx(-1.0f).epsilon(0.01));
    CHECK(p.sunDir.y == doctest::Approx(0.0f).epsilon(0.01));

    sky.setTimeOfDay(0.0f); // midnight: sun below, moon up
    p = sky.computeParams();
    CHECK(p.sunDir.y == doctest::Approx(-maxEl).epsilon(0.01));
    CHECK(p.moonDir.y == doctest::Approx(maxEl).epsilon(0.01));

    // direction stays normalized across the whole cycle
    for (float t = 0.0f; t < 24.0f; t += 0.7f)
    {
        sky.setTimeOfDay(t);
        p = sky.computeParams();
        float l = std::sqrt(p.sunDir.x*p.sunDir.x + p.sunDir.y*p.sunDir.y
                          + p.sunDir.z*p.sunDir.z);
        CHECK(l == doctest::Approx(1.0f).epsilon(0.001));
    }
}

TEST_CASE("lsSky: day/night factors track the sun elevation")
{
    lsSky sky;

    sky.setTimeOfDay(12.0f);
    lsSkyParams p = sky.computeParams();
    CHECK(p.dayFactor == doctest::Approx(1.0f));
    CHECK(p.starAmount == doctest::Approx(0.0f));
    CHECK(p.sunIntensity > 0.9f);
    CHECK(p.moonIntensity == doctest::Approx(0.0f));
    CHECK(p.sunsetGlow < 0.05f);

    sky.setTimeOfDay(0.0f);
    p = sky.computeParams();
    CHECK(p.dayFactor == doctest::Approx(0.0f));
    CHECK(p.starAmount == doctest::Approx(1.0f));
    CHECK(p.sunIntensity == doctest::Approx(0.0f));
    CHECK(p.moonIntensity > 0.9f);

    // sunset: warm glow peaks, day factor mid-transition
    sky.setTimeOfDay(18.0f);
    p = sky.computeParams();
    CHECK(p.sunsetGlow == doctest::Approx(1.0f).epsilon(0.02));

    // horizon sun is deep orange, noon sun near-white
    sky.setTimeOfDay(12.0f);
    lsSkyParams noon = sky.computeParams();
    sky.setTimeOfDay(17.5f); // low sun, still day
    lsSkyParams low = sky.computeParams();
    CHECK(low.sunColor.y < noon.sunColor.y); // less green = more orange
    CHECK(low.sunIntensity < noon.sunIntensity);
}

TEST_CASE("lsSky: night sky is much darker than day sky")
{
    lsSky sky;
    sky.setTimeOfDay(12.0f);
    lsSkyParams day = sky.computeParams();
    sky.setTimeOfDay(0.0f);
    lsSkyParams night = sky.computeParams();
    CHECK(night.zenithColor.y < day.zenithColor.y * 0.1f);
    CHECK(night.horizonColor.y < day.horizonColor.y * 0.1f);
    // below-horizon haze is always darker than the horizon
    CHECK(day.groundColor.y < day.horizonColor.y);
    CHECK(night.groundColor.y < night.horizonColor.y);
}
