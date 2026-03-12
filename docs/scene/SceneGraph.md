# Scene Graph System

The scene graph is the hierarchical data structure that organizes all visual and logical elements in the vortxGE engine. It provides a parent-child relationship system for transforms, enabling complex scenes with nested objects.

## Coordinate System

vortxGE uses a **Right-Handed Coordinate System** with the view direction along the **-Z axis**:

- **+X**: Right
- **+Y**: Up  
- **-Z**: Forward (View Direction)

---

## Node Types

The engine defines various node types in `vortxGE/src/scene/lsNode.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `LSNT_NODE` | 0 | Base node type |
| `LSNT_OBJECT` | 1 | Renderable object |
| `LSNT_TERRAIN` | 2 | Terrain object |
| `LSNT_CAMERA` | 4 | Camera node |
| `LSNT_SCENE` | 5 | Scene root node |
| `LSNT_MOTION` | 6 | Motion path node |
| `LSNT_VEHICLE` | 7 | Vehicle controller |
| `LSNT_FIRSTPERSONRIG` | 8 | First-person character |
| `LSNT_ZONE` | 9 | Zone/area node |
| `LSNT_DEBUG` | 10 | Debug visualization |
| `LSNT_ACTOR` | 11 | Actor/character |
| `LSNT_LIGHT` | 12 | Light source |
| `LSNT_EMPTY` | 13 | Empty/placeholder node |
| `LSNT_OBJECT_LOD` | 14 | LOD-enabled object |
| `LSNT_OBJECT_EMITTER` | 15 | Object spawner |
| `LSNT_PARTICLE_EMITTER` | 16 | Particle emitter |
| `LSNT_PARTICLE` | 17 | Particle |
| `LSNT_ANIMNODE` | 18 | Animatable node |
| `LSNT_TRIGGER` | 100 | Trigger volume |
| `LSNT_PICKUP` | 101 | Pickup item |

---

## lsNode Class

The base class for all scene graph elements, defined in `vortxGE/src/scene/lsNode.h`.

### Class Hierarchy

```
lsNode
├── lsScene (LSNT_SCENE)
├── lsAnimNode (LSNT_ANIMNODE)
│   ├── lsObject (LSNT_OBJECT)
│   │   ├── lsCamera (LSNT_CAMERA)
│   │   ├── lsLight (LSNT_LIGHT)
│   │   │   ├── lsLight_Point
│   │   │   ├── lsLight_Spot
│   │   │   └── lsLight_Direct
│   │   ├── lsFirstPersonRig (LSNT_FIRSTPERSONRIG)
│   │   └── lsObjectEmitter (LSNT_OBJECT_EMITTER)
```

### Public Data Members

| Member | Type | Description |
|--------|------|-------------|
| `ModelMatrix` | `lsMatrix4x4` | Final world transformation matrix |
| `position` | `lsVector3` | Local position (x, y, z) |
| `rotation` | `lsVector3` | Local rotation as Euler angles (degrees) |
| `scale` | `lsVector3` | Local scale (x, y, z) |

### Key Methods

#### Lifecycle

```cpp
// Constructor - parent can be nullptr for root nodes
lsNode(lsNode* parent);

// Override in subclasses to implement custom behavior
virtual void Move(lsScene* scn, lsRender* render, double delta);
virtual void Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render);
virtual void Draw(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render);
virtual void InitDrawlist();
```

#### Node Hierarchy

```cpp
// Add a child node (takes ownership)
void Add_Node(lsNode* node);

// Remove a child node
void Del_Node(lsNode* node);

// Find a node by name in the hierarchy
void Find_Node(lsScene* scn, const string& name);

// Remove all children
void Del_Children();

// Get children vector
vector<unique_ptr<lsNode>>& getChildren();
```

#### Transformations

```cpp
// Set local transform
void Set_Pos(float x, float y, float z);
void Set_Rot(float x, float y, float z);  // Euler angles in degrees
void Set_Scale(float x, float y, float z);

// Apply world-space rotation
void RotateWorld(float angle, float x, float y, float z);
```

#### Lifecycle Management

```cpp
// Activation
void SetActive(bool active);
bool GetActive() const;

// Deletion
void MarkForDeletion();
bool IsMarkedForDeletion() const;
void CleanupDeletedNodes();

// Life timer
void Set_Life(double val);
void checkLife(lsScene* scn);
```

#### Navigation

```cpp
// Parent access
lsNode* getParent() const;
void setParent(lsNode* val);

// Name
const string& getName() const;
void setName(const string& val);

// Node type
int getNode_type() const;
void setNode_type(int val);
```

---

## lsScene Class

The root node of a scene graph, defined in `vortxGE/src/scene/lsScene.h`. It owns the physics world, event manager, and asset manager.

### Construction

```cpp
lsScene(lsRender* render, float world_size, int maxProxies);
```

### Key Methods

```cpp
// Clear all objects from the scene
void ClearScene();

// Add object to render queue
void Add_DrawObject(lsObject* object);

// Event handling
void Add_Event(lsEvent* event);
lsEvent* Poll_Event();

// Get associated scene (returns self)
lsScene* Get_Scene();

// Get event manager
lsEventManager* Get_EventManager();
```

### Owned Components

| Member | Type | Description |
|--------|------|-------------|
| `draw_list` | `lsObject*` | Linked list of objects to render |
| `bulletworld` | `lsBulletWorld*` | Physics simulation world |
| `m_assetManager` | `lsAssetManager*` | Scene-specific asset loader |
| `m_lightManager` | `lsLightManager*` | Global light management |
| `sky_object` | `lsObject*` | Skybox object |
| `m_listener` | `lsListener*` | Audio listener |

---

## lsObject Class

A renderable scene node that holds a mesh asset, light manager, and optional physics body. Defined in `vortxGE/src/scene/lsObject.h`.

### Class Definition

```cpp
class lsObject : public lsAnimNode
{
public:
    lsObject(lsNode* parent);
    virtual ~lsObject();
    
    // Rendering pipeline methods
    virtual void Move(lsScene* scn, lsRender* render, double delta);
    virtual void Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render);
    virtual void Draw(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render);
};
```

### Components

| Member | Type | Description |
|--------|------|-------------|
| `asset` | `lsAsset*` | Pointer to mesh asset |
| `lightmanager` | `lsLightManager*` | Lights affecting this object |
| `bulletobject` | `lsBulletObject*` | Physics body (optional) |

### Shadow Properties

```cpp
// Control shadow casting/receiving
bool getCasts_shadows() const;
void setCasts_shadows(bool val);
bool getReceives_shadows() const;
void setReceives_shadows(bool val);
```

### Occlusion Culling

```cpp
// Occlusion query results
bool getOccluded() const;
void setOccluded(bool val);
float getDistance_to_camera() const;
void setDistance_to_camera(float val);
int getOcclusion_type() const;  // LSOT_NONE, LSOT_OCCLUDER
void setOcclusion_type(int val);
```

### Mesh Access

```cpp
// Get the rendered mesh
lsMesh* Get_Mesh();

// Set mesh asset
void Set_Asset(lsAsset* asset);
lsAsset* getAsset() const;
```

---

## lsCamera Class

A viewpoint node that defines how the scene is rendered. Defined in `vortxGE/src/scene/lsCamera.h`.

### Projection Modes

| Constant | Value | Description |
|----------|-------|-------------|
| `LSCM_PERSPECTIVE` | 0 | Perspective projection (default) |
| `LSCM_ORTHOGRAPHIC` | 1 | Orthographic projection |

### Public Data Members

| Member | Type | Description |
|--------|------|-------------|
| `ViewMatrix` | `lsMatrix4x4` | Camera view matrix |
| `frustum` | `lsFrustum` | View frustum for culling |
| `cam_fov` | `float` | Field of view (degrees) |
| `cam_ratio` | `float` | Aspect ratio (width/height) |
| `cam_near` | `float` | Near clipping plane |
| `cam_far` | `float` | Far clipping plane |

### Key Methods

```cpp
// Set perspective projection
void Set_Perspective(float view_angle, float width_ratio, float nplane, float fplane);

// Set aspect ratio
void Set_ViewRatio(float vr);

// Set orthographic projection (box)
void Set_Orthographic(float cube);

// Set orthographic projection (custom bounds)
void Set_Orthographic(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);

// Update view matrix
void Update_Camera();
```

### Usage Example

```cpp
// Create a camera
lsCamera* camera = new lsCamera(scene);
camera->setName("MainCamera");
camera->Set_Perspective(60.0f, 1.778f, 0.1f, 1000.0f);
camera->Set_Pos(0.0f, 5.0f, 10.0f);
```

---

## lsLight Class

A light source node that extends `lsCamera` (since lights need position and orientation for shadow mapping). Defined in `vortxGE/src/scene/lsLight.h`.

### Light Types

| Constant | Value | Description |
|----------|-------|-------------|
| `LSLT_NONE` | 0 | No light |
| `LSLT_POINT` | 1 | Point light (omnidirectional) |
| `LSLT_SPOT` | 2 | Spot light (cone) |
| `LSLT_DIRECT` | 3 | Directional light (sun) |

### Public Data Members

| Member | Type | Description |
|--------|------|-------------|
| `ambient` | `lsVector4` | Ambient color (r, g, b, intensity) |
| `diffuse` | `lsVector4` | Diffuse color (r, g, b, intensity) |
| `specular` | `lsVector4` | Specular color (r, g, b, intensity) |
| `intensity` | `float` | Light intensity multiplier |
| `spot_cutoff` | `float` | Spot light cutoff angle (degrees) |
| `spot_outer_cutoff` | `float` | Spot light outer cutoff for soft edges |
| `ShadowmapMatrix` | `lsMatrix4x4` | Light space matrix for shadow mapping |

### Shadow Mapping

```cpp
// Shadow map texture
GLuint getTex_shadowmap() const;
void setTex_shadowmap(GLuint val);
bool getHas_shadowmap() const;
void setHas_shadowmap(bool val);

// Shadow bias to prevent shadow acne
float getShadow_bias() const;
void setShadow_bias(float val);

// Shadow map resolution
float getPixel_x() const;
void setPixel_x(float val);
float getPixel_y() const;
void setPixel_y(float val);

// Build shadow matrix
void buildShadowmap_matrix();
```

### Usage Example

```cpp
// Create a spot light
lsLight_Spot* spot = new lsLight_Spot(scene);
spot->setName("SpotLight");
spot->Set_Pos(10.0f, 15.0f, 0.0f);
spot->Set_Rot(-45.0f, 0.0f, 0.0f);  // Point down at 45 degrees
spot->diffuse = lsVector4(1.0f, 0.9f, 0.8f, 1.0f);
spot->setIntensity(2.0f);
spot->setHas_shadowmap(true);
```

---

## lsLightManager Class

Manages a list of lights that affect an object. Defined in `vortxGE/src/scene/lsLightManager.h`.

### Key Methods

```cpp
// Add/remove lights
void Add_Light(lsLight* light);
void Del_Light(lsLight* light);

// Find light by name
lsLight* Find_Light(const string& name);

// Get light count
int getLight_count() const;

// Get light at index
lsLight* Get_Light(int index);

// Clear all lights
void Clear();
```

---

## lsAnimNode Class

An animatable node that supports keyframe-based animations. Defined in `vortxGE/src/scene/lsAnimNode.h`.

### Key Features

- Owns an `lsAnimationManager` for playback
- Supports animated properties (position, rotation, scale)
- Can animate material properties

### Key Methods

```cpp
// Initialize animation system
void Init_Animation(double fps);

// Animation playback
void Play_Animation(bool loop);
void Stop_Animation();
void Update_Animation(double delta);
```

---

## lsFirstPersonRig Class

A first-person character controller combining physics and camera. See [FirstPersonRig.md](./FirstPersonRig.md) for complete documentation.

---

## lsObjectEmitter Class

A node that periodically spawns new objects with physics properties. Defined in `vortxGE/src/scene/lsObjectEmitter.h`.

### Key Features

- Spawns objects at specified intervals
- Supports randomized positions and velocities
- Can use object pools for performance

---

## Scene Update Flow

The scene graph is updated through a multi-phase system:

### Phase 1: Move

```cpp
// Called on each node to update transforms, physics, and animations
void lsNode::Move(lsScene* scn, lsRender* render, double delta);
```

### Phase 2: Optimize

```cpp
// Frustum culling and draw list preparation
void lsNode::Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render);
```

### Phase 3: Draw

```cpp
// Rendering to the current pass
void lsNode::Draw(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render);
```

### Threading

Nodes support threaded updates:

```cpp
// Multi-threaded versions
void MoveThread(lsScene* scn, lsRender* render, double delta);
void OptimizeThread(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render);
```

---

## Adding Objects to Scene

```cpp
// Create a scene
lsScene* scene = new lsScene(render, 1000.0f, 16384);

// Create an object
lsObject* cube = new lsObject(scene);
cube->setName("MyCube");
cube->Set_Pos(0.0f, 1.0f, 0.0f);
cube->Set_Scale(1.0f, 1.0f, 1.0f);

// Load and assign a mesh
lsAsset* meshAsset = scene->getAssetManager()->Load_Asset("data/meshes/cube.glb");
cube->Set_Asset(meshAsset);

// Add to scene hierarchy
scene->Add_Node(cube);
```
