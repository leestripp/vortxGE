# Controls and UI System

The controls system provides a hierarchical framework for managing viewports, scenes, and UI elements. It separates the rendering pipeline from the control hierarchy, allowing multiple viewports and complex layouts.

---

## Architecture Overview

```
lsControlManager
    │
    ├── lsViewer (3D viewport)
    │       ├── lsSceneManager
    │       ├── lsPassManager
    │       └── lsAssetManager
    │
    ├── lsComposite (composite output)
    │       ├── lsSceneManager
    │       ├── lsPassManager
    │       └── lsAssetManager
    │
    └── lsControl (base class)
            └── Child controls...
```

---

## Control Types

Defined in `vortxGE/src/controls/lsControl.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `LSCT_NONE` | 0 | No type |
| `LSCT_COMPOSITE` | 1 | Composite render control |
| `LSCT_MENU` | 2 | Menu control |
| `LSCT_VIEWER` | 3 | 3D viewport |
| `LSCT_LEVEL` | 4 | Level control |
| `LSCT_LEVELLOADER` | 5 | Level loader |
| `LSCT_GAMEPLAY` | 6 | Gameplay control |
| `LSCT_GAME` | 7 | Game control |
| `LSCT_SANDBOX` | 8 | Sandbox control |
| `LSCT_SANDBOXZONE` | 9 | Sandbox zone |

---

## lsControl Class

The base class for all controls. Defined in `vortxGE/src/controls/lsControl.h`.

### Class Definition

```cpp
class lsControl
{
public:
    lsControl(lsRender* render, lsControl* parent);
    virtual ~lsControl();

    // Initialization
    virtual int Init() = 0;

    // Update and render
    virtual void Update(double delta);
    virtual void Render();

    // Window resize
    virtual void Resize(int x, int y, int width, int height);

    // Hierarchy
    void Add_Control(lsControl* control);
};
```

### Key Properties

| Property | Type | Description |
|----------|------|-------------|
| `name` | `string` | Control name |
| `control_type` | `int` | Type identifier |
| `active` | `bool` | Active state |
| `parent` | `lsControl*` | Parent control |
| `controlmanager` | `lsControlManager*` | Manager reference |
| `m_render` | `lsRender*` | Render context |

### Methods

```cpp
// Activation
void setActive(bool val);
bool getActive() const;

// Navigation
lsControl* getParent() const;
void setParent(lsControl* val);

// Rendering
lsRender* getRender() const;
void setOgl(lsRender* value);
```

---

## lsControlManager Class

The root manager for all controls. Defined in `vortxGE/src/controls/lsControlManager.h`.

### Class Definition

```cpp
class lsControlManager
{
public:
    lsControlManager(lsRender* render);
    virtual ~lsControlManager();

    virtual void Update(double delta);
    virtual void Render();
    virtual void Resize(int x, int y, int width, int height);

    void Add_Control(lsControl* control);
    void Del_Control(lsControl* control);

    lsControl* getControl_list() const;
    lsRender* getRender() const;
};
```

---

## lsViewer Class

A 3D viewport control that manages scenes, passes, and assets. Defined in `vortxGE/src/controls/lsViewer.h`.

### Purpose

- Renders 3D content to a viewport
- Owns scene, pass, and asset managers
- Used for main game view or multiple viewports

### Construction

```cpp
lsViewer(lsRender* render, lsControl* parent);
```

### Owned Managers

| Manager | Description |
|---------|-------------|
| `scenemanager` | Manages scenes |
| `passmanager` | Manages rendering passes |
| `assetmanager` | Manages assets |

### Initialization

```cpp
int Init();

// Example
lsViewer* viewer = new lsViewer(render, parent);
viewer->setName("MainViewer");
viewer->Init();
```

### Access

```cpp
lsSceneManager* getSceneManager() const;
lsPassManager* getPassManager() const;
lsAssetManager* getAssetManager() const;
```

### Usage Example

```cpp
// Create viewer
lsViewer* viewer = new lsViewer(render, controlManager);
viewer->setName("MainViewport");
viewer->Init();

// Get managers
lsSceneManager* sm = viewer->getSceneManager();
lsPassManager* pm = viewer->getPassManager();
lsAssetManager* am = viewer->getAssetManager();

// Create scene
lsScene* scene = new lsScene(render, 1000.0f, 16384);
sm->Add_Scene(scene);

// Create camera
lsCamera* cam = new lsCamera(scene);
scene->Add_Node(cam);

// Create pass
lsPass_DR* pass = new lsPass_DR(render);
pass->setScene(scene);
pass->setCamera(cam);
pm->Add_Pass(pass);
```

---

## lsComposite Class

A control for composite rendering (using FBO textures). Defined in `vortxGE/src/controls/lsComposite.h`.

### Purpose

- Layer multiple renders
- Post-processing effects
- UI composition

### Structure

Same as `lsViewer` - owns scene, pass, and asset managers.

```cpp
lsComposite(lsRender* render, lsControl* parent);
```

### Usage Example

```cpp
// Create composite for post-processing
lsComposite* composite = new lsComposite(render, viewer);
composite->setName("PostProcess");
composite->Init();

// Use FBO pass output as texture
lsPass_FBO* fboPass = new lsPass_FBO(render);
// ... configure FBO pass ...
composite->getPassManager()->Add_Pass(fboPass);

// Render FBO to screen
lsPass_DR* screenPass = new lsPass_DR(render);
screenPass->setScene(composite->getSceneManager()->getScene_list());
// ... configure quad rendering ...
composite->getPassManager()->Add_Pass(screenPass);
```

---

## Control Hierarchy

### Creating a Hierarchy

```cpp
// Root control manager
lsControlManager* cm = engine->getControlManager();

// Create main viewer
lsViewer* mainView = new lsViewer(render, cm);
mainView->setName("MainView");
cm->Add_Control(mainView);

// Create picture-in-picture
lsViewer* miniMap = new lsViewer(render, mainView);
miniMap->setName("MiniMap");
miniMap->setPosition(lsVector2(0.8f, 0.8f));
miniMap->setSize(lsVector2(0.2f, 0.2f));
mainView->Add_Control(miniMap);
```

### Update Flow

```cpp
void lsControlManager::Update(double delta)
{
    lsControl* ctrl = control_list;
    while (ctrl)
    {
        if (ctrl->getActive())
        {
            ctrl->Update(delta);
        }
        ctrl = ctrl->getNext_control();
    }
}
```

---

## Viewport Layout

### Split Screen

```cpp
// Left viewport
lsViewer* left = new lsViewer(render, cm);
left->setName("LeftViewport");
left->setPosition(lsVector2(0.0f, 0.0f));
left->setSize(lsVector2(0.5f, 1.0f));
cm->Add_Control(left);

// Right viewport
lsViewer* right = new lsViewer(render, cm);
right->setName("RightViewport");
right->setPosition(lsVector2(0.5f, 0.0f));
right->setSize(lsVector2(0.5f, 1.0f));
cm->Add_Control(right);
```

### Resize Handling

```cpp
void MyApp::Resize(int width, int height)
{
    // Propagate to all controls
    if (controlManager)
    {
        controlManager->Resize(0, 0, width, height);
    }
}
```

---

## Integration with vortxGE

The engine creates a control manager automatically:

```cpp
vortxGE* engine = new vortxGE();
engine->Init();

lsControlManager* cm = engine->getControlManager();

// Add your controls
lsViewer* viewer = new lsViewer(cm->getRender(), cm);
cm->Add_Control(viewer);

// Main loop
while (!done)
{
    engine->Update(deltaTime);
    engine->Render();
}
```
