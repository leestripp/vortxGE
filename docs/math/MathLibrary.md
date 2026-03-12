# Math Library

The math library provides essential mathematical structures and operations for 3D graphics, including vectors, matrices, quaternions, and geometric primitives.

---

## Vector Classes

### lsVector2

A 2D vector with x, y components. Defined in `vortxGE/src/math/lsVector2.h`.

```cpp
class lsVector2
{
public:
    float x, y;

    // Constructors
    lsVector2();
    lsVector2(float X, float Y);

    // Operators
    lsVector2 operator+(const lsVector2& v) const;
    lsVector2 operator-(const lsVector2& v) const;
    lsVector2 operator*(float t) const;
    lsVector2 operator/(float t) const;

    // Operations
    float length() const;
    lsVector2& normalize();
    float dot(const lsVector2& v) const;
};
```

### lsVector3

A 3D vector with x, y, z components. Defined in `vortxGE/src/math/lsVector3.h`.

```cpp
class lsVector3
{
public:
    float x, y, z;

    // Constructors
    lsVector3();
    lsVector3(float X, float Y, float Z);

    // Operators
    lsVector3 operator+(const lsVector3& v) const;
    lsVector3 operator-(const lsVector3& v) const;
    lsVector3 operator*(float t) const;  // Scalar multiplication
    lsVector3 operator/(float t) const;
    lsVector3 operator-() const;  // Negation

    // Vector operations
    lsVector3 Cross(const lsVector3& v) const;  // Cross product
    float Dot(const lsVector3& v) const;  // Dot product
    float length() const;  // Magnitude
    lsVector3& normalize();

    // Rotation
    lsVector3& RotateAboutX(float angle);
    lsVector3& RotateAboutY(float angle);
    lsVector3& RotateAboutZ(float angle);
    lsVector3& RotateAboutAxis(float angle, const lsVector3& axis);

    // Distance
    float distance(const lsVector3& V1) const;
};
```

### lsVector4

A 4D vector with x, y, z, w components. Defined in `vortxGE/src/math/lsVector4.h`.

Often used for:
- Homogeneous coordinates
- Colors (r, g, b, a)
- Quaternions

```cpp
class lsVector4
{
public:
    float x, y, z, w;

    // Constructors
    lsVector4();
    lsVector4(float X, float Y, float Z, float W);

    // Operators
    lsVector4 operator+(const lsVector4& v) const;
    lsVector4 operator-(const lsVector4& v) const;
    lsVector4 operator*(float t) const;

    // Vector operations
    float Dot(const lsVector4& v) const;
    float length() const;
    lsVector4& normalize();
};
```

---

## Matrix Classes

### lsMatrix3x3

A 3x3 matrix for rotation and scale. Defined in `vortxGE/src/math/lsMatrix3x3.h`.

```cpp
class lsMatrix3x3
{
public:
    // 3x3 elements in column-major order
    float m[9];

    // Identity
    void setIdentity();

    // Transpose
    lsMatrix3x3 transpose() const;

    // Multiplication
    lsMatrix3x3 operator*(const lsMatrix3x3& m) const;
    lsVector3 operator*(const lsVector3& v) const;
};
```

### lsMatrix4x4

A 4x4 matrix for transformations. Defined in `vortxGE/src/math/lsMatrix4x4.h`.

Used for:
- Model matrix
- View matrix
- Projection matrix

```cpp
class lsMatrix4x4
{
public:
    // 4x4 elements in column-major order
    float m[16];

    // Identity
    void setIdentity();

    // Transformations
    void translate(float x, float y, float z);
    void scale(float x, float y, float z);
    void rotate(float angle, float x, float y, float z);
    void rotateX(float angle);
    void rotateY(float angle);
    void rotateZ(float angle);

    // Matrix operations
    lsMatrix4x4 transpose() const;
    lsMatrix4x4 inverse() const;

    // Multiplication
    lsMatrix4x4 operator*(const lsMatrix4x4& mat) const;
    lsVector3 operator*(const lsVector3& v) const;
    lsVector4 operator*(const lsVector4& v) const;
};
```

---

## Quaternion Class

### lsQuaternion

A quaternion for rotations. Defined in `vortxGE/src/math/lsQuaternion.h`.

Quaternions avoid gimbal lock and provide smooth interpolation.

```cpp
class lsQuaternion
{
public:
    float x, y, z, w;  // w is the scalar component

    // Constructors
    lsQuaternion();
    lsQuaternion(float X, float Y, float Z, float W);

    // Create from axis-angle
    void set(float angle, float x, float y, float z);

    // Create from Euler angles
    void set(float x, float y, float z);  // Euler angles in degrees

    // Normalize
    void normalize();

    // Conjugate
    lsQuaternion conjugate() const;

    // Multiplication (composition)
    lsQuaternion operator*(const lsQuaternion& q) const;

    // Convert to matrix
    lsMatrix4x4 toMatrix4x4() const;

    // Slerp (spherical linear interpolation)
    static lsQuaternion slerp(const lsQuaternion& a, const lsQuaternion& b, float t);
};
```

---

## Geometric Primitives

### lsPlane

A plane defined by ax + by + cz + d = 0. Defined in `vortxGE/src/math/lsPlane.h`.

```cpp
class lsPlane
{
public:
    lsVector3 normal;  // (a, b, c)
    float d;  // Plane constant

    // Constructors
    lsPlane();
    lsPlane(const lsVector3& normal, float d);

    // Create from points
    void set(const lsVector3& p1, const lsVector3& p2, const lsVector3& p3);

    // Distance from point to plane
    float distance(const lsVector3& point) const;

    // Normalize (make unit length normal)
    void normalize();
};
```

### lsFrustum

A view frustum for culling. Defined in `vortxGE/src/math/lsFrustum.h`.

```cpp
class lsFrustum
{
public:
    lsPlane planes[6];  // Near, Far, Left, Right, Top, Bottom

    // Update frustum from matrix
    void update(const lsMatrix4x4& mvpMatrix);

    // Check if point is inside
    bool isPointInside(const lsVector3& point) const;

    // Check if sphere intersects
    bool isSphereInside(const lsVector3& center, float radius) const;

    // Check if bounding box intersects
    bool isBoxInside(const lsVector3& min, const lsVector3& max) const;
};
```

### lsBoundingSphere

A bounding sphere for culling. Defined in `vortxGE/src/math/lsBoundingSphere.h`.

```cpp
class lsBoundingSphere
{
public:
    lsVector3 center;
    float radius;

    // Constructors
    lsBoundingSphere();
    lsBoundingSphere(const lsVector3& center, float radius);

    // Merge spheres
    void merge(const lsBoundingSphere& other);

    // Check intersection
    bool intersects(const lsBoundingSphere& other) const;
    bool contains(const lsVector3& point) const;
};
```

---

## Usage Examples

### Vector Operations

```cpp
// Create vectors
lsVector3 pos(1.0f, 2.0f, 3.0f);
lsVector3 dir(0.0f, 0.0f, -1.0f);

// Vector math
float len = pos.length();
lsVector3 norm = pos;
norm.normalize();

float dot = pos.Dot(dir);
lsVector3 cross = pos.Cross(dir);

// Distance
float dist = pos.distance(dir);
```

### Matrix Transformations

```cpp
// Create model matrix
lsMatrix4x4 model;
model.setIdentity();
model.translate(5.0f, 0.0f, 0.0f);
model.rotateY(45.0f);  // Degrees
model.scale(2.0f, 2.0f, 2.0f);

// Create view matrix (from camera)
lsMatrix4x4 view = camera->ViewMatrix;

// Create projection matrix
lsMatrix4x4 projection;
projection.setPerspective(60.0f, 1.778f, 0.1f, 1000.0f);

// MVP matrix
lsMatrix4x4 mvp = projection * view * model;
```

### Quaternion Rotation

```cpp
// Create rotation quaternion
lsQuaternion rot;
rot.set(45.0f, 0.0f, 1.0f, 0.0f);  // 45 degrees around Y axis

// Convert to matrix
lsMatrix4x4 rotMatrix = rot.toMatrix4x4();

// Slerp between rotations
lsQuaternion a, b;
a.set(0.0f, 0.0f, 0.0f, 1.0f);
b.set(90.0f, 0.0f, 1.0f, 0.0f);
lsQuaternion result = lsQuaternion::slerp(a, b, 0.5f);  // Halfway
```

### Frustum Culling

```cpp
// Update frustum from camera
lsMatrix4x4 mvp = projection * view;
camera->frustum.update(mvp);

// Check if object should be rendered
lsVector3 objPos = object->position;
float objRadius = object->getBoundingSphere().radius;

if (camera->frustum.isSphereInside(objPos, objRadius))
{
    // Object is visible - add to render list
}
```

### Plane Operations

```cpp
// Create plane from three points
lsPlane floor;
floor.set(
    lsVector3(0.0f, 0.0f, 0.0f),   // Point 1
    lsVector3(1.0f, 0.0f, 0.0f),   // Point 2
    lsVector3(0.0f, 0.0f, 1.0f)    // Point 3
);

// Check if point is above or below plane
lsVector3 playerPos(5.0f, 2.0f, 0.0f);
float dist = floor.distance(playerPos);

if (dist > 0)
{
    // Player is above the floor
}
```

---

## Coordinate System

vortxGE uses a **right-handed coordinate system**:

- **+X**: Right
- **+Y**: Up
- **+Z**: Towards viewer (out of screen)

For cameras, the view direction is along **-Z**:

```
       +Y (up)
        |
        |
        |
        +--------------> +X (right)
       /
      /
     /
    +Z (out of screen)
```

---

## Math Constants

Common math utilities are in `vortxGE/src/math/lsMath.h`:

```cpp
// Pi and related
const float PI = 3.14159265358979f;
const float TWO_PI = 6.28318530717958f;
const float HALF_PI = 1.57079632679490f;

// Degrees to radians
float degToRad(float degrees);

// Radians to degrees
float radToDeg(float radians);

// Clamp value
float clamp(float value, float min, float max);

// Linear interpolation
float lerp(float a, float b, float t);
```
