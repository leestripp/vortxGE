Yes, GLFW3 has full access to the desktop scaling factor. The issue you are experiencing—where a 125% scale "messes up" your 3D render—is a classic High-DPI coordination problem.

In modern desktop environments like GNOME (especially on Wayland), there is a distinction between **Screen Coordinates** (logical units) and **Pixels** (physical units). If you tell GLFW to create a $800 \times 600$ window, it creates a window that is 800x600 *units* large. At 125% scaling, the actual underlying buffer needs to be $1000 \times 750$ pixels to stay sharp. If your OpenGL `glViewport` is still set to 800x600, your render will appear in the bottom-left corner or look blurry/stretched.

### The Solution: Framebuffer vs. Window Size

The "Golden Rule" of GLFW is: **Always use the Framebuffer size for rendering, and the Window size for input/logic.**

#### 1. Query the Scaling Factor

If you specifically need the scale value (e.g., to scale your UI text or icons), use `glfwGetWindowContentScale`.

```cpp
float xscale, yscale;
glfwGetWindowContentScale(window, &xscale, &yscale);
// xscale and yscale will be 1.25 on your system.

```

#### 2. Fix the Rendering (The "Best" Solution)

Instead of using the width and height you passed to `glfwCreateWindow`, query the actual pixel dimensions of the "drawing surface" using `glfwGetFramebufferSize`.

```cpp
int display_w, display_h;
glfwGetFramebufferSize(window, &display_w, &display_h);
glViewport(0, 0, display_w, display_h);

```

#### 3. Handle Dynamic Scaling

If you drag your window from a 125% scale monitor to a 100% scale monitor, the window will "resize" in pixel terms even if it looks the same size to the user. You must use a callback to catch this.

```cpp
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // 'width' and 'height' here are already in physical pixels
    glViewport(0, 0, width, height);
}

// In your setup:
glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

```

---

### Comparison of Coordinate Systems

| Function | Unit | Use Case |
| --- | --- | --- |
| `glfwGetWindowSize` | **Screen Coordinates** | Positioning windows, UI layout logic. |
| `glfwGetFramebufferSize` | **Pixels** | `glViewport`, `glReadPixels`, textures. |
| `glfwGetWindowContentScale` | **Ratio** | Scaling fonts, line thickness, or UI icons. |


## We are on Wayland

---

## Resolution (as of August 2026)

The vortxGE GLFW3 demo (`demos/vortxGEdemo_glfw3/src/glfwApp.cpp`) now applies the
"Golden Rule" above in **three** places, with belt-and-braces against the
GLFW-Wayland quirks. After this fix the FBO + quad-render pipeline correctly
fills the window at any desktop scale (100%, 125%, 150%, 175%, 200%).

### The fix at a glance

```cpp
// 1) Before glfwCreateWindow — request a scaled framebuffer.
glfwWindowHint( GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE );
m_window = glfwCreateWindow( 1920, 1080, "vortxGEdemo_glfw3", NULL, NULL );

// 2) After creating the window — query physical pixels AND content scale.
glfwGetFramebufferSize( m_window, &width, &height );
float xscale = 1.0f, yscale = 1.0f;
glfwGetWindowContentScale( m_window, &xscale, &yscale );
if( xscale > 0.0f && yscale > 0.0f &&
    !( xscale == 1.0f && yscale == 1.0f ) )
{
    width  = (int)( width  * xscale );
    height = (int)( height * yscale );
}
demoscene->Resize( 0, 0, width, height );

// 3) Wire the resize callback to the FRAMEBUFFER-size variant, not the
//    WINDOW-size variant. The latter returns logical units.
glfwSetFramebufferSizeCallback( m_window, Resize_Callback );
//   where Resize_Callback forwards (width, height) to demoscene->Resize
//   so the FBO is recreated on every actual framebuffer-size change.
```

### Why all three pieces?

* The `GLFW_SCALE_FRAMEBUFFER` hint (GLFW 3.4+, Wayland + macOS) makes the
  compositor allocate the framebuffer at logical * scale directly. On older
  GLFW it's a no-op, which is why the **second** step (multiplying by
  `glfwGetWindowContentScale`) is required for the fix to work on every
  Wayland version.
* `glfwSetFramebufferSizeCallback` (instead of `glfwSetWindowSizeCallback`)
  is the only callback variant that reports physical pixel sizes. The
  WINDOW-size callback reports logical units, which would have put the
  engine back into the same "Quad doesn't fit" state on every resize.

### Bug symptom before the fix

User with 125% desktop scale on a 1920×1080 logical window:

* OS allocated a 2400×1350 framebuffer.
* `glfwGetFramebufferSize()` (in some Wayland/GLFW versions) returned
  1920×1080 — the LOGICAL size — without applying the 1.25x scale.
* The demo created the FBO at 1920×1080 and called `glViewport(0, 0, 1920, 1080)`.
* Result: the FBO texture covered only the top-left ~80% of the visible
  window; the remaining ~20% (right and bottom strip) showed the cleared
  default-framebuffer colour (black).

### Engine-side support

`vortxGE::lsFBO::Resize()` was added in the same fix (August 2026) so the
FBO + colour/depth textures can be recreated at any new size — see
`vortxGE/src/datablocks/lsFBO.h` and `lsFBO_ColourDepth_8.cpp`. The pass
chain (`lsPass_FBO::Resize`) calls `m_lsfbo->Resize()` and updates its
viewport; the demo's `glfwApp` code above is what supplies the correct
physical-pixel size at the right times.

### References

* [glfw/glfw#1168](https://github.com/glfw/glfw/issues/1168) — Wrong frame buffer size under GNOME/Wayland in a high resolution screen (canonical report)
* [discourse.glfw.org/t/2945](https://discourse.glfw.org/t/glfw-framebuffersize-is-messed-up-at-start/2945) — `glfwFrameBufferSize` is messed up at start (on Wayland)
* [discourse.glfw.org/t/1547](https://discourse.glfw.org/t/using-glfw-under-wayland/1547) — Using GLFW under Wayland
* [discourse.gnome.org/t/14225](https://discourse.gnome.org/t/full-explanation-of-current-hidpi-fractional-and-integer-scaling-support-in-wayland/14225) — Wayland HiDPI scaling explanation
* [ocornut/imgui#8921](https://github.com/ocornut/imgui/issues/8921) — "Some examples are double-scaled on HiDPI screens on Wayland"
