# Plan.md — vortxGE Renderer Roadmap

> **Active plan:** Vulkan primary renderer (Part 2).
> **Status:** V3 done (2026-08-04), V4 in progress.

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

---

# Part 3 — Render Pipeline Refactor (lsControl/lsViewer/passes → lsRenderPipeline)

## Why

The control/viewer/pass architecture fights everyone (user decision
2026-08-04: "I was never happy with it"):

- **Composite is implicit magic.** Displaying the scene FBO requires a
  hidden second scene ("Quad Viewer": lsViewer + lsScene + ortho camera
  + lsObject with a 180°-rotated quad + lsPass_DR) just to blit a
  texture to screen. The 180° rotation silently flips winding (backend
  culling hazard — cost us hours on Vulkan).
- **Cameras are over-wired.** lsPass carries `camera` AND `cull_camera`;
  cameras must live as scene nodes; getting a working view means
  touching scene graph, pass manager and control chain.
- **Three parallel hierarchies.** lsControl chain (next_control) ×
  per-control lsSceneManager/lsPassManager/lsAssetManager × per-pass
  linked list — data flow is unreadable.
- **Global-state leaks between passes.** Viewport/FBO bindings set in
  one pass leak into the next (observed: swapchain pass opening with
  the FBO pass's stale viewport).
- **HiDPI is scattered.** Logical-vs-physical pixel handling lives in
  the demo app, passes and FBOs simultaneously (Vulkan swapchain ended
  up 2400×1350 physical while the engine renders 1920×1080 logical).

## Target architecture

```
lsRenderPipeline  (one per view/window; owned by the app, not a control chain)
├── lsTargetPool            named render targets, auto-resize on ONE
│                           framebuffer-size event (single HiDPI owner)
│     "scene" (colour+depth)   "shadow[0..N]" (depth)   "screen"
├── stages: ordered vector<lsRenderStage*>   (explicit, data-driven)
│     lsSceneStage   { scene, camera, target, flags }      → renders
│                    an lsScene with an explicit lsCamera into a target
│     lsShadowStage  { light, target }                     → depth maps
│     lsBlitStage    { srcTarget, dstTarget|screen }       → FBO→screen
│                    (replaces the entire Quad-Viewer hack)
│     lsEffectStage  { shader, srcTarget, dstTarget }      → post-fx
│                    (future: bloom/SSAO/TAA chains)
└── each stage sets ALL state it needs (viewport, depth, blend) —
    nothing leaks in or out (backends snapshot/restore per stage)
```

Design rules:

1. **Explicit over implicit.** Every stage declares inputs/outputs up
   front — the pipeline can later reorder/parallelise/validate them
   (this is the hook for a future frame graph and deferred G-buffer
   stage: `lsDeferredStage` is just another stage type).
2. **One manager of each kind at engine level** (scene, assets) — no
   per-control triplication.
3. **Camera decoupled.** `lsCamera` usable standalone (view/proj
   without scene-node membership); a stage holds a camera*, period.
   No `cull_camera` dual-wiring (culling uses the same camera unless a
   stage explicitly overrides).
4. **No fake scenes for blits.** FBO→screen is `lsBlitStage`, one line.
5. **Single resize owner.** The app tells the pipeline the framebuffer
   size; the target pool resizes every named target; stages never call
   Viewport themselves (pipeline applies stage rect × framebuffer).
6. **Backends untouched.** Everything above the lsRender interface;
   lsOpenGL must not regress; LS_* conventions unchanged.

## Phases

| Phase | Scope | Done when |
|-------|-------|-----------|
| R0 | This plan + current-architecture doc map | committed |
| R1 | `vortxGE/src/pipeline/`: lsRenderPipeline, lsRenderStage base, lsTargetPool skeleton — alongside existing code, zero removals | builds, old path untouched |
| R2 | lsSceneStage + lsBlitStage working on GL backend | GLFW demo renders scene→"scene" target→screen via pipeline, visually identical to old path |
| R3 | Demo switched to pipeline by default (old control path behind `--legacy-controls`), Vulkan backend verified | demo parity on both backends |
| R4 | lsShadowStage (spot/sun shadow maps as stages) | shadows match old path |
| R5 | Camera decoupling (standalone lsCamera), remove cull_camera wiring | tests + demo green |
| R6 | Deprecate/remove lsControl/lsViewer/lsComposite/lsPass_* + lsPassManager/lsControlManager; migrate GTKmm demo; docs (AGENTS.md, docs/controls.md → pipeline.md), CHANGELOG | old classes gone or `#ifdef VORTXGE_LEGACY_CONTROLS`, docs updated |

Out of scope: frame-graph auto-scheduling, deferred renderer itself
(the stage API makes both possible later), multi-window pipelines.

## Definition of done (Part 3)

GLFW demo renders through lsRenderPipeline on GL and Vulkan with visual
parity; the Quad-Viewer control hack deleted; cameras usable standalone;
old control/pass classes removed or legacy-gated; CHANGELOG + docs.
