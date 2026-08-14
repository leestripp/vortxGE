# Vulkan Backend Notes (lsVulkan)

Renderer-internals documentation for `vortxGE/src/render/lsVulkan.*`.
For the GL/VK abstraction plan see
[../plans/renderer_abstraction.md](../plans/renderer_abstraction.md).

## Per-frame uniform ring (dynamic UBO)

All per-draw uniform data is staged into one persistently-mapped
uniform buffer, `m_uboRingSize` bytes (default **16 MB**). Each draw
call gets a slice from `allocUboSlice()`, memcpys its UBO block there,
and binds it via dynamic offset. The cursor resets in `BeginFrame()`.

**The GPU reads a slice at execution time, after submit — not when the
command is recorded.** A slice must therefore stay untouched for the
rest of the frame: a mid-frame ring wrap makes later draws overwrite
the *earliest* draws' uniforms before execution. Wrapping is a
corruption bug, not a benign reset. If you ever see
`WARN: lsVulkan UBO ring wrapped` in a log, raise the ring size.

### Incident record: the black-sky "sweet spot" (2026-08-14, commit 5794ee2)

- **Symptom:** the whole sky vanished to pure black (no gradient, no
  stars) at a deterministic camera orientation, restored when rotating
  away. Vulkan only; OpenGL unaffected (no ring).
- **Cause:** the ring was 1 MB. Long play sessions grow the per-frame
  draw count (emitter cube accumulation). In busy views the frame's
  uploads exceeded 1 MB, the ring wrapped mid-frame, and the sky —
  drawn *first* in the scene pass — had its slice (view/proj matrices
  + sky params) overwritten by later uploads. Later draws kept
  still-valid slices, so the rest of the scene rendered normally. The
  "sweet spot" was any view whose draw count pushed post-sky uploads
  past the sky's ring offset.
- **Fix:** ring default raised to 16 MB (~16k typical draws of
  headroom); `VORTX_UBO_RING_KB=<kb>` env override (set it small,
  e.g. 64, to force wraps in tests).
- **Repro note:** `VORTX_FIXED_DT` disables the demo's cube emitter,
  so deterministic capture runs have almost no draw-count churn — ring
  wraps need long, emitter-active sessions to reproduce.

## Other VK diagnostic env vars

| Var | Effect |
|---|---|
| `VORTX_UBO_RING_KB` | Override uniform-ring size (KB) |
| `VORTX_VK_DEBUG=1` | Draw-call counters at frame 30 |
| `VORTX_VK_NOCULL=1` | Disable culling in pipeline creation |
| `VORTX_SHOT=path.png` | Swapchain readback (see [../Debugging.md](../Debugging.md)) |
| `VORTX_FBO_SHOT=id:path.ppm` | Offscreen FBO readback |
| `VORTX_DEPTH_SHOT=id:path.png` | Depth FBO readback |
| `VORTX_RDC=1` | RenderDoc capture trigger |
