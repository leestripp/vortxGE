# vortxGE

## Version
vortxGE v0.7.2

## Overall Architecture & Philosophy

vortxGE is a C++ 3D engine designed with a modular, scene-graph-based architecture. It leverages modern OpenGL for rendering and the Bullet Physics library for physics simulation. The core philosophy is to separate concerns into distinct managers and data blocks, promoting flexibility and extensibility.

The engine follows a **Right-Handed Coordinate System** with the view direction along the **-Z axis**, consistent with OpenGL's conventions.
*   **+X**: Right
*   **+Y**: Up
*   **-Z**: Forward (View Direction)

---

## Engine Lifecycle

### Initialization

The vortxGE engine follows a specific initialization sequence:

1.  **Create Instance**: Instantiate the `vortxGE` class
2.  **Create Render Context**: Set up your windowing library (GLFW3, GTKmm, etc.) and create a valid OpenGL context
3.  **Call Init()**: Initialize the engine subsystems

```cpp
// Example initialization sequence
vortxGE* engine = new vortxGE();
// ... create window and OpenGL context ...
engine->Init();
```

### Main Loop

The engine operates on a simple main loop pattern:

```cpp
// Main game loop
while (!shouldClose) {
    double deltaTime = timer.GetDelta();
    
    // Update: Process physics, animations, and game logic
    engine->Update(deltaTime);
    
    // Render: Draw the scene
    engine->Render();
    
    // Swap buffers, handle input, etc.
}
```

### Cleanup

Always call `Cleanup()` before destroying the engine instance:

```cpp
engine->Cleanup();
delete engine;
```

---

## Core Components

The engine is structured around a central `vortxGE` class that orchestrates several managers.

### `vortxGE` Class

The main engine class located in `vortxGE/src/vortxGE.h`. It initializes and owns the core managers.

#### Public Methods

| Method | Description |
|--------|-------------|
| `vortxGE()` | Constructor - initializes pointers to null |
| `~vortxGE()` | Destructor - calls Cleanup() internally |
| `int Init()` | Initializes all subsystems. Must be called after creating a valid OpenGL context. Returns 0 on success, 1 on error. |
| `void Cleanup()` | Releases all allocated resources |
| `void Update(double delta)` | Updates all active controls and scene elements. Handles physics stepping, animation updates, and scene graph transformations. |
| `void Render()` | Renders all active controls to the screen |
| `void Resize(int x, int y, int width, int height)` | Notifies all passes of viewport changes |

#### Manager Accessors

| Method | Returns | Description |
|--------|---------|-------------|
| `getControlManager()` | `lsControlManager*` | Access the control hierarchy for UI and viewports |
| `getShaderManager()` | `lsShaderManager*` | Access shader loading and management |
| `getAssetManager()` | `lsAssetManager*` | Access global asset loading |
| `getRender()` | `lsRender*` | Access the OpenGL wrapper |
| `getOpenAL()` | `lsOpenAL*` | Access audio system |

#### Internal Subsystems

During `Init()`, the following subsystems are created:

1.  **lsStats** - Performance statistics tracking
2.  **lsRender** (lsOpenGL) - OpenGL rendering wrapper
3.  **lsShaderManager** - Shader compilation and management
4.  **lsAssetManager** - Global asset loading and caching
5.  **lsControlManager** - UI and viewport hierarchy
6.  **lsOpenAL** - Audio system initialization

### Managers

*   **`lsControlManager`**: Manages a hierarchy of `lsControl` objects. This forms the basis of the UI and viewport layout. The top-level `lsControlManager` in `vortxGE` is the entry point for the `Update` and `Render` calls.
*   **`lsAssetManager`**: Responsible for creating, loading, and managing shared `lsAsset` resources like meshes and textures. This prevents redundant loading of assets. Each `lsViewer` or `lsComposite` control has its own `lsAssetManager`.
*   **`lsShaderManager`**: Loads, compiles, and manages GLSL shader programs (`lsShader`). It provides a central repository for shaders, which can be retrieved by name.
*   **`lsSceneManager`**: Manages a list of `lsScene` objects. Each `lsViewer` or `lsComposite` has its own `lsSceneManager`.
*   **`lsPassManager`**: Manages the rendering pipeline by executing a list of `lsPass` objects in sequence. Each `lsViewer` or `lsComposite` has its own `lsPassManager`.
*   **`lsLightManager`**: Attached to each `lsObject`, it manages a list of `lsLight`s that affect that object.
*   **`lsEventManager`**: Handles a queue of events within a `lsScene`. Events are used for deferred actions like node deletion or reporting physics collisions.
*   **`lsAnimationManager`**: Manages animation curves (`lsAnimProperty`) for animatable nodes (`lsAnimNode`).

### `lsOpenGL`

A lightweight wrapper around the OpenGL API located in `vortxGE/src/render/lsOpenGL.h`. It holds references to the `lsShaderManager` and `lsStats` for easy access and manages basic OpenGL state, such as the default framebuffer ID.

---

## Configuration

### Version Information

Version numbers are defined in `vortxGE/src/config.h`:

```cpp
#define vortxGE_VERSION_MAJOR 0
#define vortxGE_VERSION_MINOR 7
#define vortxGE_VERSION_PATCH 2
```

### Debug Flags

Debug features can be enabled by uncommenting defines in `vortxGE/src/vortx_debug.h`:

| Flag | Category | Description |
|------|----------|-------------|
| `VORTX_DEBUG_STATS` | Stats | Enable statistics debugging |
| `VORTX_DEBUG_OCCLUSION` | Rendering | Enable occlusion culling debug output |
| `VORTX_DEBUG_PASSES` | Rendering | Enable pass debug output |
| `VORTX_DEBUG_RENDER` | Rendering | Enable render debug output |
| `VORTX_DEBUG_SHADERS` | Shaders | Enable shader debug output |
| `VORTX_DEBUG_ASSETS` | Assets | Enable asset loading debug output |
| `VORTX_DEBUG_ANIMATION` | Animation | Enable animation debug output |
| `VORTX_DEBUG_COREEVENTS` | Events | Enable core event debug output |
| `VORTX_DEBUG_GAMEEVENT` | Events | Enable game event debug output |
| `VORTX_DEBUG_PHYSICS` | Physics | Enable physics debug output |
| `VORTX_DEBUG_RAYCAST` | Physics | Enable raycast debug output |
| `VORTX_DEBUG_LOADSCENE` | Scene | Enable scene loading debug output |
| `VORTX_DEBUG_LIGHTS` | Scene | Enable light debug output |
| `VORTX_DEBUG_NODES` | Scene | Enable node debug output |
| `VORTX_DEBUG_NODETHREADS` | Scene | Enable node threading debug output |
| `VORTX_DEBUG_WAV` | DataBlocks | Enable WAV audio debug output |
| `VORTX_DEBUG_TGA` | DataBlocks | Enable TGA image debug output |

---

## Scene Graph (`lsNode` Hierarchy)

The engine uses a hierarchical scene graph composed of `lsNode` subclasses.

*   **`lsNode`**: The abstract base class for all scene graph elements. It defines the parent-child relationships and holds transformation data (`position`, `rotation`, `scale`) and the final `ModelMatrix`.
*   **`lsScene`**: The root node of a scene graph. It owns the scene's `lsBulletWorld`, `lsEventManager`, and `lsAssetManager`. It also manages the `draw_list` for rendering.
*   **`lsAnimNode`**: A subclass of `lsNode` that can be animated. It owns an `lsAnimationManager` and a list of `lsProperty` objects that can be animated.
*   **`lsObject`**: A renderable `lsAnimNode`. It holds a pointer to an `lsAsset` (e.g., a mesh) and an `lsLightManager`. It can also have an associated `lsBulletObject` for physics.
*   **`lsCamera`**: A specialized `lsAnimNode` that defines a viewpoint. It contains a `ViewMatrix` and an `lsFrustum` for projection and culling. It inherits from `lsAnimNode` and can be animated.
*   **`lsLight`**: A specialized `lsCamera` that acts as a light source. It contains lighting properties (color, intensity) and can cast shadows (acting as a camera for a shadow map pass).
*   **`lsNode_FirstPersonRig`**: A player controller that combines a physics capsule (`lsBulletObject`) with a camera (`lsCamera`) for first-person movement and mouse look.
*   **`lsObjectEmitter`**: A node that periodically spawns new `lsObject` instances with physics properties.

---

## Data Blocks

Data blocks represent the actual data used by the engine components.

*   **`lsAsset`**: The base class for shared resources. Assets are created by an `lsAssetManager`.
    *   `lsAsset_Mesh`: Represents a 3D model, loaded from formats like `.vbm` or `.glb`.
    *   `lsAsset_Texture`: Represents a 2D texture, loaded from an image file.
    *   `lsAsset_BulletShape_*`: Represents a collision shape for the physics engine.
*   **`lsMesh`**: A container for one or more `lsBatch` objects. It represents the renderable data of a 3D model and holds the overall bounding sphere for the model.
*   **`lsBatch`**: The lowest-level renderable unit. It contains VBOs and a VAO for a piece of geometry and is associated with a material and a shader.
*   **`lsFBO`**: A wrapper for an OpenGL Framebuffer Object, used for rendering to textures (e.g., in `lsPass_FBO` and `lsPass_ShadowMap`).
*   **`lsBulletShape`**: A wrapper around a `btCollisionShape` from the Bullet Physics library.

---

## Rendering Pipeline (`lsPass` Hierarchy)

The rendering process is defined by a sequence of passes managed by an `lsPassManager`.

*   **`lsPass`**: The abstract base class for a rendering pass. It defines the `Update` and `Render` interface and holds references to the `lsScene` and `lsCamera` it should use.
*   **`lsPass_DR` (Direct Render)**: A simple pass that renders a scene directly to the default framebuffer (the screen).
*   **`lsPass_FBO`**: Renders a scene to an `lsFBO` (a texture), which can then be used in other passes (e.g., for post-processing or display on a quad).
*   **`lsPass_ShadowMap`**: A specialized pass that renders the scene from a light's perspective into a depth-only `lsFBO` to create a shadow map.

The render loop typically flows as follows:
1.  `vortxGE::Render()` calls `lsControlManager::Render()`.
2.  `lsControlManager` calls `Render()` on its `lsControl`s (e.g., `lsViewer`).
3.  `lsViewer::Render()` calls `lsPassManager::Render()`.
4.  `lsPassManager` iterates through its `lsPass` list, calling `Render()` on each.
    *   A `lsPass_ShadowMap` might run first to generate the depth texture.
    *   A `lsPass_FBO` then runs, rendering the main scene. Its shaders can sample the shadow map texture generated in the previous pass.
    *   Another `lsPass_DR` might run to draw the FBO's texture to a quad on the screen.

---

## Physics

*   **`lsBulletWorld`**: Owned by `lsScene`, this class manages the `btDiscreteDynamicsWorld` and its configuration. It steps the physics simulation during the `Move` phase of the update loop.
*   **`lsBulletObject`**: A component attached to an `lsObject`. It wraps a `btRigidBody` and links the object's transform to the physics simulation. During the `Move` phase, the `lsObject`'s transform is updated from the `btRigidBody`'s transform.

---

## Shaders & Lighting

The engine uses a modern, PBR-like (Physically Based Rendering) shading model.
*   **Shaders**: GLSL shaders are used for rendering. They are loaded and managed by the `lsShaderManager`.
*   **Uniforms**:
    *   `pMatrix`: Projection Matrix (from `lsCamera::frustum`).
    *   `mvMatrix`: Model-View Matrix (`camera->ViewMatrix * object->ModelMatrix`).
    *   `mMatrix`: Model Matrix (`object->ModelMatrix`).
    *   `nMatrix`: Normal Matrix (inverse transpose of the upper-left 3x3 of `mvMatrix`).
    *   `sMatrix`: Light Space Matrix for shadows (`Bias * LightProjection * LightView`).
    *   Lighting uniforms (`vxLightPos`, `vxLightDiffuse`, etc.) are passed as arrays to support multiple lights.
    *   Material uniforms (`vxMatDiffuse`, `vxMatShininess`, etc.) control the surface properties.
*   **Lighting**: Lights are `lsLight` nodes in the scene. An `lsObject`'s `lsLightManager` determines which lights affect it. Light properties are passed to the shader in view space.

---

## First Person Rig Controls

lsNode_FirstPersonRig
* Mouse X position rotates the Pill ridged body.
* Mouse Y position, rotates the rigs Camera Y rotation.
* Keys: w, a, s, d : Moves the Pill ridged body with velocity so we get consistant modion.
* When in Air or falling off ledge, velocity is maintained and no movement is allowed. w, a, s, d are blocked.

