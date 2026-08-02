# Data Blocks

Data blocks are low-level data structures used by the engine for rendering and physics. They include meshes, batches, textures, and framebuffer objects.

---

## Batch Types

Defined in `vortxGE/src/datablocks/lsBatch.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `LSBT_NONE` | 0 | No batch type |
| `LSBT_LINE` | 1 | Line geometry |
| `LSBT_QUAD` | 2 | Quad geometry (2 triangles) |
| `LSBT_NORMALS` | 3 | Normal visualization |
| `LSBT_BULLET` | 4 | Triangle mesh for physics |
| `LSBT_MESH` | 5 | General mesh |

---

## lsBatch Class

The base class for renderable geometry. Defined in `vortxGE/src/datablocks/lsBatch.h`.

### Class Definition

```cpp
class lsBatch : public lsData
{
public:
    lsBatch(lsRender* render);
    virtual ~lsBatch();

    // Rendering
    virtual void Draw(lsCamera* cam, lsObject* object, lsPass* pass) = 0;
    virtual void Update(double delta) = 0;

    // OpenGL data transfer
    virtual void Copy_GLdata() = 0;
    virtual void Clear_GLdata() = 0;
};
```

### Key Properties

| Property | Type | Description |
|----------|------|-------------|
| `batch_type` | `int` | Type identifier |
| `name` | `string` | Batch name |
| `shader` | `lsShader*` | Shader program |
| `bSphere` | `lsBoundingSphere` | Bounding sphere |
| `vertex_count` | `int` | Number of vertices |

---

## lsBatch_Mesh Class

A mesh batch for general 3D geometry. Defined in `vortxGE/src/datablocks/lsBatch_Mesh.h`.

### Features

- Vertex buffer (position, normal, UV)
- Index buffer
- Multiple material groups
- PBR material support

### Public Data

```cpp
// GPU mesh data — renderer-agnostic handles (see render/lsGLTypes.h)
lsVAOID vao_mesh;               // Vertex array object
lsBufferID bufferObjects[6];    // VBOs (vertex, normal, UV, tangent, …) + index buffer

// PBR Textures (renderer texture handles, not pointers)
lsTextureID tex_albedo;
lsTextureID tex_normal;
lsTextureID tex_roughness_metallic;

// Material properties (PBR)
lsVector4 m_albedo;
float m_roughness;
float m_metallic;
```

### Usage

```cpp
lsBatch_Mesh* batch = new lsBatch_Mesh(render);
batch->Copy_GLdata();  // Upload to GPU

// Render
batch->Draw(cam, object, pass);
```

---

## lsBatch_Quad Class

A full-screen quad for post-processing. Defined in `vortxGE/src/datablocks/lsBatch_Quad.h`.

### Usage

```cpp
// Create quad for post-processing
lsBatch_Quad* quad = new lsBatch_Quad(render);
quad->Copy_GLdata();

// In pass render
quad->Draw(cam, object, pass);
```

---

## lsBatch_Lines Class

Line geometry for debugging. Defined in `vortxGE/src/datablocks/lsBatch_Lines.h`.

### Usage

```cpp
lsBatch_Lines* lines = new lsBatch_Lines(render);

// Add line
lines->Add_Vertex(pos1);
lines->Add_Vertex(pos2);
lines->Copy_GLdata();

// Render
lines->Draw(cam, object, pass);
```

---

## lsMesh Class

A container for multiple batches. Defined in `vortxGE/src/datablocks/lsMesh.h`.

### Class Definition

```cpp
class lsMesh
{
public:
    lsMesh();
    ~lsMesh();

    // Batch management
    void Add_Batch(lsBatch* batch);
    void Del_Batch(lsBatch* batch);

    // Rendering
    void Draw(lsCamera* cam, lsObject* object, lsPass* pass);

    // Bounding sphere
    lsBoundingSphere bSphere;
};
```

### Structure

```
lsMesh
    |
    +-- lsBatch_Mesh (rendering)
    +-- lsBatch_Bullet (physics)
    +-- lsBatch_Normals (debug)
```

---

## lsFBO Class

Frame Buffer Object for rendering to textures. Defined in `vortxGE/src/datablocks/lsFBO.h`.

### FBO Types

| Type | Description |
|------|-------------|
| Color Only | Color texture only |
| Color + Depth | Color and depth textures |
| Depth | Depth texture only |

### Class Definition

```cpp
class lsFBO
{
public:
    lsFBO(lsRender* render);
    virtual ~lsFBO();

    virtual void Enable() = 0;        // Bind FBO as the render target
    virtual void ClearBuffers() = 0;  // Clear colour/depth/stencil
    virtual void Resize(double width, double height) = 0;
                                      // Recreate at new physical pixel size

    double getWidth() const;
    double getHeight() const;

protected:
    lsFBOID m_fbo;   // Renderer framebuffer handle (uint32_t)
};
```

> **Renderer abstraction:** engine code never touches GL handles
> directly. All GPU resources are engine-level types (`lsFBOID`,
> `lsTextureID`, `lsBufferID`, `lsVAOID`, … — all `uint32_t`) defined in
> `vortxGE/src/render/lsGLTypes.h`, and all GPU calls go through the
> `lsRender` interface (`getRender()->Xxx(...)`).

---

## lsFBO_Depth Class

Depth-only FBO for shadow maps. Defined in `vortxGE/src/datablocks/lsFBO_Depth.h`.

### Usage

```cpp
lsFBO_Depth* depthFBO = new lsFBO_Depth(2048, 2048, render);

// Render to depth texture
depthFBO->Enable();
// ... render scene from light perspective ...
// (restore the default framebuffer via the render pass)

// Use in main pass — through the renderer abstraction, no raw GL calls
render->ActiveTexture(LS_TEXTURE0 + shadowUnit);
render->BindTexture(LS_TEXTURE_2D, depthFBO->getDepthTexture());
```

---

## lsTexture Class

2D texture handling. Defined in `vortxGE/src/datablocks/lsTexture.h`.

### Features

- Multiple formats (RGB, RGBA, depth)
- Mipmap generation
- Filtering modes
- Wrapping modes

### Class Definition

```cpp
class lsTexture
{
public:
    lsTexture(lsRender* render);
    virtual ~lsTexture();

    bool load_image(string filename, bool mipmap, bool compressed,
                    bool is_hdr = false);

    lsTextureID getTexture() const;   // Renderer texture handle (uint32_t)
    int getWidth() const;
    int getHeight() const;
};
```

---

## lsTGA Class

TGA image file loading. Defined in `vortxGE/src/datablocks/lsTGA.h`.

### Usage

```cpp
lsTGA* tga = new lsTGA();
if (tga->Load("texture.tga"))
{
    lsTexture* tex = new lsTexture(render);
    tex->Create(tga->width, tga->height, GL_RGBA, tga->data);
}
delete tga;
```

---

## lsStats Class

Performance statistics tracking. Defined in `vortxGE/src/datablocks/lsStats.h`.

### Metrics Tracked

- Frame time
- FPS (frames per second)
- Draw calls
- Vertices rendered
- Triangles rendered

### Usage

```cpp
lsStats* stats = engine->getRender()->getStats();

// In render loop
stats->Loop_Start();
// ... rendering ...
stats->Loop_Stop();
stats->Update();

// Get values
double fps = stats->getFPS();
double frameTime = stats->getFrame_Time();
int triangles = stats->getTriangle_count();
```

---

## Usage Example: Creating a Custom Mesh

```cpp
// Create mesh
lsMesh* mesh = new lsMesh();

// Create batch
lsBatch_Mesh* batch = new lsBatch_Mesh(render);

// Define vertices (position, normal, uv)
float vertices[] = {
    // position       // normal     // uv
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f,
    -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    // ... more vertices ...
};

unsigned int indices[] = {
    0, 1, 2,
    // ... more indices ...
};

// Upload to GPU
batch->Copy_GLdata();

// Add to mesh
mesh->Add_Batch(batch);
```
