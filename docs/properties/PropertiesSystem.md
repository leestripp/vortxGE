# Properties System

The properties system provides a flexible way to manage node attributes that can be animated. Properties are used by the animation system to interpolate values over time.

---

## Property Types

Defined in `vortxGE/src/properties/lsProperty.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `LSPROP_BASE` | 0 | Base property |
| `LSPROP_FLOAT` | 1 | Single float |
| `LSPROP_INT` | 2 | Integer |
| `LSPROP_VECTOR2` | 3 | 2D vector |
| `LSPROP_VECTOR3` | 4 | 3D vector |
| `LSPROP_VECTOR5` | 5 | 4D vector |

---

## Property IDs

| Constant | Value | Description |
|----------|-------|-------------|
| `LSPROPID_NONE` | 0 | No property |
| `LSPROPID_POSITION` | 1 | Node position |
| `LSPROPID_ROTATION` | 2 | Node rotation |
| `LSPROPID_SCALE` | 3 | Node scale |
| `LSPROPID_DIFFUSE` | 4 | Material diffuse |
| `LSPROPID_SPECULAR` | 5 | Material specular |
| `LSPROPID_AMBIENT` | 6 | Material ambient |
| `LSPROPID_SHININESS` | 7 | Material shininess |
| `LSPROPID_GLOW` | 8 | Material glow |
| `LSPROPID_CAMERA_FOV` | 9 | Camera field of view |
| `LSPROPID_CAMERA_RATIO` | 10 | Camera aspect ratio |
| `LSPROPID_CAMERA_NEAR` | 11 | Camera near plane |
| `LSPROPID_CAMERA_FAR` | 12 | Camera far plane |

---

## lsProperty Class

Base class for all properties. Defined in `vortxGE/src/properties/lsProperty.h`.

### Class Definition

```cpp
class lsProperty
{
public:
    lsProperty();
    virtual ~lsProperty();

    // Serialization
    virtual const string& toString() = 0;
    virtual const string& fromString(const string& str) = 0;

    // Type and ID
    void setProp_type(int val);
    int getProp_type() const;

    void setProp_id(int val);
    int getProp_id() const;

    // Navigation
    lsProperty* getNext_prop() const;
    void setNext_prop(lsProperty* val);
    lsProperty* getPrev_prop() const;
    void setPrev_prop(lsProperty* val);
};
```

---

## Property Implementations

### lsProperty_Float

A single float property. Defined in `vortxGE/src/properties/lsProperty_Float.h`.

```cpp
class lsProperty_Float : public lsProperty
{
public:
    lsProperty_Float();
    virtual ~lsProperty_Float();

    void setValue(float val);
    float getValue() const;

    virtual const string& toString() override;
    virtual const string& fromString(const string& str) override;
};
```

### lsProperty_Vector2

A 2D vector property. Defined in `vortxGE/src/properties/lsProperty_Vector2.h`.

```cpp
class lsProperty_Vector2 : public lsProperty
{
public:
    lsProperty_Vector2();
    virtual ~lsProperty_Vector2();

    void setValue(float x, float y);
    lsVector2 getValue() const;

    virtual const string& toString() override;
    virtual const string& fromString(const string& str) override;

    lsVector2 value;
};
```

### lsProperty_Vector3

A 3D vector property. Defined in `vortxGE/src/properties/lsProperty_Vector3.h`.

```cpp
class lsProperty_Vector3 : public lsProperty
{
public:
    lsProperty_Vector3();
    virtual ~lsProperty_Vector3();

    void setValue(float x, float y, float z);
    lsVector3 getValue() const;

    virtual const string& toString() override;
    virtual const string& fromString(const string& str) override;

    lsVector3 value;
};
```

### lsProperty_Vector4

A 4D vector property. Defined in `vortxGE/src/properties/lsProperty_Vector4.h`.

```cpp
class lsProperty_Vector4 : public lsProperty
{
public:
    lsProperty_Vector4();
    virtual ~lsProperty_Vector4();

    void setValue(float x, float y, float z, float w);
    lsVector4 getValue() const;

    virtual const string& toString() override;
    virtual const string& fromString(const string& str) override;

    lsVector4 value;
};
```

---

## Usage with Animation

Properties are linked to animation curves:

```cpp
// Create property
lsProperty_Vector3* posProp = new lsProperty_Vector3();
posProp->setProp_id(LSPROPID_POSITION);
posProp->setValue(0.0f, 0.0f, 0.0f);

// Add to anim node
animNode->Add_Property(posProp);

// Create animation
lsAnimProperty_Vector3* animProp = new lsAnimProperty_Vector3(posProp);
animMgr->Add_AnimProperty(posProp);

// Add keyframes
animMgr->InsertKey_Pos(0.0, lsVector3(0, 0, 0));
animMgr->InsertKey_Pos(60.0, lsVector3(10, 0, 0));  // 1 second later at 60fps
```

---

## Material Properties

Properties are used for material animation:

```cpp
// Create material property
lsProperty_Vector4* diffuse = new lsProperty_Vector4();
diffuse->setProp_id(LSPROPID_DIFFUSE);
diffuse->setValue(1.0f, 0.0f, 0.0f, 1.0f);  // Red

// Add to object
object->Add_Property(diffuse);

// Animate color
animMgr->InsertKey_Diffuse(0.0, lsVector4(1, 0, 0, 1));   // Red
animMgr->InsertKey_Diffuse(30.0, lsVector4(0, 0, 1, 1));   // Blue at 0.5s
```
