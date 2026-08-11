# AGENTS.md — Guide for AI Coding Agents working on vortxGE

This file gives an AI coding agent everything needed to be productive in the
vortxGE codebase: what the project is, how it is built, how the code is
organised, and the conventions every contributor (human or AI) is expected
to follow.

> Companion documents: `Readme.md` (user-facing overview), `docs/` (in-depth
> architecture notes), `LICENSE` (MIT, © 2025 Lee A. Stripp).

---

## 1. Project Overview

**vortxGE** is a lightweight, modern 3D graphics engine written in **C++20**.
It targets Linux, real-time graphics applications, and simulations.

Highlights (see `Readme.md` for the full feature list):

* OpenGL 4.3+ pipeline with a **pass-based** renderer
  (direct-to-screen, FBO render-to-texture, occlusion, shadow map).
* **Physically Based Rendering** via a Cook‑Torrance BRDF (albedo, normal,
  metallic/roughness/AO maps).
* **Dynamic shadow mapping** for spot lights.
* **Hierarchical scene graph** (`lsNode`) with Point and Spot lights, plus
  Cameras, Objects, Emitters, Particles, First‑person rigs, etc.
* **Bullet Physics** integration — Box, Capsule, Plane, Convex Hull,
  Triangle Mesh collision shapes; physics-based FPS character controller
  (walk, sprint, crouch, jump).
* **glTF / .glb** model loading via the bundled `tinygltf` datablock.
* **Custom Blender addon** (`blender_scripts/export_scene.py`) that emits
  `.vgs` (vortxGE Scene) hierarchy files plus per-mesh `.glb` files.
* **Keyframe animation** with cubic-curve interpolation for node transforms
  and material properties.
* Two reference demo front-ends: **GLFW3** and **GTKmm 4.0**.

The engine version is **derived from the latest git tag** at CMake
configure time (tag `0.1.0` → version `0.1.0`). CMake generates
`config.h` from the template `vortxGE/src/config.h.in` into
`build/generated/`, defining `vortxGE_VERSION_MAJOR/MINOR/PATCH` plus
`vortxGE_VERSION_STRING` and `vortxGE_VERSION_GIT` (full `git describe`
output). To cut a release: create a semver git tag and re-run CMake —
no code edits needed. Builds without git metadata (tarball/zip) use the
fallback version pinned in the root `CMakeLists.txt`. Do **not** create
or commit a static `vortxGE/src/config.h` — it would shadow the
generated header.

### Coordinate system
Right-handed, view along **-Z** (`+X` right, `+Y` up). Documented in
`docs/scene/SceneGraph.md` and `docs/vortxGE.md`.

---

## 2. Tech Stack

| Layer | Technology | Where it appears |
|-------|-----------|------------------|
| Language | **C++20** (`-std=c++20`, `-Wall -Wextra`) | All sources |
| Compiler | GCC or Clang (any C++20-capable) | Toolchain |
| Build system | **CMake ≥ 3.29** | Root + per-module `CMakeLists.txt` |
| Rendering | **OpenGL 4.3+** (GLVND preference) | `vortxGE/src/render/` |
| GL loading | **GLEW** | `vortxGE/src/render/`, demos |
| Math | Custom (`lsVector2/3/4`, `lsMatrix3x3/4x4`, `lsQuaternion`, `lsPlane`, `lsFrustum`, `lsBoundingSphere`) | `vortxGE/src/math/` |
| Physics | **Bullet Physics** | `vortxGE/src/physics/` |
| Audio | **OpenAL** | `vortxGE/src/openal/lsOpenAL.*` |
| Asset loading | **tinygltf** (bundled), **stb_image** (bundled) | `vortxGE/src/datablocks/tinygltf/` |
| Demo A | **GLFW3** (via `pkg-config`) | `demos/vortxGEdemo_glfw3/` |
| Demo B | **gtkmm-4.0** + **glibmm-2.68** (via `pkg-config`) | `demos/vortxGEdemo_gtkmm/` |
| Tools | `obj2vbm` OBJ→mesh converter | `tools/obj2vbm/` |
| Asset authoring | Blender + Python addon | `blender_scripts/export_scene.py` |

**System packages required** (Debian/Ubuntu names; adapt to your distro):

```bash
sudo apt install \
    cmake build-essential \
    libbullet-dev libgl-dev libglew-dev libopenal-dev \
    libglfw3-dev \
    libgtkmm-4.0-dev libglibmm-2.68-dev
```

---

## 3. Repository Layout

```
vortxGE/                      ← repo root
├── CMakeLists.txt            ← top-level CMake, picks up every subproject
├── Readme.md                 ← user-facing docs
├── LICENSE                   ← MIT
├── .gitignore                ← ignores build/, secrets/, blender/, game/
├── AGENTS.md                 ← this file
│
├── vortxGE/                  ← Engine library (STATIC)
│   ├── CMakeLists.txt        ← file(GLOB_RECURSE src/*.cpp) → libvortxGE.a
│   └── src/
│       ├── vortxGE.{h,cpp}   ← Engine entry point (Init/Update/Render/Cleanup)
│       ├── config.h          ← VORTX version macros
│       ├── vortx_debug.h     ← #define VORTX_DEBUG_* switches
│       ├── animation/        ← Keyframe animation (lsAnimProperty*, lsCubicCurve, lsKeyFrame, lsAnimationManager)
│       ├── assets/           ← Asset loader (lsAssetManager, lsAsset_Mesh, lsAsset_Texture,
│       │                       lsAsset_Quad, lsAsset_MeshGuide_*, lsAsset_BulletShape_*)
│       ├── controls/         ← Input layer (lsControlManager, lsControl, lsComposite, lsViewer)
│       ├── datablocks/       ← Data containers + bundled tinygltf/{json.hpp, stb_image.h, tiny_gltf.h}
│       ├── events/           ← Event bus (lsEvent, lsEventManager, lsEvent_DelNode, lsEvent_HitPoint)
│       ├── filesystem/       ← File/directory helpers
│       ├── math/             ← Vector/matrix/quaternion/frustum/plane/bounds
│       ├── openal/           ← lsOpenAL (3D audio)
│       ├── passes/           ← lsPass + lsPass_DR / lsPass_FBO / lsPass_Occlusion / lsPass_ShadowMap
│       ├── physics/          ← lsBulletWorld, lsBulletObject, lsRayCast, lsFilterCallback
│       ├── properties/       ← Reflective property system (lsProperty, lsProperty_Float/Vector*)
│       ├── render/           ← Backend-agnostic renderer abstraction (lsRender)
       │                       plus its single concrete implementation
       │                       (lsOpenGL). All other engine code talks to
       │                       GL only via `getRender()->...`. See
       │                       §5.9 “Renderer abstraction” below.
       │
       │     lsGLTypes.h        Engine-level handle/scalar types
       │                        (lsTextureID, lsEnum, lsFloat, lsUInt, …)
       │                        — no <GL/glew.h>, no GL_* identifiers.
       │     lsRenderConstants.h Engine-level constants (LS_TEXTURE_2D,
       │                        LS_TRIANGLES, …) — currently #defined
       │                        to GL_* via this header; a future
       │                        non-GL backend rewrites this one file.
       │                        The only header besides lsOpenGL.cpp
       │                        that includes <GL/glew.h>.
       │     lsRender.h         Pure abstract interface, 75+ virtuals.
       │                        NO <GL/glew.h>, NO GL_* identifiers.
       │                        Pulls in lsGLTypes.h + lsRenderConstants.h
       │                        so every consumer gets types + constants.
       │     lsOpenGL.h         `class lsOpenGL : public lsRender` —
       │                        overrides.
       │     lsOpenGL.cpp       THE ONLY file with raw gl*() calls —
       │                        one-liner wrappers around gl*() that
       │                        cast LS_* constants to GL_* and
       │                        reinterpret engine-level handle pointers
       │                        to GLuint*/GLint*/GLsizei*.
       │     lsVulkan.{h,cpp}   PLACEHOLDER skeleton for the future
       │                        Vulkan backend — overrides all lsRender
       │                        pure virtuals with no-op stubs. NOT
       │                        implemented; constructor warns on use.
       │                        See docs/plans/renderer_abstraction.md §10 for the recipe.
│       ├── scene/            ← Scene graph (lsNode base, lsObject, lsCamera, lsLight*,
│       │                       lsScene, lsSceneManager, lsFirstPersonRig, lsAnimNode,
│       │                       lsObjectEmitter, lsSpeaker, lsListener)
│       ├── shader/           ← lsShader, lsShaderManager, lsShaderKinds
│       └── timers/           ← lsTimer (high-resolution timing)
│
├── tests/                    ← doctest test suite (CTest-wired; src/math/*_tests.cpp,
│                               test_main.cpp entry, vendored third_party/doctest/)
│   ├── CMakeLists.txt
│   └── src/main.cpp
│
├── tools/
│   └── obj2vbm/              ← Stand-alone OBJ→vbm mesh converter
│       ├── CMakeLists.txt
│       ├── Readme.md
│       └── src/{convert,filesystem,mesh}/*.cpp
│
├── demos/
│   ├── data/                 ← Audio, demoscene.vgs, meshes/, shaders/ (used by both demos)
│   ├── shared_code/          ← demoscene.{h,cpp} shared between both demo front-ends
│   ├── vortxGEdemo_glfw3/    ← GLFW3 demo (WASD + mouse FPS controller)
│   │   ├── CMakeLists.txt
│   │   └── src/glfwApp.{h,cpp}
│   └── vortxGEdemo_gtkmm/    ← GTKmm 4 demo (engine embedded in a Gtk::GLArea)
│       ├── CMakeLists.txt
│       └── src/{main.cpp, glwindow.{h,cpp}}
│
├── blender_scripts/
│   └── export_scene.py       ← Blender addon (File → Export → vortxGE Scene .vgs)
│
├── docs/                     ← Markdown design notes
│   ├── vortxGE.md            ← High-level architecture & lifecycle
│   ├── blender_export.md
│   ├── glfw_desktop_scale_factor.md
│   ├── animation/ assets/ audio/ controls/ datablocks/ demos/ events/
│   ├── math/ passes/ physics/ properties/ scene/ shader/ utils/
│
└── build/                    ← Out-of-tree CMake build dir (gitignored)
```

`vortxGE/CMakeLists.txt` discovers every source file with
`file(GLOB_RECURSE src/*.cpp)`, so adding a new `.cpp` anywhere under
`vortxGE/src/` is enough — **no manual edit to the CMake file is required**
on the next `cmake` reconfigure (or `make` if the build is already
configured — CMake re-globs when the source list changes).

A top-level optional subdirectory `./game/` is also picked up if it exists;
it is gitignored and intended for per-project game code.

---

## 4. Build & Test Commands

All commands assume the repo root as the starting point. Builds are
**out-of-tree** under `build/`.

### Configure & build (debug — default)

```bash
mkdir -p build
cd build
cmake ..
make -j"$(nproc)"
```

`cmake ..` configures everything: the `vortxGE` static library, `tests`,
the `obj2vbm` tool, and both demos (`vortxGEdemo_glfw3` and
`vortxGEdemo_gtkmm`). Two extra `make` targets wire up the data symlinks
that the demos need:

```bash
make symlink_glfw3 symlink_gtkmm
```

### Configure & build (release)

In `CMakeLists.txt` swap the commented/uncommented line:

```cmake
set(CMAKE_BUILD_TYPE Release)
```

then `cmake .. && make -j`. The release flags are `-O3`; debug is `-g`.

### Run the tests

The test suite uses **doctest** (vendored single header,
`tests/third_party/doctest/doctest.h`) and is registered with **CTest**:

```bash
cd build
ctest --output-on-failure      # run everything via CTest
./tests/tests                  # run the doctest binary directly
./tests/tests --test-case="lsMatrix4x4*" --success   # doctest CLI filters
```

Tests live in `tests/src/` (entry point `test_main.cpp`, suites under
`tests/src/math/`). Conventions — float compares use `doctest::Approx`,
oracles are hand-computed, tests are deterministic — are documented in
`Plan.md` §5, which also holds the phased coverage plan (Phases 0-5).

### Run the demos

```bash
cd build
./demos/vortxGEdemo_glfw3/vortxGEdemo_glfw3
./demos/vortxGEdemo_gtkmm/vortxGEdemo_gtkmm
```

The GLFW3 demo controls (from `Readme.md`): WASD move, mouse look, Space
jump, Left Shift sprint, Left Ctrl crouch, F1 fullscreen toggle, F12 save
screenshot (PNG, `screenshot_<timestamp>.png` in the working directory),
Esc quit.

### Debug tools compile-time gate (D-series)

CMake option `VORTX_DEBUG_TOOLS` (default ON for Debug, OFF for Release)
compiles ALL diagnostic env vars out of the binary — Release builds
contain none of them (`strings`-verified). The diagnostics table below
only applies to builds with the tools enabled. The builtin screenshot
(`lsRender::Screenshot`, F12) is a feature, NOT gated. The `build/` dir
is a Release build with tools force-enabled
(`cmake -DVORTX_DEBUG_TOOLS=ON`).

### Screenshots & diagnostics (env vars)

Builtin screenshot: `lsRender::Screenshot(path)` (both backends; `.png`
extension selects PNG via stb_image_write, otherwise binary PPM). GLFW
demo: F12. Headless / scripted captures use env vars:

| Env var | Effect |
|---|---|
| `VORTX_RENDERER=vulkan\|opengl` | backend selection (Vulkan tried first by default) |
| `VORTX_PIPELINE=1` | explicit render-pipeline mode |
| `VORTX_SHOT=path.png` | one-shot screenshot at frame 30 (both backends) |
| `VORTX_FBO_SHOT=id:path.png` | Vulkan: read back FBO `id`'s colour attachment at frame 30 |
| `VORTX_VK_DEBUG=1` | Vulkan: dump draw/VAO/UBO counters at frame 30 |
| `VORTX_VK_NOCULL` / `VORTX_VK_CCW` | Vulkan: pipeline cull-mode / front-face overrides |
| `VORTX_DISABLE_SHADOWS=1` | force `use_shadowmap=0` (shadow-related A/B tests) |
| `VORTX_RDC=1` or `=template` | RenderDoc: trigger capture at frame 30 (launch under `renderdoccmd capture`; needs X11/XCB — RenderDoc has no Wayland support, use `DISPLAY=:99 GLFW_PLATFORM=x11`) |

### Build only a single target

```bash
make vortxGE                # static library only
make vortxGEdemo_glfw3      # GLFW3 demo
make vortxGEdemo_gtkmm      # GTKmm demo
make obj2vbm                # standalone OBJ tool
make tests                  # test program
```

### Adding your own game sub-project

Create `./game/` (any directory layout) with its own `CMakeLists.txt`. The
root `CMakeLists.txt` auto-detects it with `if(EXISTS "${GAME_DIR}")` and
runs `add_subdirectory(${GAME_DIR})`. This is the supported extension
point — your game code can `#include` headers from `vortxGE/src/` and link
against the `vortxGE` target.

---

### CI/CD policy: deliberately NONE (user decision, 2026-08-03)

This project has **no CI pipelines** — do not add `.gitlab-ci.yml` or
recommend hosted CI. Rationale: GitLab Free tier (400 CI min/month) is
too small for this build size, and hosted Windows/macOS runners are
Premium-only anyway. Builds and the full doctest suite are run **locally
before every push** — that discipline replaces CI. If the engine is ever
ported to Windows/Mac, revisit with **self-hosted runners** (free,
unlimited minutes), not hosted CI.

## 5. Code Style and Conventions

These rules are derived from the existing source — please follow them when
adding or modifying code so the codebase stays consistent.

### 5.1 Naming

* **All engine types use the `ls` prefix** (legacy "L-Stripp" branding).
  Examples: `lsNode`, `lsVector3`, `lsMatrix4x4`, `lsRender`, `lsPass_FBO`,
  `lsBulletWorld`, `lsAssetManager`, `lsFirstPersonRig`.
* **Files mirror type names**, lowercase with no separators:
  `lsNode` → `lsNode.h` / `lsNode.cpp`, `lsFirstPersonRig` →
  `lsFirstPersonRig.h` / `lsFirstPersonRig.cpp`.
  Multiple-word class names stay in PascalCase but are joined into one word
  in the file name (see `lsAsset_BulletShape_Box` → file
  `lsAsset_BulletShape_Box.{h,cpp}`).
* **Public member variables** start with `m_` (e.g. `m_active`, `m_delete`,
  `m_children`, `m_vortxge`). Public data members that pre-date the
  convention (e.g. `position`, `rotation`, `scale`, `ModelMatrix` on
  `lsNode`) are kept as-is — do **not** rename them in unrelated patches.
* **Accessors** use **camelCase** getters/setters: `getName/setName`,
  `getNode_type/setNode_type`, `getActive`, `getChildren`, etc. Many are
  inlined in the header; non-trivial logic lives in the `.cpp`.
* **Functions**: PascalCase methods that perform actions
  (`Add_Node`, `Del_Node`, `Find_Node`, `Set_Pos`, `Set_Rot`, `Set_Scale`,
  `InitDrawlist`, `MarkForDeletion`, `CleanupDeletedNodes`), camelCase
  convenience accessors (`getName`, `setParent`). Either is fine for new
  APIs but be consistent within the class.
* **Node type constants** are upper-snake-case with the `LSNT_` prefix and
  are defined as `const int` in `vortxGE/src/scene/lsNode.h` (e.g.
  `LSNT_NODE`, `LSNT_OBJECT`, `LSNT_CAMERA`, `LSNT_LIGHT`, `LSNT_SCENE`,
  `LSNT_FIRSTPERSONRIG`, `LSNT_ANIMNODE`, `LSNT_PARTICLE_EMITTER`).
* **Shader kind identifiers** live in `vortxGE/src/shader/lsShaderKinds.h`.
* **Debug switches** are `VORTX_DEBUG_*` and live in `vortxGE/src/vortx_debug.h`.

### 5.2 File / header conventions

* Every header uses **`#pragma once`**.
* Every header and `.cpp` starts with a **Doxygen comment block**
  (`@file`, `@brief`, `@class`, `@section`, `@see`, …). Match the style
  used in `vortxGE.h`, `lsNode.h`, and the rest of the engine — including
  `@pre`, `@post`, `@note`, `@param`, `@return`.
* Most headers include `using namespace std;` after the forward
  declarations. Follow that pattern in new headers for consistency with
  the surrounding code.
* Tabs are used for indentation throughout `vortxGE/src/`.
* Member functions that are short, inlineable, and have no side effects
  may be defined inside the class body; everything else goes to the `.cpp`.

### 5.3 Ownership and lifetime

* The engine mixes raw owning pointers (the manager fields on
  `vortxGE`) and `std::unique_ptr` (the `m_children` vector on
  `lsNode`). New code should prefer `std::unique_ptr` for owned
  resources and raw non-owning pointers for borrowed references.
* `lsNode` exposes both `const vector<unique_ptr<lsNode>>& getChildren() const`
  and a non-const overload — follow this pattern when adding container
  accessors that need both views.
* Lifecycle is "mark then sweep": nodes set `m_delete = true` via
  `MarkForDeletion()`, and `CleanupDeletedNodes()` is called once per frame
  to actually erase them from the children vector.

### 5.4 Debugging

* All non-trivial subsystems are gated by `#define VORTX_DEBUG_*`
  switches in `vortxGE/src/vortx_debug.h` (e.g. `VORTX_DEBUG_RENDER`,
  `VORTX_DEBUG_PHYSICS`, `VORTX_DEBUG_NODES`, `VORTX_DEBUG_SHADERS`,
  `VORTX_DEBUG_LOADSCENE`). Use the same `#ifdef` / `#endif` pattern for
  new trace points so they cost nothing in release builds.
* Status / info prints go to `std::cout`; errors / warnings to `std::cerr`
  (see `glfwApp.cpp`).

### 5.5 Engine lifecycle (host applications)

Every front-end follows the same three-phase pattern, documented in
`vortxGE.h` and `docs/vortxGE.md`:

1. Create a `vortxGE*` (`new vortxGE()`).
2. **Create a valid OpenGL context first** (via GLFW3, Gtk::GLArea, etc.)
   *then* call `engine->Init()`.
3. Each frame: `engine->Update(delta); engine->Render();`
4. `engine->Cleanup(); delete engine;`

Subsystem access is through getters on the engine instance:
`getRender()`, `getShaderManager()`, `getAssetManager()`,
`getControlManager()`, `getOpenAL()`.

### 5.6 Demos

* `demos/shared_code/demoscene.{h,cpp}` is the **single** source of truth
  for the demo scene used by both front-ends. When changing the demo
  scene, edit it there only — both demos link it.
* The GLFW3 demo lives in `glfwApp.{h,cpp}` (`glfwApp.cpp` declares a
  global `glfwApp glfw_app;` used by the static GLFW callbacks — keep
  that single-instance pattern).
* The GTKmm demo lives in `glwindow.{h,cpp}` plus a tiny `main.cpp`;
  `glviewwindow` inherits from `Gtk::Window`.

### 5.7 Asset / scene workflow

* Run `blender_scripts/export_scene.py` as a Blender addon, then
  **File → Export → vortxGE Scene (.vgs)**. This writes a `.vgs` hierarchy
  file and a `meshes/` directory of per-mesh `.glb` files (see
  `docs/blender_export.md`).
* The engine loads scenes via `lsSceneManager` from those `.vgs` files
  (see `docs/scene/SceneLoader.md`).
* Demo data lives in `demos/data/` and is symlinked into each demo's
  build directory by the `symlink_glfw3` / `symlink_gtkmm` custom
  targets defined in the root `CMakeLists.txt`. Update the source data
  there, not in `build/`.

### 5.8 Commit / hygiene guidelines

* The `.gitignore` excludes `build/`, `secrets/`, `blender/`, and `game/`.
  Do not commit anything under those paths.
* The `.daedalus/` directory at the repo root is owned by the agent
  runtime — leave it alone.
* Keep Doxygen comments accurate. If you change a public API
  signature, update both the comment and any callers in the demos and
  in `demos/shared_code/`.
* Prefer additive changes: new files under `vortxGE/src/<module>/` are
  picked up automatically thanks to `file(GLOB_RECURSE …)`. If you add
  a new *subproject* with its own executable or library, mirror the
  structure used by `tests/` or `demos/vortxGEdemo_glfw3/` (own
  `CMakeLists.txt`, `add_subdirectory()` from the root CMake file).

### 5.9 Renderer abstraction (do not call gl* directly)

**All non-renderer code routes through `lsRender`** (see
`vortxGE/src/render/`). The two rules:

1. **Do not call `gl*()` functions anywhere except inside
   `vortxGE/src/render/lsOpenGL.cpp`.** Use `getRender()->Xxx(...)` —
   the `Xxx` is the engine-level wrapper name (capitalised, no `gl`
   prefix; see §5.10 below). For example:

   ```cpp
   getRender()->TexImage2D( LS_TEXTURE_2D, 0, LS_RGBA,
                            width, height, 0, LS_RGBA,
                            LS_UNSIGNED_BYTE, NULL );
   ```

2. **Do not reference `GL_*` identifiers or `<GL/glew.h>` outside the
   `render/` directory.** Use the engine-level `LS_*` constants defined
   in `lsRenderConstants.h` (transitively included via `lsRender.h`),
   and the engine-level handle types in `lsGLTypes.h`:

   | Engine type | Replaces | |
   |---|---|---|
   | `lsTextureID` | `GLuint` (texture handle) | |
   | `lsFBOID` | `GLuint` (FBO handle) | |
   | `lsShaderID` | `GLuint` (shader object handle) | |
   | `lsProgramID` | `GLuint` (program handle) | |
   | `lsBufferID` | `GLuint` (VBO handle) | |
   | `lsVAOID` | `GLuint` (VAO handle) | |
   | `lsQueryID` | `GLuint` (occlusion-query handle) | |
   | `lsUniformID` | `GLint` (uniform location) | |
   | `lsEnum` | `GLenum` | |
   | `lsInt` | `GLint` | |
   | `lsUInt` | `GLuint` (non-handle counts) | |
   | `lsSizei` | `GLsizei` | |
   | `lsSizeiptr` | `GLsizeiptr` | |
   | `lsBitfield` | `GLbitfield` | |
   | `lsFloat` | `GLfloat` | |
   | `lsUByte` | `GLubyte` | |
   | `lsChar` | `GLchar` | |
   | `lsBoolean` | `GLboolean` | |

   Members of subsystems (`lsTexture::texture`, `lsFBO::m_fbo`,
   `lsShader::shader`, `lsBatch_Mesh::vao_mesh`, …) use these engine
   types; never store a raw `GLuint` in a non-renderer class.

3. **Adding a new GL call?** Add it in three places:
   * `lsRender.h` — pure virtual method declaration (e.g.
     `virtual void MyCall( lsEnum target, lsSizei n, lsInt* values ) = 0;`).
   * `lsOpenGL.h` — `void MyCall(...) override;`.
   * `lsOpenGL.cpp` — `void lsOpenGL::MyCall(...) { glMyCall(...); }`.

4. **Adding a new GL constant?** If it’s used outside the renderer,
   add it to `lsRenderConstants.h` as `#define LS_X GL_X`. Constants
   used only inside `lsOpenGL.cpp` don’t need to be re-exported.

5. **Adding a new backend** (e.g. `lsVulkan`): implement `lsRender` in
   `render/lsVulkan.{h,cpp}`. Rewrite `lsRenderConstants.h` so `LS_X`
   maps to the new backend’s constants. **No other source file
   changes.**

### 5.10 Wrapper-method naming convention

The wrapper methods in `lsRender`/`lsOpenGL` use the engine-level
**PascalCase** form of the GL name, with the `gl` prefix dropped.
Some examples:

| Wrapper (engine) | Wraps (OpenGL) |
|------------------|----------------|
| `BindTexture(target, id)` | `glBindTexture` |
| `TexImage2D(target, level, ifmt, w, h, b, fmt, type, data)` | `glTexImage2D` |
| `BindFramebuffer(target, fbo)` | `glBindFramebuffer` |
| `GenBuffers(n, buffers)` | `glGenBuffers` |
| `UseProgram(program)` | `glUseProgram` |
| `UniformMatrix4fv(loc, count, transpose, value)` | `glUniformMatrix4fv` |
| `GetUniformLocation(program, name)` | `glGetUniformLocation` |
| `ActiveTexture(unit)` | `glActiveTexture` |
| `Clear(mask)` | `glClear` |
| `Enable(cap)` / `Disable(cap)` | `glEnable` / `glDisable` |
| `BeginQuery(target, id)` / `EndQuery(target)` | `glBeginQuery` / `glEndQuery` |
| `CheckGLErrors(tag)` | (composite of `glGetError` + logging) |
| `GetInfo()` | (composite of `glGetString(GL_VERSION/VENDOR/RENDERER)`) |

The full list lives in `lsRender.h` (declarations) and
`lsOpenGL.cpp` (one-liner overrides).

---

## 6. Common Tasks for an AI Agent

Quick recipes you'll likely need:

* **"Add a new node type"** → add a `LSNT_*` constant in
  `vortxGE/src/scene/lsNode.h`, subclass `lsNode` (or `lsObject` if it is
  renderable), put it under `vortxGE/src/scene/`, document with Doxygen,
  no CMake change needed.
* **"Add a new shader pass"** → subclass `lsPass` (see
  `vortxGE/src/passes/lsPass_FBO.cpp` as a template), register a new
  shader kind in `vortxGE/src/shader/lsShaderKinds.h`, and add the pass
  via `lsPassManager` from the host demo.
* **"Add a new asset loader"** → subclass `lsAsset`, add a manager
  entry point in `lsAssetManager`, and put the asset under
  `vortxGE/src/assets/`.
* **"Add a new light type"** → subclass `lsLight` (see `lsLight_Point`
  and `lsLight_Spot`), then register it with `lsLightManager` so the
  Blender exporter can find it (`blender_scripts/export_scene.py` may
  also need updates).
* **"Tweak a demo scene"** → edit `demos/shared_code/demoscene.cpp`
  (and `.h`). Both demos recompile automatically.
* **"Add a runtime option"** → `lsProperty` / `lsProperty_Float` /
  `lsProperty_Vector3` exist exactly for this; bind through
  `lsControlManager` or `lsAssetManager`.
* **"Add a new renderer call"** → add a pure virtual in `lsRender.h`,
  override it in `lsOpenGL.h`, implement in `lsOpenGL.cpp` as a
  one-liner around `gl*()`. If the GL constant involved is used
  outside the renderer, also add it to `lsRenderConstants.h`. See
  §5.9 for the three-step checklist.
* **"Add a new GL_* constant used outside the renderer"** → add
  `#define LS_X GL_X` to `lsRenderConstants.h`. Never use `GL_*` in
  non-renderer code.
* **"Add a new render backend (Vulkan, Metal, …)"** → see §5.9 step 5.
  In short: create `render/lsNewBackend.{h,cpp}` implementing
  `lsRender`, rewrite `lsRenderConstants.h` so `LS_X` maps to the new
  constants. Zero changes outside `render/`.

---

## 7. Where to Read More

**Start at [docs/index.md](docs/index.md)** — the catalog of all
project documentation (updated as we go).

| Topic | File |
|-------|------|
| Engine lifecycle, subsystems, init order | `docs/vortxGE.md` |
| Scene graph node types & coordinate system | `docs/scene/SceneGraph.md` |
| Loading `.vgs` scenes | `docs/scene/SceneLoader.md` |
| First-person controller | `docs/scene/FirstPersonRig.md` |
| Asset pipeline & datablocks | `docs/assets/AssetManagement.md`, `docs/datablocks/` |
| Render passes | `docs/passes/RenderingPipeline.md` |
| Physics & Bullet integration | `docs/physics/` |
| Animation system | `docs/animation/` |
| Audio (OpenAL) | `docs/audio/` |
| Event bus | `docs/events/` |
| Math utilities | `docs/math/`, `docs/utils/Utilities.md` |
| Renderer abstraction (`lsRender`, `lsOpenGL`, `LS_*` constants) | §5.9 of `AGENTS.md`, `vortxGE/src/render/lsRender.h`, `docs/plans/renderer_abstraction.md` |
| Shaders | `docs/shader/` |
| Controls / input | `docs/controls/` |
| Blender export | `docs/blender_export.md` |
| glfw desktop scaling caveat | `docs/glfw_desktop_scale_factor.md` |
| OBJ → vbm tool | `tools/obj2vbm/Readme.md` |
| Demo walkthroughs | `docs/demos/` |
| User-facing overview | `Readme.md` |

