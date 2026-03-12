# Shader System

The shader system in vortxGE manages GLSL shader programs for rendering. It provides a unified interface for loading, compiling, and using shaders with support for PBR materials, lighting, and shadow mapping.

---

## Architecture Overview

```
lsShaderManager
    │
    └── lsShader (per shader program)
            │
            ├── Vertex Shader (.vert)
            └── Fragment Shader (.frag)
```

---

## Shader Kinds

Defined in `vortxGE/src/shader/lsShaderKinds.h`:

| Kind | Description |
|------|-------------|
| `None` | No shader |
| `Debug` | Debug visualization |
| `LightTexNone` | Lighting without textures |
| `LightTexD` | Lighting with diffuse texture |
| `LightTexDN` | Lighting with diffuse + normal maps |
| `LightTexDNS` | Lighting with diffuse + normal + PBR (metallic/roughness) |
| `Occlusion` | Occlusion culling shader |
| `ShadelessTexD` | Unlit with diffuse texture |
| `ShadelessTexNone` | Unlit without textures |
| `Comp` | Composite shader |
| `Quad` | Full-screen quad shader |
| `Bloom` | Bloom post-processing |
| `DebugNormals` | Normal visualization |
| `ShadowMap` | Shadow map generation |
| `Skybox` | Skybox rendering |

---

## lsShader Class

Manages a single shader program. Defined in `vortxGE/src/shader/lsShader.h`.

### Construction

```cpp
lsShader(lsRender* render);
```

### Loading Shaders

```cpp
void Load_Shader(const string& name);
```

Loads vertex and fragment shaders from files:
- `{name}.vert` - Vertex shader
- `{name}.frag` - Fragment shader

### Using Shaders

```cpp
void UseShader(lsCamera* cam, lsObject* object, lsPass* pass, lsBatch* batch);
```

Binds the shader and sets all uniforms for rendering.

---

## Uniforms

### Matrices

| Uniform | Type | Description |
|---------|------|-------------|
| `pMatrix` | mat4 | Projection matrix |
| `mvMatrix` | mat4 | Model-view matrix |
| `mMatrix` | mat4 | Model matrix |
| `nMatrix` | mat3 | Normal matrix |
| `sMatrix[3]` | mat4[3] | Shadow matrices (up to 3) |

### PBR Textures

| Uniform | Type | Description |
|---------|------|-------------|
| `tex_albedo` | sampler2D | Diffuse/albedo texture (RGB) |
| `tex_normal` | sampler2D | Normal map (RGB) |
| `tex_roughness_metallic` | sampler2D | R: roughness, G: metallic, B: AO |

### Material Properties

| Uniform | Type | Description |
|---------|------|-------------|
| `matAlbedo` | vec3 | Albedo color |
| `matRoughness` | float | Roughness value |
| `matMetallic` | float | Metallic value |

### Light Properties (Array, max 3 lights)

| Uniform | Type | Description |
|---------|------|-------------|
| `lightCount` | int | Number of active lights |
| `lightType[i]` | int | Light type (0=point, 1=spot, 2=direct) |
| `lightPos[i]` | vec3 | Light position (view space) |
| `lightDir[i]` | vec3 | Light direction |
| `lightCutoff[i]` | float | Spot light inner cutoff |
| `lightOuterCutoff[i]` | float | Spot light outer cutoff |
| `lightDiffuse[i]` | vec4 | Diffuse color (rgb) + intensity |
| `lightSpecular[i]` | vec4 | Specular color (rgb) + intensity |
| `lightIntensity[i]` | float | Light intensity |

### Shadow Mapping

| Uniform | Type | Description |
|---------|------|-------------|
| `tex_shadowmaps[3]` | sampler2D[3] | Shadow map textures |
| `use_shadowmap[3]` | bool[3] | Shadow map enabled flags |

---

## lsShaderManager Class

Manages all shader programs. Defined in `vortxGE/src/shader/lsShaderManager.h`.

### Construction

```cpp
lsShaderManager(lsRender* render);
```

### Loading Shaders

```cpp
// Load shader from files
lsShader* Load_Shader(const string& name);

// Find existing shader
lsShader* Find_Shader(const string& name);
```

### Predefined Shaders

The engine provides several built-in shaders:

```cpp
// Debug shaders
manager->Load_Shader("debug");
manager->Load_Shader("debug_normals");

// Lighting shaders (untextured)
manager->Load_Shader("light_tex_none");

// Lighting shaders (diffuse only)
manager->Load_Shader("light_tex_d");

// Lighting shaders (diffuse + normal)
manager->Load_Shader("light_tex_dn");

// Lighting shaders (full PBR)
manager->Load_Shader("light_tex_dns");

// Shadow mapping
manager->Load_Shader("shadowmap");

// Post-processing
manager->Load_Shader("quad");
manager->Load_Shader("bloom");

// Skybox
manager->Load_Shader("skybox");
```

---

## Shader Files

Shaders are located in `demos/data/shaders/`:

### Vertex Shader Structure

```glsl
#version 430 core

// Inputs
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

// Uniforms
uniform mat4 pMatrix;
uniform mat4 mvMatrix;
uniform mat4 mMatrix;
uniform mat3 nMatrix;

// Outputs
out vec3 vPosition;
out vec3 vNormal;
out vec2 vTexcoord;

void main()
{
    vPosition = (mMatrix * vec4(position, 1.0)).xyz;
    vNormal = normalize(nMatrix * normal);
    vTexcoord = texcoord;
    gl_Position = pMatrix * mvMatrix * vec4(position, 1.0);
}
```

### Fragment Shader Structure (PBR)

```glsl
#version 430 core

// Inputs from vertex shader
in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexcoord;

// Outputs
out vec4 fragColor;

// Uniforms
uniform sampler2D tex_albedo;
uniform sampler2D tex_normal;
uniform sampler2D tex_roughness_metallic;

// Material uniforms
uniform vec3 matAlbedo;
uniform float matRoughness;
uniform float matMetallic;

// Light uniforms (array)
uniform int lightCount;
uniform vec3 lightPos[3];
// ... other light uniforms

void main()
{
    // Sample textures
    vec3 albedo = texture(tex_albedo, vTexcoord).rgb * matAlbedo;
    vec3 normal = texture(tex_normal, vTexcoord).rgb;
    float roughness = texture(tex_roughness_metallic, vTexcoord).r * matRoughness;
    float metallic = texture(tex_roughness_metallic, vTexcoord).g * matMetallic;

    // PBR calculation
    // ... Cook-Torrance BRDF

    fragColor = vec4(result, 1.0);
}
```

---

## PBR Shading

The engine uses a Physically Based Rendering (PBR) model based on the Cook-Torrance BRDF.

### Material Workflow

1. **Albedo** - Base color texture or uniform
2. **Normal** - Tangent space normal map
3. **Roughness** - Surface roughness (0 = mirror, 1 = diffuse)
4. **Metallic** - Metalness (0 = dielectric, 1 = metal)

### Shader Selection

```cpp
// Select appropriate shader based on material
if (hasAlbedo && hasNormal && hasMetallicRoughness)
{
    // Full PBR
    shader = shaderManager->Find_Shader("light_tex_dns");
}
else if (hasAlbedo && hasNormal)
{
    // Diffuse + Normal
    shader = shaderManager->Find_Shader("light_tex_dn");
}
else if (hasAlbedo)
{
    // Diffuse only
    shader = shaderManager->Find_Shader("light_tex_d");
}
else
{
    // No textures
    shader = shaderManager->Find_Shader("light_tex_none");
}
```

---

## Shadow Mapping

### Vertex Shader

```glsl
uniform mat4 pMatrix;
uniform mat4 mvMatrix;

void main()
{
    gl_Position = pMatrix * mvMatrix * vec4(position, 1.0);
}
```

### Fragment Shader

```glsl
#version 430 core
out vec4 fragColor;

void main()
{
    // Depth is automatically written
    fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}
```

### Shadow Calculation

```glsl
// In main lighting shader
float shadow = 1.0;
if (use_shadowmap[i])
{
    vec4 shadowCoord = sMatrix[i] * vec4(vPosition, 1.0);
    shadowCoord.xyz /= shadowCoord.w;
    shadowCoord.xyz = shadowCoord.xyz * 0.5 + 0.5;

    float shadowDepth = texture(shadowMap, shadowCoord.xy).r;
    shadow = shadowCoord.z - shadow_bias > shadowDepth ? 0.2 : 1.0;
}
```

---

## Custom Shaders

To create custom shaders:

1. Create `.vert` and `.frag` files in your shader directory
2. Load with `shaderManager->Load_Shader("shader_name")`
3. Use with objects via `lsBatch`

### Example: Simple Color Shader

**simple_color.vert**
```glsl
#version 430 core

layout(location = 0) in vec3 position;

uniform mat4 pMatrix;
uniform mat4 mvMatrix;

void main()
{
    gl_Position = pMatrix * mvMatrix * vec4(position, 1.0);
}
```

**simple_color.frag**
```glsl
#version 430 core

out vec4 fragColor;

uniform vec3 color;

void main()
{
    fragColor = vec4(color, 1.0);
}
```

---

## Debug Shaders

The engine includes debug shaders for visualization:

### Debug Normals

Displays surface normals as RGB colors:
- X axis = Red
- Y axis = Green
- Z axis = Blue

```cpp
lsShader* debug = shaderManager->Load_Shader("debug_normals");
```

### Debug Colors

Displays wireframe or solid debug colors.

```cpp
lsShader* debug = shaderManager->Load_Shader("debug");
```
