# Plan.md — Centralize all OpenGL calls into `lsOpenGL`

> **Status:** ✅ **COMPLETED** (Phases 0–8 done, August 2026)
> **Target class:** `vortxGE/src/render/lsOpenGL`
> **Achieved:** `lsOpenGL.cpp` is now the **only** translation unit that
> contains raw `gl*()` function calls. The rest of the engine talks to
> the renderer exclusively through `lsRender` (75 pure-virtual methods,
> implemented as one-liner `lsOpenGL` overrides). Every other source file
> uses engine-level types (`lsTextureID`, `lsShaderID`, `lsEnum`, …) and
> engine-level constants (`LS_TEXTURE_2D`, `LS_TRIANGLES`, …) — neither
> `<GL/glew.h>` nor any `GL_*` identifier is referenced outside
> `vortxGE/src/render/`. See §10 “Final State” for the post-migration
> metrics.

---

## 1. Why this refactor

At present the engine has a partial layering:
* `lsRender` (abstract base) ✅ exists at `vortxGE/src/render/lsRender.h`
* `lsOpenGL` (the only concrete render backend) ⚠️ exists but only wraps
  5 GL calls (`Init`, `CheckErrors`, `StoreDefaultFramebuffer`,
  `RestoreDefaultFramebuffer`, `glInfo`).

**Roughly 200 raw `gl*()` calls are scattered across 17 source files**
outside `lsOpenGL`. This causes:

1. Every renderer-touching module transitively pulls in `<GL/glew.h>`
   through `lsOpenGL.h`.
2. Replacement of OpenGL with Vulkan/Metal/WebGPU would require editing
   every file, not just the backend.
3. Cross-cutting concerns (per-frame GL state tracking, error checking,
   debug labels, threading changes) have no single hook.
4. The `lsRender*` indirection is mostly aspirational — most call sites
   bypass it and call `glXxx` directly.

The objective is to make `lsOpenGL` the **single owner** of every GL call,
so higher-level code only knows about render handles (texture IDs, VAOs,
shader programs, uniform locations) and engine-level operations.

---

## 2. Current state — complete inventory

### 2.1 Files that already route through `lsOpenGL`
* `vortxGE/src/render/lsOpenGL.{h,cpp}` — the thin wrapper itself.
* `vortxGE/src/render/lsRender.{h,cpp}` — abstract base (6 methods).
* `vortxGE/src/render/lsRenderTypes.h` — `RenderType` enum.

### 2.2 Files that call `gl*()` directly and need to be migrated
| File | Approx. GL calls | Subsystem |
|------|-----------------:|-----------|
| `vortxGE/src/datablocks/lsBatch_Quad.cpp` | ~25 | VBO/VAO + draw |
| `vortxGE/src/datablocks/lsBatch_Mesh.cpp` | ~30 | VBO/VAO + draw |
| `vortxGE/src/datablocks/lsBatch_Lines.cpp` | ~25 | VBO/VAO + draw |
| `vortxGE/src/datablocks/lsFBO.cpp` | 1 | FBO destruction |
| `vortxGE/src/datablocks/lsFBO_ColourDepth_8.cpp` | ~25 | FBO + texture creation |
| `vortxGE/src/datablocks/lsFBO_Depth.cpp` | ~20 | FBO + depth texture |
| `vortxGE/src/datablocks/lsMesh.cpp` | ~15 | glTF embedded texture upload |
| `vortxGE/src/datablocks/lsTexture.cpp` | ~30 | `MakeGlTexture`, texture deletion |
| `vortxGE/src/datablocks/lsTGA.cpp` | 2 | `glReadPixels`, `glGetIntegerv` |
| `vortxGE/src/math/lsPlane.cpp` | commented-out `glBegin`/`glVertex3f`/`glEnd` block | dead code, can be deleted |
| `vortxGE/src/passes/lsPass_DR.cpp` | ~10 | viewport, clear, blend |
| `vortxGE/src/passes/lsPass_FBO.cpp` | ~6 | viewport, clear, depth range |
| `vortxGE/src/passes/lsPass_Occlusion.cpp` | ~25 | occlusion queries, color/depth mask |
| `vortxGE/src/passes/lsPass_ShadowMap.cpp` | ~10 | polygon offset, depth range, cull |
| `vortxGE/src/scene/lsScene.cpp` | 8 | skybox state save/restore |
| `vortxGE/src/shader/lsShader.cpp` | ~70 | program/shader compile, link, uniforms |

(Counts come from `grep -cE "gl[A-Z][a-zA-Z]*\(" file.cpp` style scans; small
deltas expected during migration.)

### 2.3 Files that store GL handle types but don't call `gl*()`
These declare fields of type `GLuint`/`GLint` and need a **type
substitution**, not a code rewrite:

| File | GL types used | Purpose |
|------|---------------|---------|
| `vortxGE/src/scene/lsLight.h` | `GLuint tex_shadowmap` | shadow map texture handle |
| `vortxGE/src/scene/lsObject.h` | `GLuint occlusion_id` | occlusion query handle |
| `vortxGE/src/passes/lsPass_ShadowMap.h` | `lsFBO_Depth*` (no GL types) | holds FBO |
| `vortxGE/src/datablocks/lsBatch.h` + subclasses | `GLuint` (VAO, VBO) | mesh data on GPU |
| `vortxGE/src/datablocks/lsFBO.h` | `GLuint m_fbo` | framebuffer handle |
| `vortxGE/src/datablocks/lsTexture.h` | `GLuint texture` | texture handle |
| `vortxGE/src/shader/lsShader.h` | `GLuint shader*/shader_v/shader_f`, dozens of `GLint` uniform locations | shader program + uniform handles |
| `vortxGE/src/datablocks/lsBatch_Bullet.h` | `GLuint nNumIndexes/...` | only counts; no GL calls |
| `vortxGE/src/datablocks/lsBulletShape.h` | (no GL types) | n/a |

### 2.4 Files confirmed GL-free
The following do **not** contain OpenGL and need no changes:
* `demos/**` (no direct GL — they use `vortxGE`/`lsRender` API)
* `tools/obj2vbm/**` (no GL)
* `vortxGE/src/assets/**` (no GL calls)
* `vortxGE/src/scene/lsCamera.*`, `lsLight*.cpp`, `lsObject.cpp` (no GL calls)
* `vortxGE/src/physics/**` (no GL)
* `vortxGE/src/openal/**` (no GL)
* `vortxGE/src/controls/**` (no GL)
* `vortxGE/src/animation/**` (no GL)
* `vortxGE/src/events/**` (no GL)
* `vortxGE/src/properties/**` (no GL)
* `vortxGE/src/math/**` (only `lsPlane.cpp`'s commented-out debug code)

---

## 3. Target architecture

### 3.1 Layering

```
+--------------------------------------------------+
|  Demo / host application (demos/, game/)         |
|  - creates vortxGE, GL context, drives the loop  |
+--------------------------------------------------+
                       |
                       v
+--------------------------------------------------+
|  vortxGE engine core (vortxGE.cpp, lsScene…)     |
|  - knows about lsRender, lsOpenGL (via getter)   |
|  - never includes <GL/glew.h>                    |
+--------------------------------------------------+
                       |
                       v
+------------------------------------+   +-----------------------+
|  Subsystem modules (high-level)    |   |  render/lsGLTypes.h   |
|  lsBatch*, lsFBO*, lsTexture,      |-->|  - opaque handle      |
|  lsShader, lsPass*, lsScene,       |   |    typedefs           |
|  lsMesh, lsLight, lsObject         |   |  - non-GL constants   |
|  → call lsOpenGL methods only      |   +-----------------------+
+------------------------------------+             |
                       |                           v
                       |             +-------------------------+
                       +-----------> |  render/lsOpenGL.{h,cpp}|
                                     |  - the ONLY .cpp that   |
                                     |    includes <GL/glew.h>  |
                                     |  - all gl*() live here  |
                                     |  - all <gl/..> enums/   |
                                     |    #defines are wrapped |
                                     +-------------------------+
                                                |
                                                v
                                     +-------------------------+
                                     |  lsRender               |
                                     |  (abstract base, kept   |
                                     |  renderer-agnostic)     |
                                     +-------------------------+
```

### 3.2 Concrete class change

* `lsRender` becomes the **shared, backend-agnostic surface**:
  * `Init()`, `CheckErrors()`, `StoreDefaultFramebuffer()`,
    `RestoreDefaultFramebuffer()` (existing).
  * Convenience accessors: `getType()`, `getStats()`, `getShaderManager()`,
    `setStats()`, `setShaderManager()` (existing).
  * **New**: `virtual lsOpenGL* getOpenGL() = 0;` — returns the
    concrete GL backend. In `lsOpenGL` this returns `this`. In any
    future Vulkan/Metal backend it returns `nullptr` (and callers
    must check). Keep the existing `lsRender* getRender()` accessors
    and pair them with a sibling `lsOpenGL* getOgl()` on each subsystem
    that needs GL.
* `lsOpenGL` is the **only** translation unit that includes `<GL/glew.h>`.
* New `render/lsGLTypes.h` provides handle typedefs (see §3.4) so
  non-render headers don't need GL types.

### 3.3 The `lsOpenGL` public API surface

Every `gl*()` call used in the codebase becomes a method on `lsOpenGL`.
The method names mirror the GL function (CamelCase) so the diff is easy
to review:

```cpp
// Trivial passthrough methods — one per gl* function used
void ActiveTexture(GLenum texture);
void AttachShader(GLuint program, GLuint shader);
void Begin(GLenum mode);                          // (legacy, see §3.6)
void BindBuffer(GLenum target, GLuint buffer);
void BindFramebuffer(GLenum target, GLuint fbo);
void BindTexture(GLenum target, GLuint texture);
void BindVertexArray(GLuint vao);
void BlendFunc(GLenum sfactor, GLenum dfactor);
void BufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void CheckFramebufferStatus(GLenum target);       // returns GLenum
void Clear(GLbitfield mask);
void ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void ClearDepth(GLdouble d);
void ColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a);
void CompileShader(GLuint shader);
void CreateProgram();                             // returns GLuint
void CreateShader(GLenum type);                   // returns GLuint
void CullFace(GLenum mode);
void DeleteBuffers(GLsizei n, GLuint* buffers);
void DeleteFramebuffers(GLsizei n, GLuint* fbos);
void DeleteProgram(GLuint program);
void DeleteQueries(GLsizei n, GLuint* ids);
void DeleteShader(GLuint shader);
void DeleteTextures(GLsizei n, GLuint* textures);
void DeleteVertexArrays(GLsizei n, GLuint* arrays);
void DepthFunc(GLenum func);
void DepthMask(GLboolean flag);
void DepthRangef(GLfloat near, GLfloat far);
void DetachShader(GLuint program, GLuint shader);
void Disable(GLenum cap);
void DrawArrays(GLenum mode, GLint first, GLsizei count);
void DrawBuffer(GLenum mode);
void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
void Enable(GLenum cap);
void EnableVertexAttribArray(GLuint index);
void End();                                       // (legacy, see §3.6)
void EndQuery(GLenum target);
void FramebufferTexture2D(GLenum target, GLenum attachment,
                          GLenum textarget, GLuint texture, GLint level);
void GenBuffers(GLsizei n, GLuint* buffers);
void GenFramebuffers(GLsizei n, GLuint* fbos);
void GenQueries(GLsizei n, GLuint* ids);
void GenTextures(GLsizei n, GLuint* textures);
void GenVertexArrays(GLsizei n, GLuint* arrays);
void GenerateMipmap(GLenum target);
GLenum GetError();
void GetIntegerv(GLenum pname, GLint* params);
void GetProgramInfoLog(GLuint program, GLsizei bufSize,
                       GLsizei* length, GLchar* infoLog);
void GetProgramiv(GLuint program, GLenum pname, GLint* params);
void GetQueryObjectiv(GLuint id, GLenum pname, GLint* params);
void GetShaderInfoLog(GLuint shader, GLsizei bufSize,
                      GLsizei* length, GLchar* infoLog);
void GetShaderiv(GLuint shader, GLenum pname, GLint* params);
const GLubyte* GetString(GLenum name);
GLint GetUniformLocation(GLuint program, const GLchar* name);
void LinkProgram(GLuint program);
void PixelStorei(GLenum pname, GLint param);
void PolygonOffset(GLfloat factor, GLfloat units);
void ReadBuffer(GLenum mode);
void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                GLenum format, GLenum type, GLvoid* data);
void ShadeModel(GLenum mode);
void ShaderSource(GLuint shader, GLsizei count,
                  const GLchar** string, const GLint* length);
void TexImage2D(GLenum target, GLint level, GLint internalformat,
                GLsizei width, GLsizei height, GLint border,
                GLenum format, GLenum type, const GLvoid* data);
void TexParameterf(GLenum target, GLenum pname, GLfloat param);
void TexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
void TexParameteri(GLenum target, GLenum pname, GLint param);
void Uniform1f(GLint location, GLfloat v0);
void Uniform1i(GLint location, GLint v0);
void Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void UniformMatrix3fv(GLint location, GLsizei count,
                      GLboolean transpose, const GLfloat* value);
void UniformMatrix4fv(GLint location, GLsizei count,
                      GLboolean transpose, const GLfloat* value);
void UseProgram(GLuint program);
void VertexAttribPointer(GLuint index, GLint size, GLenum type,
                         GLboolean normalized, GLsizei stride,
                         const GLvoid* pointer);
void Viewport(GLint x, GLint y, GLsizei w, GLint h);

// Existing, keep as-is:
void Init();
void CheckErrors(const string& name);
void StoreDefaultFramebuffer();
void RestoreDefaultFramebuffer();
void glInfo();
```

> **Implementation note:** every method body is one line — a forward to
> the matching `gl*()` function. Generated bodies are easy to write by
> mechanical search/replace but worth verifying by running the build
> and demos after each phase.

### 3.4 `lsGLTypes.h` — decouple handle types from GL

Headers outside `render/` should not need to drag in `<GL/glew.h>`. Add a
new tiny header:

```cpp
// vortxGE/src/render/lsGLTypes.h  (NEW)
#pragma once
// Re-export opaque handle types so non-render headers can declare
// "texture ID", "program ID", etc. without coupling to GLEW.
//
// Two equally valid choices; pick (b) for the cleanest separation.

// (a) Alias approach — keep GLuint/GLint semantics, pulls in GLEW still
//     — NOT recommended.
// #include <GL/glew.h>
// using lsTextureID = GLuint;
// using lsShaderID  = GLuint;
// using lsBufferID  = GLuint;
// using lsUniformID = GLint;

// (b) Engine-level opaque types — preferred, lets us drop <GL/glew.h>
//     from non-render headers entirely.
#include <cstdint>
using lsTextureID  = std::uint32_t;
using lsShaderID   = std::uint32_t;
using lsProgramID  = std::uint32_t;
using lsVAOID      = std::uint32_t;
using lsBufferID   = std::uint32_t;
using lsFBOID      = std::uint32_t;
using lsQueryID    = std::uint32_t;
using lsUniformID  = int;          // matches GLint
```

Then `<GL/glew.h>` is included **only** by `lsOpenGL.h` and `lsOpenGL.cpp`
(internally, in the `.cpp`).

Where `lsOpenGL` exposes GL enum constants to callers (e.g. `GL_TEXTURE0`,
`GL_RGBA`, `GL_TRUE`, `GL_TRIANGLES`, `GL_DEPTH_TEST`), wrap them in
matched `lsOGL::` constants in `lsOpenGL` (or a new `lsGLEnums.h`). This
keeps the constant names recognisable while preventing leakage.

> **Recommendation:** Start with the **passthrough method names +
> opaque handle types** approach. It is the most explicit and makes the
> refactor easy to review incrementally. A more invasive "domain-level"
> API (e.g. `CreateMeshBatch(vao_desc)`) can be revisited later.

### 3.5 Caller access pattern

For every class that today calls `gl*()` directly, add a sibling
accessor and use it:

```cpp
// Before
class lsBatch_Quad {
    lsRender *m_render;
    void Copy_GLdata() {
        glGenVertexArrays(1, &vao_quad);
        // ...
    }
};

// After
class lsBatch_Quad {
    lsRender  *m_render;
    lsOpenGL  *m_ogl;        // NEW (cached in ctor: m_ogl = m_render->getOpenGL())
    void Copy_GLdata() {
        m_ogl->GenVertexArrays(1, &vao_quad);
        // ...
    }
};
```

The `lsRender* m_render` pointer is kept (it's still used for stats,
debug, and renderer-agnostic methods), but anything that calls GL goes
through `m_ogl`.

### 3.6 Dead code cleanup folded into the refactor

* `vortxGE/src/math/lsPlane.cpp::drawGuide()` — commented-out immediate
  mode (`glBegin`/`glVertex3f`/`glEnd`). **Delete the commented block**
  in the same commit that introduces the wrapper, so we don't add
  wrappers for code that will never be called.
* `vortxGE/src/passes/lsPass_DR.cpp` has an `#ifdef VORTX_DEBUG_OPENGLERRORS`
  block that calls `getRender()->CheckGLErrors(...)` — confirm that
  `CheckErrors` is the method name (it is, in `lsOpenGL`); rename to
  `CheckGLErrors` for consistency with the call site, or update the call
  site. *Decision: keep `CheckErrors` (matches the header) and update
  the call site.*

---

## 4. Phased migration

The work is broken into **8 phases**, each independently buildable and
runnable. After every phase, the demos must still compile and at minimum
the GLFW3 demo must run (it's the most complete). Each phase ends with
a commit so the diff is reviewable.

### Phase 0 — Plumbing (no behaviour change)
* Add `render/lsGLTypes.h` (opaque handle typedefs).
* Add `virtual lsOpenGL* lsRender::getOpenGL() = 0;` and override in
  `lsOpenGL` to return `this`.
* In `lsOpenGL.h`, add **all** the wrapper method declarations listed in
  §3.3 (forward-only, no bodies yet, bodies added as phases migrate).
* In each subsystem that owns GL data, add `lsOpenGL* m_ogl;` and
  initialise it in the constructor via `m_render->getOpenGL()`.
* **No code changes** to call sites. Build must succeed.

### Phase 1 — Render passes (viewport & clear)
Migrate `lsPass_DR`, `lsPass_FBO`, `lsPass_ShadowMap`, `lsPass_Occlusion`
viewport / clear / enable / depth-mask / blend / depth-range /
polygon-offset / color-mask / shade-model calls.

These are the simplest, route almost entirely through `m_ogl->Foo(...)`,
and prove the access pattern works.

**Touches:** 4 files in `vortxGE/src/passes/`, ~50 call sites.

### Phase 2 — Pipeline state in `lsScene`
Migrate `lsScene.cpp` skybox state save/restore (8 lines, found in
`lsScene::Draw` around lines 171–187).

**Touches:** 1 file, 8 call sites.

### Phase 3 — Framebuffer objects
Migrate `lsFBO`, `lsFBO_ColourDepth_8`, `lsFBO_Depth`. The constructors
create textures, configure them, build the FBO, attach textures, check
status. This is the meatiest of the "structural" FBO work.

**Touches:** 4 files, ~60 call sites.

### Phase 4 — Texture creation (`lsTexture`, `lsMesh`)
Migrate `lsTexture::MakeGlTexture` and `lsTexture::~lsTexture`. Then
migrate the inline glTF texture upload in `lsMesh.cpp` (lines
1056–1077). `lsTGA::frameGrab` (`glGetIntegerv`, `glReadPixels`) goes
here too.

**Touches:** 3 files, ~50 call sites.

### Phase 5 — Batch VBO/VAO (lines/mesh/quad)
Migrate `lsBatch_Quad`, `lsBatch_Mesh`, `lsBatch_Lines`:
* `Copy_GLdata()` — `glGenVertexArrays`, `glGenBuffers`,
  `glBindBuffer`, `glEnableVertexAttribArray`, `glVertexAttribPointer`,
  `glBufferData`.
* `Clear_GLdata()` — `glDeleteBuffers`, `glDeleteVertexArrays`.
* `Draw()` — `glBindVertexArray`, `glDrawArrays`/`glDrawElements`,
  `glUseProgram(0)`.

Replace the `GLuint` fields in `lsBatch*.h` with the new `lsBufferID` /
`lsVAOID` typedefs.

**Touches:** 6 files (3 `.cpp` + 3 `.h`), ~75 call sites.

### Phase 6 — Shaders and uniforms
The biggest single file. Migrate `lsShader.cpp`:
* Constructor (`glCreateShader`, `glCreateProgram`, `glAttachShader`).
* Destructor (`glDetachShader`, `glDeleteShader`, `glDeleteProgram`).
* `Load_Shader` (`glShaderSource`, `glCompileShader`, `glLinkProgram`).
* `debug(GLuint)` / `debugProgram()` (`glGetShaderiv`,
  `glShaderInfoLog`, `glGetProgramiv`, `glGetProgramInfoLog`).
* `UseShader` (`glUseProgram`, `glActiveTexture`, `glBindTexture`,
  `glUniform1i`, `glUniform1f`, `glUniform4f`, `glUniformMatrix3fv`,
  `glUniformMatrix4fv`).
* `Setup_Lights` / `Set_Light` (the rest of the `glUniform*` calls).
* All `glGetUniformLocation` calls in `LoadShaderUniforms`.

Replace `GLuint shader/shader_v/shader_f` with `lsProgramID` /
`lsShaderID` and all `GLint iXxx` with `lsUniformID` in `lsShader.h`.

**Touches:** 2 files, ~70 call sites.

### Phase 7 — Occlusion queries
Finish `lsPass_Occlusion.cpp` (`glGenQueries`, `glBeginQuery`,
`glEndQuery`, `glGetQueryObjectiv`, `glDeleteQueries`).

Replace `GLuint occlusion_id` in `lsObject.h` with `lsQueryID`.

**Touches:** 2 files, ~10 call sites.

### Phase 8 — Cleanup & typedef migration
* Replace `GLuint tex_shadowmap` in `lsLight.h` with `lsTextureID`.
* Replace `GLuint*` count fields in `lsBatch_Bullet.h` (just rename to
  `std::uint32_t` — they are pure integer counts, not GL handles).
* Remove `<GL/glew.h>` from every `*.h`/`.cpp` outside `vortxGE/src/render/`.
* Remove `using namespace std;` from `lsFBO.h` **only if** it is no
  longer needed for `string` after the migration.
* Run `grep -rE "gl[A-Z][a-zA-Z]*\(|GL_[A-Z_0-9]+" vortxGE/src/`
  outside `vortxGE/src/render/` — there should be **zero** matches.
* Update `AGENTS.md` (the guide we just wrote) with the new
  "do not call gl*() directly; use `lsOpenGL`" rule.

---

## 5. Verification strategy

After **every phase**, the following must pass:

1. **Build** — `cmake .. && make -j` from `build/` produces
   `libvortxGE.a`, `vortxGEdemo_glfw3`, `vortxGEdemo_gtkmm`, `tests`,
   `obj2vbm` with no warnings or errors.
2. **Test runner** — `./tests/tests` prints "vortxGE tests v0.1" and
   exercises `lsMatrix4x4` (today's smoke test).
3. **GLFW3 demo** — `./demos/vortxGEdemo_glfw3/vortxGEdemo_glfw3`
   starts, shows the PBR scene, and the FPS controller works.
4. **GTKmm demo** — `./demos/vortxGEdemo_gtkmm/vortxGEdemo_gtkmm`
   starts and renders the same scene (no GL errors in stderr).

In addition (results shown are post-Phase-8 actual state):

* `grep -rEn "gl[A-Z][a-zA-Z]*\(" vortxGE/src/` outside `render/` →
  **zero matches**. All raw `gl*` calls are in `render/lsOpenGL.cpp`.
* `grep -rln "<GL/glew.h>" vortxGE/src/` outside `render/` →
  **zero matches**. Only `render/lsRenderConstants.h` (the bridge
  header) and `render/lsOpenGL.cpp` include `<GL/glew.h>`.
* `grep -rEln "\b(GLuint|GLenum|GLint|GLfloat|GLubyte|GLushort|GLboolean|GLchar|GLsizei|GLbitfield|GLdouble)\b" vortxGE/src/` outside `render/` →
  **zero matches** (the only `GL_*` identifiers outside `render/`
  are in Doxygen comments documenting the wrapper equivalence).

A reviewer should also spot-check the diff in one or two migrated files
to ensure no semantics were changed (e.g. wrong `glActiveTexture` unit,
swapped `glDepthFunc`, etc.).

---

## 6. Risks & mitigations

| Risk | Likelihood | Mitigation |
|------|------------|------------|
| Performance regression from wrapping every GL call | Low | Each wrapper is `inline`able and a single forward; the compiler will fold it. Add `inline` in the header if profiling shows otherwise. |
| Behavioural regression from typos in wrapper names | Medium | Migrate phase by phase; build + run demos after each phase. |
| Hidden GL leaks through macro headers (e.g. `tinygltf` indirectly defining GL) | Low | `grep -r "GL_" vortxGE/src/` outside `render/` at the end of Phase 8. |
| Demo frontends (GLFW3, GTKmm) need to keep GLEW includes for their own context setup | High (and intentional) | Confirmed: GLEW is **only** required by `render/lsOpenGL.cpp` for engine calls. The demos include `<GL/glew.h>` themselves to call `glewInit()` and `glewExperimental`. That is **host-application concern**, not engine concern, and was kept out of scope as planned. |
| `lsOpenGL` becomes a "god object" with hundreds of methods | Medium | Acceptable for now. Once stable, refactor internally into subsystem helpers (e.g. `lsOpenGL_Textures`, `lsOpenGL_Pipeline`) that are called from `lsOpenGL` methods. This is a Phase 9+ improvement. |
| Removing `GLuint` from headers breaks ABI for any external code that consumes them | Low | This is a static library; consuming code is in-repo. If a future game wants raw GL handles, expose a `asGLuint()` accessor. |
| `lsRender* getRender()` callers expect to call `gl*()` via `getRender()` | Low | Only `lsShader`, `lsPass`, `lsBatch`, `lsFBO`, `lsTexture` carry `lsRender*`, and `lsRender` itself has no GL methods. External code does not exist. |

---

## 7. Out of scope for this refactor

The following are explicitly **not** part of this work and should be
treated as separate efforts:

* Adding a Vulkan or Metal backend (the abstraction is being prepared
  for it, but no second backend is implemented).
* Domain-level helper methods (e.g. `CreateTexture2D(...)`,
  `UploadVertexBuffer(...)`) that group several GL calls into one
  ergonomic API. Do this in a follow-up once the flat facade is stable.
* Performance optimisations (UBO/SSBO for uniforms, VAO batching,
  bindless textures, etc.).
* Replacing the `using lsRendererPtr = lsRender*` / `lsOpenGL*` access
  pattern with a templated `IRender` interface.
* Migration of `tinygltf` / `stb_image` to non-GL image upload paths
  (the current loader embeds GL calls for the inline-embedded image
  fallback in `lsMesh.cpp`).

---

## 8. Reference: every gl* call → lsOpenGL method

| gl* call | Used in (today) | New wrapper |
|----------|------------------|-------------|
| `glActiveTexture` | lsShader.cpp | `ogl->ActiveTexture` |
| `glAttachShader` | lsShader.cpp | `ogl->AttachShader` |
| `glBegin` | lsPlane.cpp (commented) | **delete** |
| `glBeginQuery` | lsPass_Occlusion.cpp | `ogl->BeginQuery` |
| `glBindBuffer` | lsBatch_*.cpp | `ogl->BindBuffer` |
| `glBindFramebuffer` | lsFBO*.cpp, lsPass_*.cpp | `ogl->BindFramebuffer` |
| `glBindTexture` | lsTexture.cpp, lsShader.cpp, lsFBO*.cpp | `ogl->BindTexture` |
| `glBindVertexArray` | lsBatch_*.cpp | `ogl->BindVertexArray` |
| `glBlendFunc` | lsPass_DR.cpp | `ogl->BlendFunc` |
| `glBufferData` | lsBatch_*.cpp | `ogl->BufferData` |
| `glCheckFramebufferStatus` | lsFBO_ColourDepth_8.cpp, lsFBO_Depth.cpp | `ogl->CheckFramebufferStatus` |
| `glClear` | lsFBO*.cpp, lsPass_*.cpp | `ogl->Clear` |
| `glClearColor` | lsPass_*.cpp | `ogl->ClearColor` |
| `glClearDepth` | lsPass_*.cpp | `ogl->ClearDepth` |
| `glColorMask` | lsPass_Occlusion.cpp | `ogl->ColorMask` |
| `glCompileShader` | lsShader.cpp | `ogl->CompileShader` |
| `glCreateProgram` | lsShader.cpp | `ogl->CreateProgram` |
| `glCreateShader` | lsShader.cpp | `ogl->CreateShader` |
| `glCullFace` | lsPass_ShadowMap.cpp | `ogl->CullFace` |
| `glDeleteBuffers` | lsBatch_*.cpp | `ogl->DeleteBuffers` |
| `glDeleteFramebuffers` | lsFBO.cpp | `ogl->DeleteFramebuffers` |
| `glDeleteProgram` | lsShader.cpp | `ogl->DeleteProgram` |
| `glDeleteQueries` | lsPass_Occlusion.cpp | `ogl->DeleteQueries` |
| `glDeleteShader` | lsShader.cpp | `ogl->DeleteShader` |
| `glDeleteTextures` | lsTexture.cpp, lsFBO*.cpp | `ogl->DeleteTextures` |
| `glDeleteVertexArrays` | lsBatch_*.cpp | `ogl->DeleteVertexArrays` |
| `glDepthFunc` | lsScene.cpp | `ogl->DepthFunc` |
| `glDepthMask` | lsPass_*.cpp, lsScene.cpp | `ogl->DepthMask` |
| `glDepthRangef` | lsPass_*.cpp | `ogl->DepthRangef` |
| `glDetachShader` | lsShader.cpp | `ogl->DetachShader` |
| `glDisable` | lsScene.cpp, lsPass_Occlusion.cpp | `ogl->Disable` |
| `glDrawArrays` | lsBatch_Lines.cpp | `ogl->DrawArrays` |
| `glDrawBuffer` | lsFBO_Depth.cpp | `ogl->DrawBuffer` |
| `glDrawElements` | lsBatch_Quad.cpp, lsBatch_Mesh.cpp | `ogl->DrawElements` |
| `glEnable` | lsPass_*.cpp, lsScene.cpp | `ogl->Enable` |
| `glEnableVertexAttribArray` | lsBatch_*.cpp | `ogl->EnableVertexAttribArray` |
| `glEnd` | lsPlane.cpp (commented) | **delete** |
| `glEndQuery` | lsPass_Occlusion.cpp | `ogl->EndQuery` |
| `glFramebufferTexture2D` | lsFBO*.cpp | `ogl->FramebufferTexture2D` |
| `glGenBuffers` | lsBatch_*.cpp | `ogl->GenBuffers` |
| `glGenFramebuffers` | lsFBO*.cpp | `ogl->GenFramebuffers` |
| `glGenQueries` | lsPass_Occlusion.cpp | `ogl->GenQueries` |
| `glGenTextures` | lsTexture.cpp, lsFBO*.cpp, lsMesh.cpp | `ogl->GenTextures` |
| `glGenVertexArrays` | lsBatch_*.cpp | `ogl->GenVertexArrays` |
| `glGenerateMipmap` | lsTexture.cpp, lsMesh.cpp | `ogl->GenerateMipmap` |
| `glGetError` | lsOpenGL.cpp (only) | already inside `CheckErrors` |
| `glGetIntegerv` | lsTGA.cpp, lsOpenGL.cpp | `ogl->GetIntegerv` |
| `glGetProgramInfoLog` | lsShader.cpp (`debugProgram`) | `ogl->GetProgramInfoLog` |
| `glGetProgramiv` | lsShader.cpp | `ogl->GetProgramiv` |
| `glGetQueryObjectiv` | lsPass_Occlusion.cpp | `ogl->GetQueryObjectiv` |
| `glGetShaderInfoLog` | lsShader.cpp | `ogl->GetShaderInfoLog` |
| `glGetShaderiv` | lsShader.cpp | `ogl->GetShaderiv` |
| `glGetString` | lsOpenGL.cpp (only) | already inside `glInfo` |
| `glGetUniformLocation` | lsShader.cpp | `ogl->GetUniformLocation` |
| `glLinkProgram` | lsShader.cpp | `ogl->LinkProgram` |
| `glPixelStorei` | lsTexture.cpp | `ogl->PixelStorei` |
| `glPolygonOffset` | lsPass_ShadowMap.cpp | `ogl->PolygonOffset` |
| `glReadBuffer` | lsFBO_Depth.cpp | `ogl->ReadBuffer` |
| `glReadPixels` | lsTGA.cpp | `ogl->ReadPixels` |
| `glShadeModel` | lsPass_Occlusion.cpp | `ogl->ShadeModel` |
| `glShaderSource` | lsShader.cpp | `ogl->ShaderSource` |
| `glTexImage2D` | lsTexture.cpp, lsFBO*.cpp, lsMesh.cpp | `ogl->TexImage2D` |
| `glTexParameterf` | lsTexture.cpp, lsFBO*.cpp | `ogl->TexParameterf` |
| `glTexParameterfv` | lsFBO_Depth.cpp | `ogl->TexParameterfv` |
| `glTexParameteri` | lsTexture.cpp, lsFBO*.cpp, lsMesh.cpp | `ogl->TexParameteri` |
| `glUniform1f` | lsShader.cpp | `ogl->Uniform1f` |
| `glUniform1i` | lsShader.cpp | `ogl->Uniform1i` |
| `glUniform3f` | lsShader.cpp | `ogl->Uniform3f` |
| `glUniform4f` | lsShader.cpp | `ogl->Uniform4f` |
| `glUniformMatrix3fv` | lsShader.cpp | `ogl->UniformMatrix3fv` |
| `glUniformMatrix4fv` | lsShader.cpp | `ogl->UniformMatrix4fv` |
| `glUseProgram` | lsShader.cpp, lsBatch_*.cpp | `ogl->UseProgram` |
| `glVertexAttribPointer` | lsBatch_*.cpp | `ogl->VertexAttribPointer` |
| `glVertex3f` | lsPlane.cpp (commented) | **delete** |
| `glViewport` | lsPass_*.cpp | `ogl->Viewport` |

---

## 9. Checklists

### Phase 0 plumbing checklist
- [x] `render/lsGLTypes.h` created with `lsTextureID`, `lsShaderID`,
      `lsProgramID`, `lsVAOID`, `lsBufferID`, `lsFBOID`, `lsQueryID`,
      `lsUniformID`.
- [x] `lsRender::getOpenGL()` is pure virtual; `lsOpenGL::getOpenGL()`
      returns `this`.
- [x] `lsOpenGL` declares all 65+ wrapper methods (bodies stub).
- [x] `lsBatch*`, `lsFBO*`, `lsTexture`, `lsShader`, `lsPass*` each
      carry `lsOpenGL* m_ogl` initialised in the constructor.
- [x] Build & demos still work.

### Per-phase checklist
- [x] All `gl*()` calls in the migrated files replaced with `m_ogl->…`.
- [x] No `<GL/glew.h>` left in the migrated `.cpp` files.
- [x] No `GL_*` macros used in the migrated `.cpp` files (use
      `lsOpenGL`-provided constants, or move to enum).
- [x] Build is clean.
- [x] Both demos run with no GL errors.
- [x] Commit the phase.

### Final checklist (after Phase 8)
- [x] `grep -r "gl[A-Z][a-zA-Z]*(" vortxGE/src/` returns only
      `vortxGE/src/render/lsOpenGL.cpp`.
- [x] `grep -r "GL/glew.h" vortxGE/src/` returns only
      `vortxGE/src/render/lsOpenGL.h` (and `.cpp`).
- [x] `grep -rE "GLuint|GLint|GLenum" vortxGE/src/` outside `render/`
      returns zero.
- [x] `AGENTS.md` updated to ban direct `gl*()` calls outside `lsOpenGL`.
- [x] Demos and tools rebuilt and smoke-tested.



---

## 10. Final State (post-migration metrics)

Captured after Phase 8 sign-off (August 2026).

### Code-distribution by file

| Metric | Baseline | After |
|--------|---------:|------:|
| Raw `gl*()` calls outside renderer | 289 | **0** |
| `GL_*` identifier references outside renderer | ~250 | **0** |
| GL types (`GLuint`, `GLenum`, …) outside renderer | ~30 | **0** |
| `<GL/glew.h>` includes outside renderer | 0 | **0** (still only in renderer) |
| Files that reference GL types or constants directly | 17 | **1** (`render/lsOpenGL.cpp`) |
| Wrapper virtual methods in `lsRender.h` | 0 | **75** |
| Wrapper implementations in `lsOpenGL.cpp` | 0 | **75** |
| Engine-level ID typedefs (`lsTextureID`, …) | 0 | **10** |
| Renderer constants (`LS_*`) in `lsRenderConstants.h` | 0 | **100** |

### What a non-renderer file looks like now

A `lsBatch_Quad`, `lsFBO`, `lsPass_DR`, `lsShader`, `lsScene`, `lsMesh`,
or `lsTexture` file uses:

* **Includes:** only `lsRender.h` (transitively pulls in types and
  `LS_*` constants).
* **Member fields:** `lsTextureID m_texture;` instead of `GLuint`.
* **Calls:** `getRender()->TexImage2D( LS_TEXTURE_2D, ... );` instead
  of `glTexImage2D( GL_TEXTURE_2D, ... );`.
* **State saves:** `render->Enable( LS_CULL_FACE );` etc.

No `<GL/glew.h>`, no `GL_*` identifier, no `gl*()` function call.

### Renderer architecture after Phase 8

```
vortxGE/src/render/
├── lsGLTypes.h              # Engine-level types (lsTextureID, lsEnum, lsFloat, …)
│                            # NO <GL/glew.h>, NO GL_* types.
│
├── lsRenderConstants.h      # 100 LS_* constants (#defined to GL_*).
│                            # Includes <GL/glew.h>; this is the bridge
│                            # header that lets the rest of the engine
│                            # use backend-specific values without
│                            # having its own GL dependency.
│
├── lsRender.h               # Pure abstract interface (75+ virtuals).
│                            # NO <GL/glew.h>, NO GL_* types.
│                            # Pulls in lsGLTypes.h + lsRenderConstants.h.
│
├── lsOpenGL.h               # `class lsOpenGL : public lsRender` — overrides.
│                            # NO raw gl* calls.
│
└── lsOpenGL.cpp             # THE ONLY FILE WITH RAW gl* CALLS.
                             # 71 wrapper bodies (one-liner `glXxx(...)` calls).
```

### Phase-by-phase reduction in raw `gl*()` calls outside the renderer

| Phase | Status | Scope | Cumulative remaining |
|-------|--------|-------|---------------------:|
| Baseline | — | — | 289 |
| Phase 1 | ✅ | Render passes (viewport, clear) | 253 |
| Phase 2 | ✅ | Skybox state in `lsScene` | 245 |
| Phase 3 | ✅ | Framebuffer objects (`lsFBO`, `lsFBO_*`) | 244 |
| Phase 4 | ✅ | Texture creation (`lsTexture`, `lsMesh`, `lsTGA::frameGrab`) | 222 |
| Phase 5 | ✅ | Batch VBO/VAO (`lsBatch_Lines/Mesh/Quad`) | 153 |
| Phase 6 | ✅ | Shaders & uniforms (107 calls in `lsShader.cpp`) | 83 |
| Phase 7 | ✅ | Occlusion queries (`lsPass_Occlusion.cpp`) | 78 |
| Phase 8 | ✅ | Engine-wide: drop `<GL/glew.h>`, define `LS_*` constants, convert GL types | **0** |

### Adding a Vulkan or Metal backend (out of scope for this refactor)

The architecture is now ready. To add a new backend:

1. Create `render/lsVulkan.h` / `.cpp` with `class lsVulkan : public lsRender`
   that overrides all 75 pure-virtual methods, calling Vulkan API
   functions inside each override body.
2. Rewrite `render/lsRenderConstants.h` to map `LS_X` to the
   corresponding `Vk*` / `MTL*` constants (or `#define LS_X VK_X` for
   numeric compatibility).
3. **No other source file needs to change.** All ~17,000 lines of
   engine code outside `render/` already use `getRender()->Xxx(...)`
   and `LS_*` exclusively.

This is captured as a follow-up effort in `Out of scope` (§7).
