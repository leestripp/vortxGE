# Plan.md — Real test suite + CTest for vortxGE

> **Status:** ✅ **CORE COMPLETE** (Phases 0-4 done, 2026-08-03) — Phase 5 (CI) declined by user; beyond-math (lsNode) ✅ done 2026-08-03.
> **Next up:** Phase 7 — Dynamic streaming Asset Manager (end goal set by user 2026-08-03: load/unload by distance from camera/player). Phase 6 step 5 (std::expected) lands inside Phase 7 M1.
> **Created:** 2026-08-03, after the 0.1.0 tag + renderer abstraction
> **Previous plan:** the completed renderer-abstraction plan (Phases 0-8)
> is archived at [`docs/plans/renderer_abstraction.md`](docs/plans/renderer_abstraction.md).
>
> **Goal in one sentence:** replace the 32-line "print a matrix and
> eyeball it" `tests/main.cpp` with a real, assertion-based test suite
> (doctest) wired into CTest, starting with the math library — the
> pure-CPU, deterministic foundation every other engine system trusts.

---

## 1. Why this is the next priority

The codebase analysis (2026-08-02) found:

| Signal | State |
|--------|-------|
| Tests | ❌ `tests/main.cpp` = 32 lines of `cout` debugging, no assertions, no framework, not wired to CTest |
| CI | ❌ No `.gitlab-ci.yml` — nothing validates a push except the author |
| Build | ✅ Zero warnings under `-Wall -Wextra`; clean baseline to build on |

Every other roadmap item (threading-model rework, Vulkan backend,
memory-ownership cleanup) becomes **safer and cheaper with a safety
net**. The math library is the right beachhead:

* **Pure CPU, deterministic** — no GL context, no window, no audio device.
* **Foundation code** — `lsMatrix4x4`, `lsQuaternion`, `lsVector*`,
  `lsPlane`, `lsFrustum` are trusted by scene graph, physics, camera,
  and animation. A bug here corrupts everything downstream.
* **Largest untested surface** — `lsMatrix4x4.cpp` (850 lines) and
  `lsQuaternion.cpp` (557 lines) are two of the seven biggest engine
  files and currently have *zero* automated verification.

---

## 2. Current state — inventory

```
tests/
├── CMakeLists.txt     builds `tests` exe, links vortxGE + Bullet + GLEW + EGL
│                      ❌ no add_test(), no enable_testing() anywhere
└── src/
    └── main.cpp       32 lines: prints matrices for visual inspection
```

* Test framework: **none**.
* Assertions in engine code: 16 total (sparse).
* `ctest` currently reports: *"No tests were found!!!"*

---

## 3. Framework decision: **doctest**

| Candidate | Verdict | Reason |
|-----------|---------|--------|
| **doctest** | ✅ **Chosen** | Single header → matches the project's existing vendoring style (tinygltf, stb_image). Fastest compile times of the three. Catch2-compatible syntax (easy migration if ever needed). Ships `doctest::Approx` for float comparison. Has CTest integration. Actively maintained. |
| Catch2 | ❌ | v3 is no longer single-header (needs build/install); v2 single-header is frozen. Heavier compile times. |
| GoogleTest | ❌ | Heaviest weight; wants a build system of its own; over-featured for a ~24k-line engine. |

**Vendor:** `tests/third_party/doctest/doctest.h` (doctest 2.4.x,
latest stable, single header from the upstream repo).

---

## 4. Target layout

```
tests/
├── CMakeLists.txt              ← + add_test(), CTest wiring
├── third_party/
│   └── doctest/
│       └── doctest.h           ← vendored (Phase 0)
└── src/
    ├── test_main.cpp           ← DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN (replaces main.cpp)
    └── math/
        ├── lsVector_tests.cpp        (2/3/4 combined or split — author's choice)
        ├── lsMatrix4x4_tests.cpp
        ├── lsQuaternion_tests.cpp
        ├── lsMatrix3x3_tests.cpp
        ├── lsPlane_tests.cpp
        ├── lsFrustum_tests.cpp
        ├── lsBoundingSphere_tests.cpp
        └── lsRandom_tests.cpp
```

Engine source code does **not** change. Tests link the existing
`vortxGE` static lib and include headers via the existing
`../vortxGE/src/` include path.

---

## 5. Conventions

* **Naming:** one `TEST_CASE("...")` per behaviour, file per class:
  `tests/src/math/<ClassName>_tests.cpp`.
* **Float comparison:** never `==` on floats. Use
  `doctest::Approx(x).epsilon(1e-5)` for general math, `epsilon(1e-4)`
  for trig-heavy paths (slerp, axis-angle). Document any looser epsilon
  inline with a reason.
* **Hand-computed oracles:** every arithmetic test verifies against a
  result computed by hand or by an independent method (e.g.
  `transpose(transpose(m)) == m`), not against the same code path.
* **Determinism:** no random seeds from the clock; `lsRandom` tests pin
  the seed. No GL context, no filesystem, no threads in math tests.
* **Old `main.cpp`:** deleted in Phase 0 — its transpose/multiply
  print-outs are converted into real assertions first.

---

## 6. Phases

### Phase 0 — Skeleton: framework + CTest wiring ✅ **DONE** (2026-08-03)
* Vendor `doctest.h` into `tests/third_party/doctest/`.
* New `tests/src/test_main.cpp` (`DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`).
* `CMakeLists.txt` (root): `enable_testing()` before `add_subdirectory(tests)`.
* `tests/CMakeLists.txt`: `add_test(NAME vortxGE_tests COMMAND tests)`;
  add `third_party/` to includes. (Optional later: `doctest_discover_tests()`
  for per-test-case CTest registration — requires vendoring doctest's
  CMake module too; single `add_test` is sufficient for Phase 0.)
* Convert the old `main.cpp` matrix transpose/multiply prints into the
  first real `TEST_CASE`s; delete `main.cpp`.
* **Acceptance:** `cd build && ctest --output-on-failure` runs green;
  `make tests` compiles with zero new warnings.

### Phase 1 — lsVector2 / lsVector3 / lsVector4 ✅ **DONE** (2026-08-03)
Arithmetic (`+ - * /`), dot, cross (v3), length/normalize (incl.
zero-length guard), lerp, index/accessor sanity, equality.
* **Acceptance:** all four-component paths covered; edge cases
  (normalize(0), cross of parallel vectors) asserted.

### Phase 2 — lsMatrix4x4 ✅ **DONE** (2026-08-03)
Identity (`isIdentity`, `isEqual`), `transpose` round-trip, `operator*`
(hand-computed 4×4 product; non-commutativity), matrix×`lsVector3`,
`Translate/Rotate/ScaleMatrix` composition order, `Set(quaternion)`
vs `q.toMatrix4x4()` (cross-check with Phase 3), `PerspectiveProjection`
/`OrthographicProjection` invariants (z-mapping of near/far planes),
`LookAt` properties (forward = normalize(lookAt−eye)), `extractRotation`,
`ToEulerZYX` round-trip.
* **Acceptance:** the 850-line file has every public method touched by
  at least one assertion; one composition test mirrors a real
  scene-graph transform chain.

### Phase 3 — lsQuaternion ✅ **DONE** (2026-08-03)
Axis-angle constructor ↔ `fromAxisAngle` consistency, `magnitude`/
`normalize`, `conjugate`/`inverse` (`q * q.inverse() == identity`),
multiplication non-commutativity, `Slerp` endpoints (t=0, t=1 exact;
t=0.5 halfway angle), `Lerp`, `toMatrix4x4` orthonormality.
* **Acceptance:** slerp half-angle verified against `dot` of the two
  endpoint quats; all within `epsilon(1e-4)`.

### Phase 4 — Remaining math ✅ **DONE** (2026-08-03)
`lsMatrix3x3`, `lsPlane` (point distance, normalize), `lsFrustum`
(plane extraction from a known projection; contains/outside corners),
`lsBoundingSphere` (radius growth on merge), `lsMath` helpers,
`lsRandom` (pinned-seed determinism + range bounds).
* **Acceptance:** `vortxGE/src/math/` — every public class has a test
  file; no file in the directory is untested.

### Phase 5 — *(follow-up, optional)* CI + beyond-math 🔲
* `.gitlab-ci.yml`: deps image → `cmake && make && ctest` on every push
  (this is roadmap item #2 — trivial once Phases 0-4 exist).
* Expand to other pure-CPU systems: `properties/` (reflective set/get),
  `events/` (bus dispatch), `assets/` path handling.
* **Not** GL-render tests — those need a headless GL context
  (OSMesa/EGL surfaceless) and are a separate plan.

### Phase 6 — C++23 migration ✅ **DONE** (2026-08-03, steps 1-4; step 5 adoption ongoing)

**Goal:** move the whole project from C++20 to **C++23** to unlock
modern async and memory/error-handling facilities.

**Feasibility (checked 2026-08-03):** toolchain is GCC 15.2 — full
C++23 compiler + libstdc++ support. Current standard is C++20
(`CMAKE_CXX_STANDARD 20`, REQUIRED ON), so the delta is small.

**Feature mapping (what actually serves the goals):**
* *Async calls:* coroutines (`co_await`/`co_return` — language core
  since C++20) plus C++23 library additions (`std::generator`).
  No `std::async` replacement exists — real async adoption means
  designing coroutine-aware task/scheduler types (separate design task).
* *Memory/error handling:* `std::expected` for fallible APIs (mesh/
  asset loading), `std::out_ptr`/`std::inout_ptr` for C-API interop,
  move-only improvements. Smart-pointer story is unchanged (already
  `unique_ptr` in scene graph).
* *Nice-to-haves:* `std::print` (replace printf/iostream debug spam),
  `std::mdspan` (candidate for matrix/vertex buffer views),
  multidimensional subscript `m[i, j]`, `if consteval`, ranges `to<>`.

**Steps:**
1. Flip `CMAKE_CXX_STANDARD` 20 → 23 in root `CMakeLists.txt`.
2. Full rebuild — zero new warnings policy stays in force.
3. **Regression gate: entire test suite must stay green**
   (47 cases / 1692 assertions via `ctest --output-on-failure`) —
   this is exactly what Phases 0-4 were built for.
4. Smoke-test `vortxGEdemo_glfw3` (runs ≥60 s without crash).
5. Adopt C++23 features **incrementally**, one focused commit at a
   time (no big-bang rewrite): start with `std::expected` in asset
   loading and `std::print` in debug output.

**Rollback:** single-line revert of the standard flag; feature-adoption
commits are independent and individually revertable.

### Phase 7 — Dynamic streaming Asset Manager 🔲 **VISION** (added 2026-08-03)

**End goal (user):** assets load and unload **dynamically based on
distance from the camera/player** — open-world style streaming instead
of the current eager load-everything-at-init model.

**Current state (scouted 2026-08-03):**
* `lsAssetManager` = raw-pointer factory registry (`CreateAsset_*` per
  type, `Find_Asset`, `Add_Asset`) — eager loading, no lifetime mgmt.
* `lsAsset::AllocAsset()` = polymorphic virtual, ~8 overrides,
  GL-coupled (needs `getOpenGL()`), errors via `void` + `cout`.
* Demo evidence: whole scene (13 objects, all meshes) loads at init.
* Bullet shape assets depend on mesh assets (dependency tracking needed).

**Design pillars:**
1. **Two-stage pipeline** — Stage A (worker threads, async): file I/O +
   parse into CPU-side buffers. Stage B (main thread only): GPU upload
   (GL context lives on the main thread). This split is what makes
   async loading possible at all.
2. **Distance-based priority queue** — each streamable asset gets a
   world position (from its node); load priority = f(distance to
   camera); eviction beyond a radius, with a hysteresis band to avoid
   load/unload thrash.
3. **Async via C++23** — start with a thread pool + futures (simple,
   safe); coroutine task/scheduler types later once designed
   (Phase 6 step 5 feeds this).
4. **`std::expected` error handling** — new async load APIs return
   `expected<Data, Error>`; this is where Phase 6 step 5 actually lands
   (replaces the void+cout pattern for new code).
5. **Lifetime** — shared/weak ownership or handle-based refs; LRU
   eviction for far assets; dependency tracking (bullet shapes → meshes).

**Milestones (each independently committable):**
* **M1:** async file loader — worker-thread file I/O returning
  `std::expected`; pure CPU → **unit-testable in the suite**.
* **M2:** split `lsMesh` loading into CPU parse (vbm/gltf → buffers)
  vs GPU upload; parse stage testable.
* **M3:** distance-based priority + eviction logic in the manager;
  priority/eviction rules unit-testable with mocked distances.
* **M4:** demo integration — objects stream in/out as the player moves.

**Regression gate:** full ctest suite + 60 s demo smoke per milestone.

---

## 7. Out of scope

* Rendering/visual tests (need GL context — separate effort, OSMesa or
  EGL surfaceless + image diff).
* Physics integration tests (Bullet world stepping — comes with CI).
* Threading tests (`lsNode::MoveThread`) — pending the threading-model
  review (roadmap item #3).
* Engine refactors — if a test reveals a bug, file it; don't fix
  engine code inside this plan unless the fix is trivial and isolated.

---

## 8. Metrics

| Metric | Baseline | Phase 0 | Phase 4 target |
|--------|---------:|--------:|---------------:|
| Assertion-based test cases | 0 | ≥ 2 | ≥ 60 |
| `ctest` discovers tests | no | yes | yes |
| Math classes with a test file | 0 / 11 | 0 | 11 / 11 |
| Engine files modified | — | 0 | 0 |
| New build warnings | — | 0 | 0 |

## 9. Definition of done (whole plan)

`ctest --output-on-failure` green from a clean build; every class in
`vortxGE/src/math/` covered; conventions in §5 followed; CHANGELOG
`[Unreleased]` updated per phase; AGENTS.md gains a "Running tests"
section in Phase 0.
