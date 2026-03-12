# Event System

The event system provides a mechanism for deferred actions and communication between game systems. Events are queued and processed in a separate pass, allowing safe modification of scene elements during simulation.

---

## Event Types

Defined in `vortxGE/src/events/lsEvent.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `LSET_NULL` | 0 | No event |
| `LSET_DEL_NODE` | 1 | Node deletion |
| `LSET_HIT_POINT` | 2 | Physics collision |

---

## lsEvent Class

Base class for all events. Defined in `vortxGE/src/events/lsEvent.h`.

### Class Definition

```cpp
class lsEvent
{
public:
    lsEvent();
    virtual ~lsEvent();

    // Process the event
    virtual void ProcessEvent();

    // Accessors
    lsNode* getNode() const;
    void setNode(lsNode* val);

    lsEventManager* getEventmanager() const;
    void setEventManager(lsEventManager* val);

    int getEvent_type() const;
    void setEvent_type(int val);
};
```

---

## lsEventManager Class

Manages event queue. Defined in `vortxGE/src/events/lsEventManager.h`.

### Class Definition

```cpp
class lsEventManager
{
public:
    lsEventManager();
    ~lsEventManager();

    // Event queue management
    void Add_Event(lsEvent* event);
    lsEvent* Poll_Event();

    // Process all events
    void Process_Queue();
};
```

### Usage

```cpp
// Add event to queue
lsEvent* event = new lsEvent_DelNode(myNode);
eventManager->Add_Event(event);

// Process events (usually in scene update)
eventManager->Process_Queue();
```

---

## Event Types

### lsEvent_DelNode

Schedules a node for deletion. Defined in `vortxGE/src/events/lsEvent_DelNode.h`.

```cpp
class lsEvent_DelNode : public lsEvent
{
public:
    lsEvent_DelNode();
    virtual ~lsEvent_DelNode();

    virtual void ProcessEvent() override;
};
```

### lsEvent_HitPoint

Reports physics collision points. Defined in `vortxGE/src/events/lsEvent_HitPoint.h`.

```cpp
class lsEvent_HitPoint : public lsEvent
{
public:
    lsEvent_HitPoint();
    virtual ~lsEvent_HitPoint();

    virtual void ProcessEvent() override;

    // Hit information
    void Set_HitNode(lsNode* node);
    lsNode* Get_HitNode() const;

    void Set_HitPoint(float x, float y, float z);
    lsVector3 Get_HitPoint() const;

    void Set_HitNormal(float x, float y, float z);
    lsVector3 Get_HitNormal() const;
};
```

---

## Usage Examples

### Node Deletion

```cpp
// Mark node for deletion (safe during iteration)
lsEvent_DelNode* event = new lsEvent_DelNode();
event->setNode(objectToDelete);
scene->Get_EventManager()->Add_Event(event);

// Later, in update loop
scene->Get_EventManager()->Process_Queue();
```

### Collision Detection

```cpp
// In physics callback (lsBulletWorld::lsProcessCallback)
lsEvent_HitPoint* event = new lsEvent_HitPoint();
event->Set_HitNode(obj_A);
event->Set_HitPoint(ptB.getX(), ptB.getY(), ptB.getZ());
event->Set_HitNormal(normalOnB.getX(), normalOnB.getY(), normalOnB.getZ());
obj_B->Get_EventManager()->Add_Event(event);

// Handle collision in game code
void MyGame::HandleEvent(lsEvent* event)
{
    if (event->getEvent_type() == LSET_HIT_POINT)
    {
        lsEvent_HitPoint* hit = static_cast<lsEvent_HitPoint*>(event);
        lsNode* hitNode = hit->Get_HitNode();
        lsVector3 hitPoint = hit->Get_HitPoint();
        lsVector3 hitNormal = hit->Get_HitNormal();

        // Apply damage, play sound, etc.
    }
}
```

---

## Event Flow

```
Physics Simulation
    │
    ├── Collision detected
    │
    └── lsBulletWorld::lsProcessCallback()
            │
            └── Create lsEvent_HitPoint
                    │
                    └── scene->Get_EventManager()->Add_Event()

Scene Update
    │
    └── scene->Get_EventManager()->Process_Queue()
            │
            └── event->ProcessEvent()
                    │
                    └── Game code handles event
```

---

## Best Practices

1. **Don't modify scene during physics step** - Use events to defer changes
2. **Process events after physics** - Call `Process_Queue()` after physics stepping
3. **Clean up events** - Delete events after processing
4. **Use for deferred deletions** - Safely remove nodes during iteration
