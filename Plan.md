# Plan.md — vortxGE Renderer Roadmap

> **Active plan:** Vulkan primary renderer (Part 2).
> **Status:** V2 done (2026-08-04), V3 in progress.

---

## Part 1 — Completed work (history, condensed)

Details live in `CHANGELOG.md`; this section exists so the old plan text
doesn't bury the active one.

### Renderer abstraction (Phases 0–8) ✅ 2026-08-02 — tag `0.1.0`
Centralised all raw `gl*()` calls into `render/lsOpenGL.cpp`. The rest of
the engine talks to the GPU only through `lsRender` (80 virtuals) using
engine-level handle types (`lsTextureID`, `lsFBOID`, …) and `LS_*`
constants. Result: 289 raw GL calls / ~250 `GL_*` refs removed from 16
files; exactly one TU includes `<GL/glew.h>`.

### Testing (Phases 0–5) ✅/partial 2026-08-03
doctest vendored, CTest wiring, math suites (vectors, matrices,
quaternion), beyond-math suites (events, properties, streaming).
**CI: deliberately none** (user decision — see `AGENTS.md` §4).

### Phase 6 — C++23 migration ✅ 2026-08-03 — tag `0.2.0`
### Phase 7 — Dynamic streaming asset manager (M1–M5) ✅ 2026-08-03 — tag `0.2.0`
glTF CPU-parse/GL-upload split, `lsStreamingManager` (distance-driven
residency with hysteresis + budget), demo integration, async 2-worker
parse→upload pipeline. Post-release fixes: shutdown UAF in `~lsScene`,
streaming flapping from per-pass cull cameras.

---

## Part 2 — ACTIVE PLAN: Vulkan as the primary renderer

### Why now
The `lsRender` abstraction was designed blind (GL-shaped, never exercised
by a second backend). Validating it at ~17k lines with a working demo and
green tests is the cheapest this will ever be. **User decision
(2026-08-04): Vulkan becomes the main renderer; OpenGL becomes the
fallback.**

### Design decisions
1. **Vulkan primary, OpenGL fallback.** `vortxGE` constructs `lsVulkan`
   first; if Vulkan init fails (no driver/device), it falls back to
   `lsOpenGL` and logs the fallback. `VORTX_RENDERER=vulkan|opengl`
   env var overrides.
2. **`lsOpenGL` stays stable.** The fallback must not regress: existing
   GL call sites and behaviour are preserved; new interface methods get
   thin GL shims.
3. **`LS_*` constants keep GL numeric values.** Zero churn for GL;
   `lsVulkan` translates `LS_*` → `Vk*` at its boundary (its own mapping
   unit). `lsRenderConstants.h` stays the GL bridge only.
4. **Frame lifecycle enters the interface.** Vulkan needs
   `BeginFrame()` / `EndFrame()` / `Present()`, `Resize(w,h)`, and a
   native-window hook (`SetWindow(void* glfwWindow)`). GL implements
   these as shims; the app keeps calling `glfwSwapBuffers` for GL in V0
   (swap ownership moves into backends at V6 parity).
5. **Debug builds use Vulkan validation layers**; Release builds strip.
6. **No CI** — verification is local: build, doctest suite, demo smoke
   (per `AGENTS.md` §4).

### Phases

#### V0 — Interface refactor + backend selection ✅ **DONE** (2026-08-04)
Add lifecycle methods to `lsRender` (+ GL shims), `RenderType::VULKAN`,
`VORTX_RENDERER` selection + fallback in `vortxGE`, empty `lsVulkan`
skeleton compiling in tree. **Done when:** demo runs identically on GL
(fallback path), selection env var works, suite green.

#### V1 — Vulkan core: clear a window ✅ **DONE** (2026-08-04)
Instance, physical/logical device, GLFW surface, swapchain, per-frame
sync, command buffers, `Clear()` + `Present()`. **Done when:** demo
window clears to a colour through `lsVulkan` (GL untouched).

#### V2 — Buffers + first geometry ✅ **DONE** (2026-08-04)
Vertex/index buffers (via VMA or manual), pipeline for the output quad,
GLSL→SPIR-V compile path (`glslang-tools`). **Done when:** FBO→quad
pipeline draws on Vulkan.

#### V3 — Textures, descriptors, full shaders 🔲
Texture upload/samplers, descriptor set layout matching engine uniform
patterns, PBR shader set to SPIR-V. **Done when:** scene renders
textured on Vulkan.

#### V4 — Offscreen targets (FBO) 🔲
`lsFBOID` → VkFramebuffer/renderpass; FBO pass + resize parity
(HiDPI contract). **Done when:** render-to-texture works.

#### V5 — Shadow maps 🔲
Depth-only targets + sampling in PBR shaders. **Done when:** spot/sun
shadows match GL output.

#### V6 — Parity & unification 🔲
Occlusion queries, state audit, present ownership unified in backends
(app swap removed), runtime side-by-side GL↔Vulkan comparison, docs
(AGENTS.md, docs/), CHANGELOG, release tag. **Done when:** full demo
parity, Vulkan default, GL verified as working fallback.

### Out of scope
Metal/D3D12 backends; Vulkan-specific perf features (bindless, RT);
Windows/macOS ports (Linux-only project).

### Definition of done (whole plan)
Demo runs full feature parity on Vulkan by default; GL fallback verified
by `VORTX_RENDERER=opengl`; test suite green; docs updated; release
tagged (0.3.0 at V1+ or per milestone review).
