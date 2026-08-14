// ============================================================================
// Terrain module tests (plan-2026-08-terrain.md §9) — doctest suites for the
// built-in terrain system: heightfield, noise, erosion, biomes, masks,
// vegetation scattering, LOD selection, and the OBJ mesh loader.
// ============================================================================

#include "doctest/doctest.h"

#include "terrain/Biome.h"
#include "terrain/Erosion.h"
#include "terrain/Heightfield.h"
#include "terrain/Masks.h"
#include "terrain/Mesh.h"
#include "terrain/Noise.h"
#include "terrain/lod/ChunkLod.h"
#include "terrain/veg/Scatterer.h"

#include <cmath>
#include <string>
#include <vector>

namespace {

// Repo root for data-file tests (compile definition from tests/CMakeLists).
std::string resPath(const char* rel) {
    return std::string(VORTXGE_SOURCE_DIR) + "/" + rel;
}

} // namespace

TEST_CASE("terrain::Heightfield sampling and normalize") {
    terrain::Heightfield hf(4, 4);
    for (std::size_t y = 0; y < 4; ++y)
        for (std::size_t x = 0; x < 4; ++x)
            hf(x, y) = static_cast<float>(x + y); // 0..6

    CHECK(hf(0, 0) == 0.0f);
    CHECK(hf(3, 3) == 6.0f);
    CHECK(std::fabs(hf.sampleBilinear(0.5f, 0.5f) - 3.0f) < 1e-5f);
    CHECK(std::fabs(hf.sampleBilinear(0.0f, 0.0f) - 0.0f) < 1e-6f);
    CHECK(std::fabs(hf.sampleBilinear(1.0f, 1.0f) - 6.0f) < 1e-6f);

    hf.normalize(10.0f, 20.0f);
    CHECK(std::fabs(hf.minValue() - 10.0f) < 1e-4f);
    CHECK(std::fabs(hf.maxValue() - 20.0f) < 1e-4f);

    terrain::Heightfield flat(8, 8, 0.5f);
    CHECK(terrain::slopeDegAt(flat, 0.5f, 0.5f, 100.0f, 50.0f) < 1e-3f);
}

TEST_CASE("terrain::Noise determinism and ranges") {
    const terrain::Noise a(123);
    const terrain::Noise b(123);
    const terrain::Noise c(124);
    CHECK(a.noise(3.7f, -2.1f) == b.noise(3.7f, -2.1f));
    CHECK(a.noise(3.7f, -2.1f) != c.noise(3.7f, -2.1f));
    for (int i = 0; i < 2000; ++i) {
        const float x = static_cast<float>(i) * 0.173f;
        const float y = static_cast<float>(i) * 0.311f;
        const float n = a.noise(x, y);
        CHECK(n >= -1.0f);
        CHECK(n <= 1.0f);
        const float r = a.ridged(x, y, 4);
        CHECK(r >= 0.0f);
        CHECK(r <= 1.0f);
    }
}

TEST_CASE("terrain::erode determinism, finite non-negative heights") {
    terrain::Noise n(7);
    auto makeField = [&] {
        terrain::Heightfield hf(64, 64);
        for (std::size_t y = 0; y < 64; ++y)
            for (std::size_t x = 0; x < 64; ++x)
                hf(x, y) = n.fbm(static_cast<float>(x) * 0.05f,
                                 static_cast<float>(y) * 0.05f, 4) * 0.5f + 0.5f;
        return hf;
    };

    terrain::ErosionParams p;
    p.seed = 99;
    p.droplets = 5000;
    p.brushRadius = 2;

    auto hf = makeField();
    terrain::erode(hf, p);
    for (std::size_t i = 0; i < hf.width() * hf.height(); ++i) {
        CHECK(std::isfinite(hf.data()[i]));
        CHECK(hf.data()[i] >= -1e-3f);
    }

    auto hf2 = makeField();
    terrain::erode(hf2, p);
    for (std::size_t i = 0; i < hf.width() * hf.height(); ++i)
        CHECK(hf.data()[i] == hf2.data()[i]);
}

TEST_CASE("terrain::classifyBiome table") {
    CHECK(terrain::classifyBiome(0.1f, 0.8f, 0.8f) == terrain::Biome::Ocean);
    CHECK(terrain::classifyBiome(0.31f, 0.8f, 0.8f) == terrain::Biome::Beach);
    CHECK(terrain::classifyBiome(0.95f, 0.8f, 0.8f) == terrain::Biome::Snow);
    CHECK(terrain::classifyBiome(0.5f, 0.9f, 0.1f) == terrain::Biome::Desert);
    CHECK(terrain::classifyBiome(0.5f, 0.5f, 0.8f) == terrain::Biome::Forest);
    CHECK(terrain::classifyBiome(0.5f, 0.1f, 0.5f) == terrain::Biome::Tundra);
}

TEST_CASE("terrain::altitudeBand fuzzy membership") {
    CHECK(terrain::altitudeBand(0.5f, 0.4f, 0.6f, 0.05f) > 0.99f);
    CHECK(terrain::altitudeBand(0.2f, 0.4f, 0.6f, 0.05f) < 0.01f);
    const float edge = terrain::altitudeBand(0.4f, 0.4f, 0.6f, 0.05f);
    CHECK(edge > 0.4f);
    CHECK(edge < 0.6f);
}

TEST_CASE("veg::Scatterer determinism, rules, chunk clipping") {
    terrain::Heightfield hf(64, 64, 0.5f);
    std::vector<terrain::Biome> biomes(64 * 64, terrain::Biome::Grassland);

    terrain::veg::Scatterer sc(hf, biomes, 256.0f, 100.0f);
    terrain::veg::VegetationLayer grass;
    grass.id = 1;
    grass.name = "grass";
    grass.rules.maxSlopeDeg = 45.0f;
    grass.rules.minSpacing = 4.0f;
    grass.rules.density = 1.0f;

    const auto a = sc.scatter(grass, 2, 3, 4, 42);
    const auto b = sc.scatter(grass, 2, 3, 4, 42);
    REQUIRE(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        CHECK(a[i].x == b[i].x);
        CHECK(a[i].y == b[i].y);
        CHECK(a[i].z == b[i].z);
    }
    CHECK(!a.empty());

    // Chunk (2,3) of 4x4 over 256 m = [128,192) x [192,256); clipped at edges.
    for (const auto& inst : a) {
        CHECK(inst.x >= 128.0f);
        CHECK(inst.x < 192.0f);
        CHECK(inst.y >= 192.0f);
        CHECK(inst.y < 256.0f);
        CHECK(std::fabs(inst.z - 50.0f) < 1e-2f); // 0.5 * heightScale
    }

    // Altitude band that misses the field entirely → no instances.
    grass.rules.minAltitude = 0.9f;
    grass.rules.maxAltitude = 1.0f;
    CHECK(sc.scatter(grass, 0, 0, 4, 42).empty());
}

TEST_CASE("lod::ChunkLod coverage and morph weights") {
    terrain::lod::ChunkLod lod(1024.0f, 4);
    const auto sel = lod.select(0.0f, 0.0f);
    REQUIRE(!sel.nodes.empty());

    bool hasFine = false;
    bool hasCoarse = false;
    double area = 0.0;
    for (const auto& n : sel.nodes) {
        if (n.level == 4)
            hasFine = true;
        if (n.level <= 1)
            hasCoarse = true;
        CHECK(n.morph >= 0.0f);
        CHECK(n.morph <= 1.0f);
        if (n.level == 4)
            CHECK(n.morph == 0.0f); // deepest level never morphs
        const double s = static_cast<double>(lod.nodeSize(n.level));
        area += s * s;
    }
    CHECK(hasFine);
    CHECK(hasCoarse);
    // Full coverage: node areas tile the world without gaps/overlaps.
    CHECK(std::fabs(area - 1024.0 * 1024.0) < 1.0);
}

TEST_CASE("terrain::loadObj — example vegetation meshes") {
    const char* files[] = {
        "demos/data/vegetation/grass_clump.obj",
        "demos/data/vegetation/fern.obj",
        "demos/data/vegetation/bush.obj",
        "demos/data/vegetation/broadleaf.obj",
        "demos/data/vegetation/conifer.obj",
    };
    for (const char* f : files) {
        auto mesh = terrain::loadObj(resPath(f));
        INFO(f, ": ", mesh ? "ok" : mesh.error());
        REQUIRE(mesh.has_value());
        CHECK(!mesh->vertices.empty());
        CHECK(mesh->triangleCount() > 0);
        CHECK(mesh->indices.size() % 3 == 0);
        for (const auto i : mesh->indices)
            CHECK(i < mesh->vertices.size());
        // Plants stand on the ground: base near y=0, some height.
        CHECK(mesh->aabbMin[1] < 0.1f);
        CHECK(mesh->height() > 0.1f);
    }

    auto grass = terrain::loadObj(resPath("demos/data/vegetation/grass_clump.obj"));
    auto conifer = terrain::loadObj(resPath("demos/data/vegetation/conifer.obj"));
    REQUIRE(grass.has_value());
    REQUIRE(conifer.has_value());
    CHECK(conifer->height() > grass->height());

    CHECK(!terrain::loadObj(resPath("demos/data/vegetation/does_not_exist.obj"))
               .has_value());
}

// ---------------------------------------------------------------------------
// Engine bridge: grid mesh generation (pure geometry, no GL)
// ---------------------------------------------------------------------------
#include "terrain/TerrainMesh.h"

TEST_CASE("terrain::generateGridMesh — geometry from heightfield") {
    terrain::Heightfield hf(8, 8);
    for (std::size_t y = 0; y < 8; ++y)
        for (std::size_t x = 0; x < 8; ++x)
            hf(x, y) = static_cast<float>(x) / 7.0f; // ramp along x

    const auto g = terrain::generateGridMesh(hf, 70.0f, 10.0f);

    REQUIRE(g.positions.size() == 8 * 8 * 3);
    REQUIRE(g.normals.size() == 8 * 8 * 3);
    REQUIRE(g.uvs.size() == 8 * 8 * 2);
    REQUIRE(g.indices.size() == 6 * 7 * 7);

    // centred: corners at +/-35 in x/z
    CHECK(g.positions[0] == doctest::Approx(-35.0f));   // x of (0,0)
    CHECK(g.positions[2] == doctest::Approx(-35.0f));   // z of (0,0)
    const auto last = (8 * 8 - 1) * 3;
    CHECK(g.positions[last + 0] == doctest::Approx(35.0f));
    CHECK(g.positions[last + 2] == doctest::Approx(35.0f));

    // heights: y = h * maxHeight; left column 0, right column 10
    CHECK(g.positions[1] == doctest::Approx(0.0f));
    CHECK(g.positions[last + 1] == doctest::Approx(10.0f));

    // indices in range, normals unit length and up-leaning
    for (const auto i : g.indices)
        CHECK(i < 8 * 8);
    for (std::size_t i = 0; i < 64; ++i) {
        const float nx = g.normals[i * 3 + 0];
        const float ny = g.normals[i * 3 + 1];
        const float nz = g.normals[i * 3 + 2];
        CHECK(std::sqrt(nx * nx + ny * ny + nz * nz) == doctest::Approx(1.0).epsilon(0.01));
        CHECK(ny > 0.5f);
        CHECK(nx < 0.0f); // ramp rises along +x -> normal leans -x
    }
}

// ---------------------------------------------------------------------------
// Bullet heightfield collision: sphere drop + raycast
// ---------------------------------------------------------------------------
#include "datablocks/lsBulletShape.h"
#include <bullet/btBulletDynamicsCommon.h>

TEST_CASE("lsBulletShape::Make_Heightfield — collision behaviour") {
    // Flat field at normalized 0.5, heightScale 10 -> surface at y=5.
    constexpr int kW = 16, kH = 16;
    constexpr float kScale = 10.0f, kCell = 2.0f;
    std::vector<float> flat(static_cast<std::size_t>(kW) * kH, 0.5f);

    lsBulletShape shape(nullptr);
    shape.Make_Heightfield(kW, kH, flat.data(), kScale, kCell, kCell);
    REQUIRE(shape.getShape() != nullptr);

    btDefaultCollisionConfiguration conf;
    btCollisionDispatcher dispatcher(&conf);
    btDbvtBroadphase broadphase;
    btSequentialImpulseConstraintSolver solver;
    btDiscreteDynamicsWorld world(&dispatcher, &broadphase, &solver, &conf);
    world.setGravity(btVector3(0, -10, 0));

    // static terrain body at origin
    btTransform groundXf;
    groundXf.setIdentity();
    groundXf.setOrigin(btVector3(0, kScale * 0.5f, 0)); // Y-centred shape
    auto* groundMotion = new btDefaultMotionState(groundXf);
    btRigidBody::btRigidBodyConstructionInfo groundCi(
        0.0f, groundMotion, shape.getShape(), btVector3(0, 0, 0));
    auto* ground = new btRigidBody(groundCi);
    world.addRigidBody(ground);

    // drop a 0.5m sphere from y=20 over the centre
    auto* sphereShape = new btSphereShape(0.5f);
    btTransform sphereXf;
    sphereXf.setIdentity();
    sphereXf.setOrigin(btVector3(0, 20, 0));
    auto* sphereMotion = new btDefaultMotionState(sphereXf);
    btVector3 inertia(0, 0, 0);
    sphereShape->calculateLocalInertia(1.0f, inertia);
    btRigidBody::btRigidBodyConstructionInfo sphereCi(
        1.0f, sphereMotion, sphereShape, inertia);
    auto* sphere = new btRigidBody(sphereCi);
    world.addRigidBody(sphere);

    for (int i = 0; i < 240; ++i)
        world.stepSimulation(1.0f / 60.0f, 4);

    const float restY = sphere->getWorldTransform().getOrigin().getY();
    CHECK(restY == doctest::Approx(5.5).epsilon(0.05)); // surface 5 + radius

    // raycast down onto the field from above: hit at y=5
    btCollisionWorld::ClosestRayResultCallback ray(
        btVector3(4, 50, -4), btVector3(4, -50, -4));
    world.rayTest(btVector3(4, 50, -4), btVector3(4, -50, -4), ray);
    REQUIRE(ray.hasHit());
    CHECK(ray.m_hitPointWorld.getY() == doctest::Approx(5.0).epsilon(0.02));

    world.removeRigidBody(sphere);
    world.removeRigidBody(ground);
    delete sphere; delete sphereMotion; delete sphereShape;
    delete ground; delete groundMotion;
}
