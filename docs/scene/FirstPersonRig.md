# First Person Rig Node API Documentation

## Overview
The `lsFirstPersonRig` class represents a first-person character controller in the Vortx Game Engine. It provides player movement, camera control, physics integration, and object interaction capabilities for first-person perspective gameplay. Use this as a guide to create more complex rigs for your game.

## Class Hierarchy
```
lsNode
└── lsObject
    └── lsFirstPersonRig
```

## Constructor
```cpp
lsFirstPersonRig(lsScene* scene, btDiscreteDynamicsWorld* dynamicsWorld)
```
Creates a new first-person rig with physics and camera components.

**Parameters:**
- `scene`: Pointer to the parent scene
- `dynamicsWorld`: Bullet physics dynamics world for physics simulation

## Public Methods

### Movement Control
```cpp
void HandleKeyboardInput(bool forward, bool backward, bool strafeLeft, bool strafeRight, bool crouch, bool jump, bool sprint)
```
Handles keyboard input for player movement and actions.

**Parameters:**
- `forward`: Move forward
- `backward`: Move backward  
- `strafeLeft`: Strafe left
- `strafeRight`: Strafe right
- `crouch`: Crouch state
- `jump`: Jump action
- `sprint`: Sprint state

```cpp
void HandleMouseLook(float delta, float delta_x, float delta_y)
```
Processes mouse movement for camera rotation.

**Parameters:**
- `delta`: Time delta for smooth interpolation
- `delta_x`: Horizontal mouse movement
- `delta_y`: Vertical mouse movement

```cpp
void HandleMouseButtons(bool left, bool right)
```
Handles mouse button actions (pickup/throw).

**Parameters:**
- `left`: Left mouse button state
- `right`: Right mouse button state

### Object Interaction
```cpp
void tryPickupObject()
```
Attempts to pickup an object in front of the player.

```cpp
void pickupObject(lsObject* obj)
```
Picks up a specific object.

**Parameters:**
- `obj`: Pointer to the object to pickup

```cpp
void releaseObject()
```
Releases the currently held object.

```cpp
void throwObject()
```
Throws the currently held object with force.

### Camera Access
```cpp
lsCamera* getCamera() const
```
Returns pointer to the first-person camera.

## Properties

### Movement Parameters
- `m_walkSpeed`: Walking movement speed (default: 4.0f)
- `m_sprintSpeed`: Sprinting movement speed (default: 8.0f)  
- `m_crouchSpeed`: Crouching movement speed (default: 2.0f)
- `m_jumpForce`: Jump impulse force (default: 15.0f)
- `m_mouseXSensitivity`: Horizontal mouse sensitivity (default: 0.05f)
- `m_mouseYSensitivity`: Vertical mouse sensitivity (default: 1.85f)
- `m_airResistance`: Air resistance when not on ground (default: 0.95)

### Physics Properties
- `m_capsuleRadius`: Capsule collision shape radius (default: 0.5f)
- `m_standingCapsuleHeight`: Standing capsule height (default: 1.8f)
- `m_crouchingCapsuleHeight`: Crouching capsule height (default: 1.0f)

### Camera Properties
- `m_cameraHeightOffset`: Current camera Y offset from rig center
- `m_standingCameraOffset`: Camera offset when standing (default: 0.6f)
- `m_crouchingCameraOffset`: Camera offset when crouching (default: 0.0f)
- `m_cameraLerpSpeed`: Camera height interpolation speed (default: 0.1f)

### Pickup Properties
- `m_pickupDistance`: Distance to pickup objects (default: 5.0f)
- `m_throwForce`: Force applied when throwing objects (default: 30.0f)
- `m_holdDistance`: Distance where held objects are positioned (default: 4.0f)

## State Management

### Ground Detection
```cpp
void checkGroundCollision()
```
Checks if the player is currently on the ground using raycasting.

### Capsule Shape Management
```cpp
void updateCapsuleShape()
```
Updates the physics capsule shape when crouching/standing.

## Physics Integration

The rig integrates with Bullet physics through:
- Dynamic rigid body with mass (75kg)
- Capsule collision shapes that adjust for crouching
- Proper physics damping and friction settings
- Ground detection using raycasting

## Camera System
- Attached as a child node to the rig
- Position adjusted based on crouching state
- Rotation controlled via mouse input
- Smooth interpolation of camera height transitions

## Usage Example
```cpp
// Create first person rig
lsFirstPersonRig* player = new lsFirstPersonRig(scene, dynamicsWorld);

// Handle input
player->HandleKeyboardInput(forward, backward, strafeLeft, strafeRight, crouch, jump, sprint);
player->HandleMouseLook(deltaTime, mouseX, mouseY);

// Handle mouse buttons for pickup/throw
player->HandleMouseButtons(leftClick, rightClick);
```

## Notes
- The rig automatically manages physics body position synchronization
- Camera height smoothly interpolates between standing/crouching states
- Object pickup system uses 6DOF constraints for stable object holding
- All physics properties are configured for smooth first-person movement behavior

