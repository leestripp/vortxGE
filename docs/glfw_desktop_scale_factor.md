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
