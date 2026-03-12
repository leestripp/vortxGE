# Asset Management System

The asset management system in vortxGE provides a unified interface for loading, caching, and managing resources like meshes, textures, and physics shapes. Assets are reference-counted to prevent redundant loading and ensure proper memory management.

---

## Architecture Overview

```
lsAssetManager
    │
    ├── lsAsset (base class)
    │   ├── lsAsset_Mesh (.glb, .vbm)
    │   ├── lsAsset_Texture
    │   ├── lsAsset_Quad
    │   ├── lsAsset_BulletShape_* (physics)
    │   └── lsAsset_MeshGuide_* (debug)
    │
    └── Asset Cache (by name)
```

---

## Asset Types

Defined in `vortxGE/src/assets/lsAsset.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `LSAT_NONE` | 0 | No asset type |
| `LSAT_TEXTURE` | 1 | Texture image |
| `LSAT_MESH` | 2 | 3D mesh model |
| `LSAT_BULLETSHAPE_PLANE` | 3 | Infinite plane collision |
| `LSAT_BULLETSHAPE_BOX` | 4 | Box collision shape |
| `LSAT_BULLETSHAPE_SPHERE` | 5 | Sphere collision shape |
| `LSAT_BULLETSHAPE_PILL` | 6 | Capsule collision shape |
| `LSAT_QUAD` | 7 | Quad geometry |
| `LSAT_BULLETSHAPE_CONVEXHULL` | 8 | Convex hull collision |
| `LSAT_BULLETSHAPE_TRIMESH` | 9 | Triangle mesh collision |
| `LSAT_MESHGUIDE_EMPTY` | 1000 | Empty guide object |
| `LSAT_MESHGUIDE_GRID` | 1001 | Grid guide object |

---

## lsAsset Class

The base class for all assets, defined in `vortxGE/src/assets/lsAsset.h`.

### Class Definition

```cpp
class lsAsset
{
public:
    lsAsset(lsRender* render);
    virtual ~lsAsset();

    // Asset lifecycle
    virtual void AllocAsset() = 0;  // Load/create the asset
    virtual void FreeAsset() = 0;   // Unload/release the asset

    // Reference counting
    void AddUser();
    void RemoveUser();
};
```

### Key Properties

| Property | Type | Description |
|----------|------|-------------|
| `asset_type` | `int` | Type identifier |
| `asset_name` | `string` | Unique name |
| `users` | `unsigned int` | Reference count |
| `data` | `lsData*` | Raw asset data |
| `render` | `lsRender*` | Render context |
| `assetmanager` | `lsAssetManager*` | Owning manager |

### Reference Counting

```cpp
// Increment reference count
void AddUser();

// Decrement reference count
void RemoveUser();

// Get current count
unsigned int getUsers() const;
```

### Navigation

```cpp
// Linked list navigation
lsAsset* getNext_asset() const;
void setNext_asset(lsAsset* val);
lsAsset* getPrev_asset() const;
void setPrev_asset(lsAsset* val);
```

---

## lsAssetManager Class

Manages asset loading and caching. Defined in `vortxGE/src/assets/lsAssetManager.h`.

### Class Definition

```cpp
class lsAssetManager
{
public:
    lsAssetManager(lsRender* render);
    virtual ~lsAssetManager();

    // Find existing asset by name
    lsAsset* Find_Asset(const string& name);
};
```

### Creating Assets

#### Meshes

```cpp
// Load a mesh from file (.glb, .vbm)
lsAsset_Mesh* CreateAsset_Mesh(const string& filename);

// Create a quad asset
lsAsset_Quad* CreateAsset_Quad();

// Example usage
lsAsset_Mesh* cube = assetManager->CreateAsset_Mesh("data/meshes/cube.glb");
```

#### Textures

```cpp
// Load a texture from file
lsAsset_Texture* CreateAsset_Texture(const string& filename);

// Example usage
lsAsset_Texture* tex = assetManager->CreateAsset_Texture("data/textures/brick.png");
```

#### Physics Shapes

```cpp
// Create collision shapes
lsAsset_BulletShape_Plane* CreateAsset_BulletShape_Plane(float x, float y, float z);
lsAsset_BulletShape_Box* CreateAsset_BulletShape_Box(float x, float y, float z);
lsAsset_BulletShape_Pill* CreateAsset_BulletShape_Pill(float radius, float height);
lsAsset_BulletShape_ConvexHull* CreateAsset_BulletShape_ConvexHull(lsAsset_Mesh* meshAsset);
lsAsset_BulletShape_TriMesh* CreateAsset_BulletShape_TriMesh(lsAsset_Mesh* meshAsset);

// Example usage
lsAsset_BulletShape_Box* boxShape = assetManager->CreateAsset_BulletShape_Box(0.5f, 0.5f, 0.5f);
```

#### Debug Guides

```cpp
// Create visualization guides
lsAsset_MeshGuide_Empty* CreateAsset_MeshGuide_Empty();
lsAsset_MeshGuide_Grid* CreateAsset_MeshGuide_Grid();
```

### Asset Access

```cpp
// Get total asset count
unsigned int getAsset_count() const;

// Get asset list head
lsAsset* getAssetList() const;

// Find asset by name
lsAsset* Find_Asset(const string& name);
```

---

## lsAsset_Mesh Class

Represents a 3D mesh asset. Defined in `vortxGE/src/assets/lsAsset_Mesh.h`.

### Supported Formats

- **.glb** - GL Transmission Format (binary)
- **.gltf** - GL Transmission Format (JSON)
- **.vbm** - VortxGE Binary Mesh (legacy)

### Class Definition

```cpp
class lsAsset_Mesh : public lsAsset
{
public:
    lsAsset_Mesh(lsRender* render);
    virtual ~lsAsset_Mesh();

    // Load the mesh file
    virtual void AllocAsset() override;

    // Unload the mesh
    virtual void FreeAsset() override;

    // File name
    const string& getFilename() const;
    void setFilename(const string& val);
};
```

### Usage Example

```cpp
// Load mesh
lsAsset_Mesh* meshAsset = assetManager->CreateAsset_Mesh("data/meshes/character.glb");

// Assign to object
lsObject* object = new lsObject(scene);
object->Set_Asset(meshAsset);
```

---

## lsAsset_Texture Class

Represents a 2D texture asset. Defined in `vortxGE/src/assets/lsAsset_Texture.h`.

### Supported Formats

- PNG
- JPG/JPEG
- TGA
- BMP

### Key Features

- Automatic mipmap generation
- Anisotropic filtering support
- Repeat/clamp wrapping modes

---

## lsAsset_Quad Class

Represents a quad (2 triangles) for full-screen effects or UI. Defined in `vortxGE/src/assets/lsAsset_Quad.h`.

### Usage

```cpp
// Create quad for post-processing
lsAsset_Quad* quad = assetManager->CreateAsset_Quad();

// Use in a screen quad object
lsObject* screenQuad = new lsObject(scene);
screenQuad->Set_Asset(quad);
```

---

## Physics Shape Assets

### lsAsset_BulletShape_Plane

An infinite plane collision shape.

```cpp
lsAsset_BulletShape_Plane* shape = 
    assetManager->CreateAsset_BulletShape_Plane(0.0f, 1.0f, 0.0f);  // Normal (up)
```

### lsAsset_BulletShape_Box

A box collision shape.

```cpp
lsAsset_BulletShape_Box* shape = 
    assetManager->CreateAsset_BulletShape_Box(1.0f, 2.0f, 1.0f);  // Half extents
```

### lsAsset_BulletShape_Pill

A capsule (pill) collision shape.

```cpp
lsAsset_BulletShape_Pill* shape = 
    assetManager->CreateAsset_BulletShape_Pill(0.5f, 1.0f);  // Radius, cylinder height
```

### lsAsset_BulletShape_ConvexHull

A convex hull generated from mesh vertices. Good for complex dynamic objects.

```cpp
lsAsset_BulletShape_ConvexHull* shape = 
    assetManager->CreateAsset_BulletShape_ConvexHull(meshAsset);
```

### lsAsset_BulletShape_TriMesh

A triangle mesh collision shape. Only suitable for static objects (terrain, buildings).

```cpp
lsAsset_BulletShape_TriMesh* shape = 
    assetManager->CreateAsset_BulletShape_TriMesh(meshAsset);  // Must be static!
```

---

## Debug Guide Assets

### lsAsset_MeshGuide_Empty

An empty placeholder object useful for attaching child nodes.

```cpp
lsAsset_MeshGuide_Empty* guide = assetManager->CreateAsset_MeshGuide_Empty();
```

### lsAsset_MeshGuide_Grid

A grid visualization for debugging floor placement.

```cpp
lsAsset_MeshGuide_Grid* grid = assetManager->CreateAsset_MeshGuide_Grid();
```

---

## Asset Loading Pattern

The recommended pattern for loading assets:

```cpp
// 1. Get or create asset manager
lsAssetManager* assetMgr = scene->getAssetManager();

// 2. Check if already loaded (caching)
lsAsset* existing = assetMgr->Find_Asset("myMesh");
if (existing)
{
    // Use cached asset
    object->Set_Asset(existing);
    existing->AddUser();
}
else
{
    // Load new asset
    lsAsset_Mesh* mesh = assetMgr->CreateAsset_Mesh("data/meshes/model.glb");
    object->Set_Asset(mesh);
}

// 3. When done with object, release reference
object->getAsset()->RemoveUser();
```

---

## Asset Manager Hierarchy

The engine uses multiple asset managers:

1. **Global Asset Manager** - Created by `vortxGE`, accessible via `getAssetManager()`
2. **Scene Asset Manager** - Owned by each `lsScene`
3. **Viewer/Composite Asset Manager** - Owned by each control

```cpp
// Global assets
vortxGE* engine = new vortxGE();
lsAssetManager* globalAssets = engine->getAssetManager();

// Scene-specific assets
lsScene* scene = new lsScene(render, 1000.0f, 16384);
lsAssetManager* sceneAssets = scene->getAssetManager();
```

---

## Memory Management

Assets use reference counting:

1. When created, `users = 0`
2. When assigned to an object, `AddUser()` is called
3. When an object is deleted, `RemoveUser()` is called
4. When `users` reaches 0, the asset can be unloaded

This ensures assets are kept in memory while being used and unloaded when no longer needed.
