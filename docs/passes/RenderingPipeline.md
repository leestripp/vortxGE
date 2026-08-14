# Rendering Pipeline

> **2026-08-14 update — this document describes the DEPRECATED path.**
> The stage-based pipeline (`vortxGE/src/pipeline/`) is the default since
> E3.1; the classic pass system below runs only with
> `VORTX_LEGACY_PIPELINE=1` and is retired in a future release. The new
> stage-based pipeline (`vortxGE/src/pipeline/`, opt-in in the demos via
> `VORTX_PIPELINE=1`) composes explicit stages: `lsShadowStage` /
> `lsCsmStage` (sun CSM), `lsClusterStage` (clustered light culling),
> then the scene pass. Clustered lighting supports up to 64 lights with
> per-fragment culling — see Plan.md L1–L5 and the `lsClusterGrid` /
> `lsCsm` math modules (both unit-tested).


The rendering pipeline in vortxGE uses a **pass-based system** that allows flexible composition of rendering effects. Each pass defines how a scene should be rendered - to the screen, to a texture, or for shadow mapping.

---

## Architecture Overview

```
vortxGE::Render()
    │
    └─> lsControlManager::Render()
            │
            └─> lsViewer::Render() / lsComposite::Render()
                    │
                    └─> lsPassManager::Render()
                            │
                            ├─> lsPass_ShadowMap (optional)
                            │       Renders depth from light's perspective
                            │
                            ├─> lsPass_FBO (optional)
                            │       Renders to texture for post-processing
                            │
                            └─> lsPass_DR
                                    Renders to screen
```

---

## Pass Types

Defined in `vortxGE/src/passes/lsPass.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `LSPT_NONE` | 0 | No pass type |
| `LSPT_DIRECTRENDER` | 1 | Direct render to screen |
| `LSPT_FBO` | 2 | Render to framebuffer |
| `LSPT_SHADOWMAP` | 3 | Shadow map generation |
| `LSPT_OCCLUSION` | 4 | Occlusion culling pass |

---

## lsPass Class

The base class for all rendering passes, defined in `vortxGE/src/passes/lsPass.h`.

### Class Definition

```cpp
class lsPass
{
public:
    lsPass(lsRender* render);
    virtual ~lsPass();

    // Must be implemented by subclasses
    virtual void Update() = 0;
    virtual void Render() = 0;
    virtual void Resize(int x, int y, int width, int height) = 0;
    virtual const string& toString() = 0;
};
```

### Key Properties

| Property | Type | Description |
|----------|------|-------------|
| `scene` | `lsScene*` | Scene to render |
| `camera` | `lsCamera*` | Camera for rendering |
| `cull_camera` | `lsCamera*` | Camera for frustum culling |
| `light` | `lsLight*` | Light source (for shadow maps) |
| `position` | `lsVector2` | Viewport position |
| `size` | `lsVector2` | Viewport size |
| `vratio` | `float` | Vertical ratio |
| `background_colour` | `lsVector4` | Clear color (r, g, b, a) |
| `name` | `string` | Pass name |
| `pass_type` | `int` | Type identifier |

### Viewport Management

```cpp
// Set viewport
void setPosition(const lsVector2& val);
void setSize(const lsVector2& val);

// Set background color
void setBackground_colour(const lsVector4& val);

// Get vertical ratio
float getVratio() const;
void setVratio(float val);
```

### Scene/Camera Binding

```cpp
// Bind scene and cameras
void setScene(lsScene* val);
lsScene* getScene() const;

void setCamera(lsCamera* val);
lsCamera* getCamera() const;

void setCull_camera(lsCamera* val);
lsCamera* getCull_camera() const;

void setLight(lsLight* val);
lsLight* getLight() const;
```

---

## lsPassManager Class

Manages a sequence of passes. Defined in `vortxGE/src/passes/lsPassManager.h`.

### Class Definition

```cpp
class lsPassManager
{
public:
    lsPassManager(lsRender* render);
    virtual ~lsPassManager();

    // Render all passes in sequence
    void Render();

    // Handle window resize
    void Resize(int x, int y, int width, int height);

    // Pass management
    void Add_Pass(lsPass* pass);
    void Del_Pass(lsPass* pass);

    lsPass* getPass_list() const;
};
```

### Usage Example

```cpp
// Create pass manager
lsPassManager* passMgr = new lsPassManager(render);

// Create and configure passes
lsPass_ShadowMap* shadowPass = new lsPass_ShadowMap(render);
shadowPass->setScene(scene);
shadowPass->setLight(spotLight);

lsPass_DR* drPass = new lsPass_DR(render);
drPass->setScene(scene);
drPass->setCamera(camera);

// Add to manager
passMgr->Add_Pass(shadowPass);
passMgr->Add_Pass(drPass);
```

---

## Pass Implementations

### lsPass_DR (Direct Render)

Renders a scene directly to the default framebuffer (screen). Defined in `vortxGE/src/passes/lsPass_DR.h`.

**Features:**
- Renders to screen
- Supports viewport positioning
- Configurable background color

**Usage:**
```cpp
lsPass_DR* drPass = new lsPass_DR(render);
drPass->setScene(scene);
drPass->setCamera(camera);
drPass->setBackground_colour(lsVector4(0.1f, 0.1f, 0.2f, 1.0f));
```

---

### lsPass_FBO (Frame Buffer Object)

Renders a scene to a texture (framebuffer). Defined in `vortxGE/src/passes/lsPass_FBO.h`.

**Features:**
- Renders to texture for post-processing
- Configurable color and depth attachments
- Can render to a quad for display

**Usage:**
```cpp
lsPass_FBO* fboPass = new lsPass_FBO(render);
fboPass->setScene(scene);
fboPass->setCamera(camera);

// Configure FBO properties
fboPass->setWidth(1920);
fboPass->setHeight(1080);
fboPass->setFbo_type(LSFBO_COLORDEPTH);
```

---

### lsPass_ShadowMap

Renders a shadow map from a light's perspective. Defined in `vortxGE/src/passes/lsPass_ShadowMap.h`.

**Features:**
- Depth-only rendering
- Configurable shadow resolution
- Bias settings to prevent shadow acne
- Light space matrix calculation

**Usage:**
```cpp
lsPass_ShadowMap* shadowPass = new lsPass_ShadowMap(render);
shadowPass->setScene(scene);
shadowPass->setLight(spotLight);
shadowPass->setPixel_x(2048.0f);  // Shadow map resolution
shadowPass->setPixel_y(2048.0f);
shadowPass->setShadow_bias(0.005f);
```

---

### lsPass_Occlusion

Performs occlusion culling using hardware occlusion queries. Defined in `vortxGE/src/passes/lsPass_Occlusion.h`.

**Features:**
- CPU-side occlusion testing
- Can mark objects as occluded
- Reduces draw calls for hidden objects

---

## lsRender Class

The abstract base renderer, defined in `vortxGE/src/render/lsRender.h`.

### Class Definition

```cpp
class lsRender
{
public:
    lsRender();
    virtual ~lsRender();

    // Initialization
    virtual void Init() = 0;

    // Error checking
    virtual void CheckErrors(const string& name) = 0;

    // Framebuffer management
    virtual void StoreDefaultFramebuffer() = 0;
    virtual void RestoreDefaultFramebuffer() = 0;

    // Managers
    lsStats* getStats() const;
    lsShaderManager* getShaderManager() const;
};
```

---

## lsOpenGL Class

OpenGL implementation of the renderer, defined in `vortxGE/src/render/lsOpenGL.h`.

### Class Definition

```cpp
class lsOpenGL : public lsRender
{
public:
    lsOpenGL();
    virtual ~lsOpenGL();

    // Initialize OpenGL
    virtual void Init() override;

    // Check for GL errors
    virtual void CheckErrors(const string& name) override;

    // Framebuffer operations
    virtual void StoreDefaultFramebuffer() override;
    virtual void RestoreDefaultFramebuffer() override;

    // GL information
    void glInfo();
    void rendererChanged();
};
```

### Key Methods

```cpp
// Print OpenGL renderer information
void glInfo();

// Called when the renderer changes
void rendererChanged();

// Store/restore default framebuffer
void StoreDefaultFramebuffer();
void RestoreDefaultFramebuffer();
```

---

## Render Types

Defined in `vortxGE/src/render/lsRenderTypes.h`:

```cpp
enum RenderType
{
    OPENGL_RENDER,
    VULKAN_RENDER,  // Future
    DIRECTX_RENDER   // Future
};
```

---

## Rendering Flow

### Step 1: Update Phase

```cpp
// In vortxGE::Update()
void vortxGE::Update(double delta)
{
    stats->Loop_Start();
    if (controlmanager) controlmanager->Update(delta);
}
```

### Step 2: Render Phase

```cpp
// In vortxGE::Render()
void vortxGE::Render()
{
    if (controlmanager) controlmanager->Render();
    render->RestoreDefaultFramebuffer();
    stats->Loop_Stop();
    stats->Update();
}
```

### Step 3: Pass Execution

Each pass in the `lsPassManager` executes:

```cpp
void lsPassManager::Render()
{
    lsPass* pass = pass_list;
    while (pass)
    {
        pass->Update();  // Update state
        pass->Render();   // Draw
        pass = pass->getNext_pass();
    }
}
```

---

## Viewport Management

Passes support viewport positioning for split-screen or picture-in-picture:

```cpp
// Set viewport position and size
lsPass_DR* pass = new lsPass_DR(render);
pass->setPosition(lsVector2(0.0f, 0.0f));
pass->setSize(lsVector2(0.5f, 1.0f));  // Left half of screen
```

---

## Shadow Mapping Pipeline

### Configuration

1. Create a spot light with shadow mapping enabled
2. Add a `lsPass_ShadowMap` pass to render from the light's view
3. Add a `lsPass_DR` pass to render the scene with shadows

### Shader Integration

Shaders receive shadow data via uniforms:

```glsl
// Vertex shader
uniform mat4 sMatrix;  // Light space matrix

// Fragment shader
uniform sampler2D shadowMap;
uniform float shadow_bias;
```

### Shadow Matrix Calculation

```cpp
// In lsLight::buildShadowmap_matrix()
void lsLight::buildShadowmap_matrix()
{
    // Bias * Projection * View
    lsMatrix4x4 biasMatrix;
    biasMatrix.setIdentity();
    biasMatrix.translate(0.5f, 0.5f, 0.5f);
    biasMatrix.scale(0.5f, 0.5f, 0.5f);

    ShadowmapMatrix = biasMatrix * getProjectionMatrix() * ViewMatrix;
}
```

---

## Best Practices

1. **Pass Order**: Shadow passes should run before main render passes
2. **Viewport**: Set correct viewport for each pass
3. **Cleanup**: Delete passes when no longer needed
4. **Resize**: Call `Resize()` on pass manager when window size changes
5. **Culling**: Use appropriate cull cameras for optimization
