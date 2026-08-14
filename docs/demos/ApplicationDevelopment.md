# Creating Applications with vortxGE

This guide covers how to create applications using the vortxGE engine, including window setup, scene creation, and the main loop.

---

> **Render path (since 2026-08-14, Plan.md E3):** apps built on the
> demos get the stage-based pipeline by default — no code needed. The
> legacy `lsPassManager` chain is deprecated (`VORTX_LEGACY_PIPELINE=1`
> escape for one release). Custom rendering = new `lsRenderStage`
> subclass added to the pipeline; the scene owns the active camera
> (`lsScene::setActiveCamera`), so stages never touch control classes.

## Application Architecture

### Basic Structure

```cpp
#include <vortxGE.h>

int main()
{
    // 1. Create engine instance
    vortxGE* engine = new vortxGE();

    // 2. Create window and render context
    //    OpenGL: create a GL context (shown below).
    //    Vulkan: create a GLFW_NO_API window instead — call
    //    vortxGE::PreferredRendererType() BEFORE window creation; it probes
    //    Vulkan first so you create the correct window type, with automatic
    //    OpenGL fallback if Vulkan init fails.
    // (Using GLFW3, GTKmm, or custom)

    // 3. Initialize engine
    engine->Init();

    // 4. Setup your scene
    // ...

    // 5. Main loop
    while (!shouldClose)
    {
        // Update
        engine->Update(deltaTime);

        // Render
        engine->Render();

        // Swap buffers
    }

    // 6. Cleanup
    engine->Cleanup();
    delete engine;

    return 0;
}
```

---

## Window Setup (GLFW3)

The GLFW3 demo shows the recommended approach for setting up a window with vortxGE.

### Initialization Steps

```cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Error callback
void error_callback(int error, const char* description)
{
    cerr << "ERROR: " << description << endl;
}

// Initialize GLFW
glfwSetErrorCallback(error_callback);
if (!glfwInit())
{
    return false;
}

// Configure OpenGL context
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// Create window
GLFWwindow* window = glfwCreateWindow(1920, 1080, "My App", NULL, NULL);
if (!window)
{
    glfwTerminate();
    return false;
}

// Set callbacks
glfwSetKeyCallback(window, key_callback);
glfwSetCursorPosCallback(window, cursor_position_callback);
glfwSetMouseButtonCallback(window, mouse_button_callback);
glfwSetWindowSizeCallback(window, resize_callback);

// Make OpenGL context current
glfwMakeContextCurrent(window);

// Initialize GLEW
glewExperimental = GL_TRUE;
GLenum err = glewInit();
if (err != GLEW_OK)
{
    cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
    return false;
}

// Enable vSync
glfwSwapInterval(1);
```

### Input Handling

```cpp
// Keyboard callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_W: key_w = pressed; break;
        case GLFW_KEY_A: key_a = pressed; break;
        case GLFW_KEY_S: key_s = pressed; break;
        case GLFW_KEY_D: key_d = pressed; break;
    }
}

// Mouse position callback
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    // Handle mouse movement
}

// Mouse button callback
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Left click
    }
}

// Window resize callback
static void resize_callback(GLFWwindow* window, int width, int height)
{
    engine->Resize(0, 0, width, height);
}
```

---

## Scene Setup

The demo uses a `DemoScene` class that sets up the complete scene:

### DemoScene Class

```cpp
#include <vortxGE.h>

class DemoScene
{
public:
    DemoScene();
    ~DemoScene();

    int Init();
    void Update(double delta);
    void Render();
    void Resize(int x, int y, int width, int height);

    // Player control
    void rotatePlayerView(double dx, double dy);
    void movePlayer(bool forward, bool backward, bool left, bool right,
                    bool jump, bool sprint, bool crouch);
    void handleMouseButtons(bool left, bool right);

private:
    vortxGE* engine;
    lsScene* scene;
    lsFirstPersonRig* player;
    // ...
};
```

### Initialization

```cpp
int DemoScene::Init()
{
    // Create engine
    engine = new vortxGE();
    if (!engine)
        return 1;

    // Initialize (creates managers)
    if (engine->Init())
        return 1;

    // Get control manager
    lsControlManager* cm = engine->getControlManager();

    // Create viewer (3D viewport)
    lsViewer* viewer = new lsViewer(cm->getRender(), cm);
    viewer->setName("MainViewer");
    cm->Add_Control(viewer);

    // Create scene
    scene = new lsScene(render, 1000.0f, 16384);
    scene->setName("MainScene");

    // Initialize scene
    scene->getBulletworld()->Init();

    // Create camera
    lsCamera* cam = new lsCamera(scene);
    cam->setName("MainCamera");
    cam->Set_Perspective(60.0f, 1.778f, 0.1f, 1000.0f);
    scene->Add_Node(cam);

    // Create viewer pass
    lsPassManager* pm = viewer->getPassManager();
    lsPass_DR* pass = new lsPass_DR(render);
    pass->setScene(scene);
    pass->setCamera(cam);
    pass->setBackground_colour(lsVector4(0.1f, 0.1f, 0.2f, 1.0f));
    pm->Add_Pass(pass);

    // Create first-person rig
    player = new lsFirstPersonRig(scene, scene->getBulletworld()->getDynamicsWorld());
    player->Set_Pos(0.0f, 5.0f, 0.0f);
    scene->Add_Node(player);

    // Add scene to viewer
    viewer->getSceneManager()->Add_Scene(scene);

    return 0;
}
```

---

## Main Loop

```cpp
void glfwApp::Run()
{
    lsTimer timer;
    timer.start_Timer();

    double delta = 1000.0 / 60.0;
    running = true;

    while (running)
    {
        // Check for window close
        if (glfwWindowShouldClose(m_window))
        {
            running = false;
        }

        // Handle input
        if (m_fullscreen)
        {
            // Mouse look
            demoscene->rotatePlayerView(pos_x - old_x, pos_y - old_y);

            // Movement
            demoscene->movePlayer(key_w, key_s, key_a, key_d,
                                  key_jump, key_sprint, key_crouch);

            // Mouse buttons
            demoscene->handleMouseButtons(m_mouse_button_left, m_mouse_button_right);
        }

        // Update engine
        demoscene->Update(delta);

        // Render
        demoscene->Render();

        // Swap buffers
        glfwSwapBuffers(m_window);
        glfwPollEvents();

        // Calculate delta time
        delta = timer.get_ElapsedTime();
        timer.start_Timer();
    }

    // Cleanup
    if (demoscene) delete demoscene;
    if (m_window) glfwDestroyWindow(m_window);
    glfwTerminate();
}
```

---

## Creating Objects

### Adding a Mesh

```cpp
// Load mesh asset
lsAsset_Mesh* mesh = scene->getAssetManager()->CreateAsset_Mesh("data/meshes/cube.glb");

// Create object
lsObject* obj = new lsObject(scene);
obj->setName("Cube");
obj->Set_Asset(mesh);
obj->Set_Pos(0.0f, 1.0f, 0.0f);
obj->Set_Scale(1.0f, 1.0f, 1.0f);

// Add to scene
scene->Add_Node(obj);
```

### Adding a Light

```cpp
// Create spot light
lsLight_Spot* light = new lsLight_Spot(scene);
light->setName("SpotLight");
light->Set_Pos(10.0f, 15.0f, 0.0f);
light->Set_Rot(-45.0f, 0.0f, 0.0f);
light->diffuse = lsVector4(1.0f, 0.9f, 0.8f, 1.0f);
light->setIntensity(2.0f);
light->setHas_shadowmap(true);

// Add to scene
scene->Add_Node(light);
```

### Adding Physics

```cpp
// Create box collision shape
btBoxShape* shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));

// Create physics object
lsBulletObject* bullet = new lsBulletObject(obj, shape, 1024);
bullet->Init(5.0f);  // 5kg mass

// Attach to object
obj->setBulletObject(bullet);
```

---

## Loading Scenes

```cpp
// Load from .vgs file
lsSceneManager* sm = viewer->getSceneManager();
sm->LoadSceneFromFile("data/demoscene.vgs", scene);
```

---

## Full Example

```cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vortxGE.h>

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "vortxGE App", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewInit();

    // Create engine
    vortxGE* engine = new vortxGE();
    engine->Init();

    // Setup basic viewer
    lsControlManager* cm = engine->getControlManager();
    lsViewer* viewer = new lsViewer(cm->getRender(), cm);
    cm->Add_Control(viewer);

    lsRender* render = cm->getRender();

    // Create scene
    lsScene* scene = new lsScene(render, 1000.0f, 16384);
    scene->getBulletworld()->Init();

    // Create camera
    lsCamera* cam = new lsCamera(scene);
    cam->Set_Perspective(60.0f, 1.778f, 0.1f, 1000.0f);
    cam->Set_Pos(0.0f, 5.0f, 10.0f);
    scene->Add_Node(cam);

    // Setup pass
    lsPass_DR* pass = new lsPass_DR(render);
    pass->setScene(scene);
    pass->setCamera(cam);
    viewer->getPassManager()->Add_Pass(pass);

    viewer->getSceneManager()->Add_Scene(scene);

    // Main loop
    lsTimer timer;
    timer.start_Timer();

    while (!glfwWindowShouldClose(window))
    {
        double delta = timer.get_ElapsedTime();
        timer.start_Timer();

        engine->Update(delta);
        engine->Render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    engine->Cleanup();
    delete engine;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
```

---

## Building Your Application

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.29)
project(MyApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# Find vortxGE
find_package(vortxGE REQUIRED)

# Your source files
set(SOURCES main.cpp)

add_executable(MyApp ${SOURCES})

target_link_libraries(MyApp vortxGE)
```

---

## Keyboard Controls (First Person Demo)

| Key | Action |
|-----|--------|
| W/A/S/D | Move forward/left/back/right |
| Mouse | Look around |
| Space | Jump |
| Left Shift | Sprint |
| Left Control | Crouch |
| F1 | Toggle fullscreen |
| Escape | Exit |
