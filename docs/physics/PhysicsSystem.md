# Physics System

vortxGE integrates with the Bullet Physics library to provide real-time physics simulation. This document covers the physics system architecture, collision shapes, and usage patterns.

---

## Architecture Overview

```
lsScene
    │
    └── lsBulletWorld (physics simulation)
            │
            └── btDiscreteDynamicsWorld (Bullet)
                    │
                    └── lsBulletObject (per object)
                            │
                            └── btRigidBody
```

---

## lsBulletWorld Class

Manages the physics simulation world. Defined in `vortxGE/src/physics/lsBulletWorld.h`.

### Construction

```cpp
lsBulletWorld(lsScene* scn);
```

### Initialization

```cpp
void Init();
```

The `Init()` method sets up:
- Collision configuration
- Collision dispatcher
- Broadphase collision detection (AxisSweep3)
- Constraint solver
- Gravity (default: -19.6 m/s², or -9.8*2.0)
- Internal tick callback for collision events
- Collision filter callback

### Physics Stepping

```cpp
void BulletStep(int steps, double delta);
```

- `steps`: Number of simulation sub-steps
- `delta`: Time delta in milliseconds

```cpp
// Example: Step physics
world->BulletStep(3, deltaTime);
```

### Collision Events

The world provides a tick callback for processing collision events:

```cpp
void lsTickCallback(btDynamicsWorld* world, btScalar timeStep);
void lsProcessCallback(btScalar timeStep);
```

---

## lsBulletObject Class

Attaches physics to an `lsObject`. Defined in `vortxGE/src/physics/lsBulletObject.h`.

### Motion Types

| Constant | Value | Description |
|----------|-------|-------------|
| `LSBOT_STATIC` | 0 | Immovable (mass = 0) |
| `LSBOT_DYNAMIC` | 1 | Affected by forces |
| `LSBOT_KINEMATIC` | 2 | Moved by code, affects dynamic |

### Construction

```cpp
lsBulletObject(lsObject* obj, btCollisionShape* bs, int maxProxies);
```

- `obj`: The lsObject to attach physics to
- `bs`: The collision shape
- `maxProxies`: Broadphase proxy count

### Initialization

```cpp
void Init(float basemass);
```

- `basemass`: Mass in kilograms (0 for static)

```cpp
// Dynamic object (5kg)
bulletObj->Init(5.0f);

// Static object (0 mass)
bulletObj->Init(0.0f);
```

### Key Methods

```cpp
// Apply physics simulation to object transform
void Apply_Physics(lsObject* obj);

// Reset object to initial physics state
void Reset_Physics(lsObject* obj);

// Remove/add from simulation
void RemoveFromSim();
void AddToSim();
```

### Transform Control

```cpp
// Set position
void Set_Position(float x, float y, float z);

// Set rotation (Euler angles)
void Set_Rotation(float x, float y, float z);

// Set gravity vector
void Set_Gravity(float x, float y, float z);

// Set linear velocity
void Set_Velocity(float x, float y, float z);
```

---

## Collision Shapes

### Box

```cpp
// Create box collision shape
btBoxShape* box = new btBoxShape(btVector3(halfExtentsX, halfExtentsY, halfExtentsZ));

// Attach to object
lsBulletObject* bullet = new lsBulletObject(obj, box, 1024);
bullet->Init(5.0f);  // 5kg mass
```

### Sphere

```cpp
btSphereShape* sphere = new btSphereShape(radius);
```

### Capsule (Pill)

```cpp
// Radius of capsule ends, height of cylindrical portion
btCapsuleShape* pill = new btCapsuleShape(radius, height);
```

### Plane

```cpp
// Infinite plane with normal
btStaticPlaneShape* plane = new btStaticPlaneShape(btVector3(normalX, normalY, normalZ), planeConstant);
```

### Convex Hull

```cpp
// Generate from mesh vertices
btConvexHullShape* convex = new btConvexHullShape();
for (auto& vertex : meshVertices)
{
    convex->addPoint(btVector3(vertex.x, vertex.y, vertex.z));
}
```

### Triangle Mesh (Static Only)

```cpp
// For static terrain/architecture
btBvhTriangleMeshShape* trimesh = new btBvhTriangleMeshShape(meshInterface, useAabbHeap, buildBvh);
```

---

## Collision Filtering

### Filter Groups

Use `lsFilterCallback` to customize collision filtering:

```cpp
// lsFilterCallback.h
class lsFilterCallback : public btOverlapFilterCallback
{
public:
    virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const;
};
```

### Default Groups

Define custom groups using bitmasks:

```cpp
// Example collision groups
const int GROUP_DEFAULT = 1;
const int GROUP_CHARACTER = 2;
const int GROUP_SCENERY = 4;
const int GROUP_PROJECTILE = 8;
```

---

## Usage Patterns

### Creating a Physics Scene

```cpp
// 1. Create scene
lsScene* scene = new lsScene(render, 1000.0f, 16384);

// 2. Initialize physics
scene->getBulletworld()->Init();

// 3. Add static ground
lsObject* ground = new lsObject(scene);
ground->Set_Asset(groundMesh);
ground->Set_Scale(50.0f, 1.0f, 50.0f);
scene->Add_Node(ground);

// Create static physics
btStaticPlaneShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
lsBulletObject* groundBullet = new lsBulletObject(ground, groundShape, 0);
groundBullet->Init(0.0f);  // 0 = static
ground->setBulletObject(groundBullet);
```

### Creating a Dynamic Object

```cpp
// Create object
lsObject* box = new lsObject(scene);
box->Set_Asset(cubeMesh);
box->Set_Pos(0.0f, 10.0f, 0.0f);
scene->Add_Node(box);

// Create physics
btBoxShape* boxShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
lsBulletObject* boxBullet = new lsBulletObject(box, boxShape, 1024);
boxBullet->Init(5.0f);  // 5kg dynamic object
box->setBulletObject(boxBullet);
```

### Using Convex Hull

```cpp
// Load mesh asset
lsAsset_Mesh* meshAsset = assetManager->CreateAsset_Mesh("data/meshes/complex.glb");

// Create convex hull from mesh
lsAsset_BulletShape_ConvexHull* shapeAsset = 
    assetManager->CreateAsset_BulletShape_ConvexHull(meshAsset);

// Get Bullet shape and create physics
btConvexHullShape* shape = static_cast<btConvexHullShape*>(shapeAsset->getShape());
lsBulletObject* bullet = new lsBulletObject(obj, shape, 1024);
bullet->Init(10.0f);
```

---

## Physics Update

Physics is stepped in the scene's `Move()` method:

```cpp
void lsScene::Move(lsScene* scn, lsRender* render, double delta)
{
    // Step physics
    if (bulletworld)
    {
        bulletworld->BulletStep(3, delta);
    }

    // Update objects from physics
    // ...
}
```

---

## Collision Events

Collision events are reported through the event system:

```cpp
// In lsBulletWorld::lsProcessCallback()
// For each contact manifold:
lsEvent_HitPoint* e = new lsEvent_HitPoint();
e->Set_HitNode(obj_A);
e->Set_HitPoint(ptB.getX(), ptB.getY(), ptB.getZ());
e->Set_HitNormal(normalOnB.getX(), normalOnB.getY(), normalOnB.getZ());
obj_B->Get_EventManager()->Add_Event(e);
```

Handle events:

```cpp
void MyGame::HandleEvent(lsEvent* event)
{
    if (event->getEvent_type() == LSEV_HITPOINT)
    {
        lsEvent_HitPoint* hit = static_cast<lsEvent_HitPoint*>(event);
        lsNode* hitNode = hit->Get_HitNode();
        lsVector3 hitPoint = hit->Get_HitPoint();
        // Handle collision...
    }
}
```

---

## First Person Rig Physics

The `lsFirstPersonRig` uses physics for character movement:

- Kinematic character controller
- Capsule collision shape
- Ground detection via raycasting

See [FirstPersonRig.md](./FirstPersonRig.md) for details.

---

## Performance Tips

1. **Broadphase Proxies**: Set appropriate `maxProxies` for scene complexity
2. **Static vs Dynamic**: Use static for terrain, dynamic only when needed
3. **Convex Hull**: Use for complex objects, prefer primitives when possible
4. **Sleep Thresholds**: Bullet puts inactive objects to sleep for performance
5. **Step Count**: Don't step physics more than necessary (3 sub-steps is usually sufficient)
