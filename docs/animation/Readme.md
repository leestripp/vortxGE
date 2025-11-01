# vortxGE Animation System API Documentation

This documentation describes the animation system of the **vortxGE** engine, which provides a powerful and flexible way to animate properties of game objects through keyframe-based curves.

## Overview

The animation system allows developers to animate various types of properties such as position, rotation, scale, and material colors using cubic spline interpolation. It supports both 3D vector properties (Vector3) and 4D vector properties (Vector4), with each property having its own set of animated curves.

---

## Core Classes

### `lsAnimProperty`

**Purpose**: Base class representing an animation property that links to a specific `lsProperty` instance. This class handles updating the associated property based on time using animation curves.

#### Public Methods
- `lsAnimProperty(lsProperty* p)` - Constructor taking a pointer to the target property.
- `virtual ~lsAnimProperty()` - Virtual destructor for proper inheritance handling.
- `virtual void Update(double frame)` - Pure virtual method to update the linked property at a given frame.
- `void setAnimprop_type(int val)` - Sets the type of animation property (e.g., Vector3, Vector4).
- `int getAnimprop_type() const` - Gets the current animation property type.
- `void setProperty(lsProperty* val)` - Sets the target property to animate.
- `lsProperty* getProperty() const` - Gets the currently linked property.

#### Navigation Methods
These methods allow navigation through a doubly-linked list of animation properties:
- `void setNext_animprop(lsAnimProperty* val)`
- `lsAnimProperty* getNext_animprop() const`
- `void setPrev_animprop(lsAnimProperty* val)`
- `lsAnimProperty* getPrev_animprop() const`

---

### `lsAnimProperty_Vector3`

**Inherits From**: `lsAnimProperty`

**Purpose**: Specialized class for animating Vector3 properties using three separate cubic curves (X, Y, Z).

#### Public Methods
- `lsAnimProperty_Vector3(lsProperty* p)` - Constructor initializing the three curves.
- `virtual ~lsAnimProperty_Vector3()` - Destructor cleaning up allocated curves.
- `void Update(double frame)` - Updates the associated property with interpolated values from all three curves.
- `void InsertKey(double frame, const lsVector3& val)` - Adds a keyframe for each component of the vector.

#### Private Members
- `lsCubicCurve* curve1`, `curve2`, `curve3` - Three curves for X, Y, and Z components respectively.

---

### `lsAnimProperty_Vector4`

**Inherits From**: `lsAnimProperty`

**Purpose**: Specialized class for animating Vector4 properties using four separate cubic curves (X, Y, Z, W).

#### Public Methods
- `lsAnimProperty_Vector4(lsProperty* p)` - Constructor initializing the four curves.
- `virtual ~lsAnimProperty_Vector4()` - Destructor cleaning up allocated curves.
- `void Update(double frame)` - Updates the associated property with interpolated values from all four curves.
- `void InsertKey(double frame, const lsVector4& val)` - Adds a keyframe for each component of the vector.

#### Private Members
- `lsCubicCurve* curve1`, `curve2`, `curve3`, `curve4` - Four curves for X, Y, Z, and W components respectively.

---

### `lsAnimationManager`

**Purpose**: Central manager responsible for coordinating all animation properties within an `lsAnimNode`. Handles playback controls like play/pause/looping and frame advancement.

#### Public Methods
- `lsAnimationManager()` - Constructor setting default values.
- `virtual ~lsAnimationManager()` - Destructor cleaning up the animation property list.
- `void Init(lsAnimNode* n, double fps)` - Initializes the manager with an animation node and frame rate.
- `void UpdateAll(double delta)` - Updates all animated properties for the current frame based on elapsed time.
- `void Play(bool loop)` - Starts playback with optional looping behavior.
- `void Stop()` - Stops playback.

#### Property Management
- `void Add_AnimProperty(lsProperty* p)` - Adds an animation property to the manager, categorizing it by type and ID for fast access.

#### Keyframe Insertion
Methods for inserting keyframes into specific animated properties:
- `InsertKey_Pos(double frame, const lsVector3& pos)`
- `InsertKey_Rot(double frame, const lsVector3& rot)`
- `InsertKey_Scale(double frame, const lsVector3& aca)`
- `InsertKey_Diffuse(double frame, const lsVector4& val)`
- `InsertKey_Specular(double frame, const lsVector4& val)`
- `InsertKey_Ambient(double frame, const lsVector4& val)`

#### Getters/Setters
- `void setFramerate(double val)` - Sets the animation frame rate.

#### Private Members
- `lsAnimProperty* animproperty_list` - Doubly-linked list of animation properties.
- `lsAnimNode* animnode` - Pointer to the node this manager is attached to.
- `double framerate`, `curr_frame`, `start_frame`, `end_frame` - Animation timing data.
- `bool looping`, `pause` - Playback control flags.
- Fast lookup pointers for common transformations and materials:
  - `lsAnimProperty_Vector3* position`, `rotation`, `scale`
  - `lsAnimProperty_Vector4* diffuse`, `specular`, `ambient`

---

### `lsCubicCurve`

**Purpose**: Represents a single cubic spline curve used to interpolate values between keyframes.

#### Public Methods
- `lsCubicCurve()` - Constructor.
- `virtual ~lsCubicCurve()` - Destructor.
- `void Add_Key(double frame, double value)` - Adds a new keyframe to the curve.
- `void Del_Key(lsKeyFrame* key)` - Removes a specified keyframe from the curve.
- `void Clear_AllKeys()` - Removes all keys from the curve.
- `double Get_Value(double frame)` - Interpolates and returns the value at a given frame.
- `bool Empty()` - Checks whether the curve contains any keys.

#### Private Methods
- `void InsertKey_Before(lsKeyFrame* key, lsKeyFrame* newkey)` - Inserts a key before another key in the list.
- `void Add_ToList(lsKeyFrame* key)` - Adds a key to the front of the list.
- `lsKeyFrame* Find_Key(double frame)` - Finds the appropriate starting key for interpolation.

#### Private Members
- `lsKeyFrame* keyframe_list` - Head of the linked list of keyframes.
- `int keyframe_count` - Number of stored keyframes.
- `double currpos` - Current position along the curve (used during interpolation).
- `lsKeyFrame* key0`, `key1`, `key2`, `key3` - Temporary pointers used for Catmull-Rom spline calculation.

---

### `lsKeyFrame`

**Purpose**: Represents a single point in time with an associated value, forming part of a `lsCubicCurve`.

#### Public Methods
- `lsKeyFrame()` - Constructor.
- `virtual ~lsKeyFrame()` - Destructor.
- Navigation methods:
  - `lsKeyFrame* getNext_keyframe() const`
  - `void setNext_keyframe(lsKeyFrame*)`
  - `lsKeyFrame* getPrev_keyframe() const`
  - `void setPrev_keyframe(lsKeyFrame*)`
- Frame and value accessors:
  - `double getFrame() const`
  - `void setFrame(double)`
  - `double getValue() const`
  - `void setValue(double)`

#### Private Members
- `lsKeyFrame* next_keyframe`, `prev_keyframe` - Links to adjacent keyframes.
- `double frame`, `value` - Time and value of the keyframe.

---

## Constants

Defined in `lsAnimProperty.h`:
```cpp
const int LSANIMPROP_NONE     = 0;
const int LSANIMPROP_VECTOR2  = 1;
const int LSANIMPROP_VECTOR3  = 2;
const int LSANIMPROP_VECTOR4  = 3;
const int LSANIMPROP_FLOAT    = 4;
```

---

## Usage Example

To use the animation system:

1. Create an `lsAnimNode` instance.
2. Initialize the `lsAnimationManager` with the node:
   ```cpp
   lsAnimationManager animMgr;
   animMgr.Init(myAnimNode, 60.0); // 60 FPS
   ```
3. Insert keyframes for desired properties:
   ```cpp
   animMgr.InsertKey_Pos(0.0, lsVector3(0,0,0));
   animMgr.InsertKey_Pos(10.0, lsVector3(10,10,10));
   ```
4. Start playback:
   ```cpp
   animMgr.Play(true); // loop
   ```
5. In your game loop, call update:
   ```cpp
   animMgr.UpdateAll(deltaTimeMs);
   ```

---

## Notes

- The system assumes that each property has a corresponding `lsAnimProperty` subclass.
- All curve interpolation uses Catmull-Rom splines for smooth transitions between keyframes.
- Keyframe data is stored in a doubly-linked list for efficient insertion and traversal.
- Animation playback uses real-time frame advancement scaled by the specified frame rate.

