# vortxGE Scene Exporter Documentation

## Overview

This Blender addon exports scenes in a format compatible with the vortxGE engine. It generates a `.vgs` file describing your scene structure along with individual `.glb` files for each mesh object.

## Features

- Exports all **mesh objects** as `.glb` files into a subfolder named `meshes/`
- Writes an `.vgs` file describing:
  - Scene structure
  - Lights (point, sun/directional, spot)
  - Mesh objects with transforms
  - Physics properties for bullet collision shapes
- Supports physics-only meshes via custom object properties

## Installation Instructions

1. Save this file as `export_scene.py`
2. In Blender:
   - Go to **Edit > Preferences > Add-ons**
   - Click **Install...**
   - Select the saved `.py` file
3. Enable the add-on by checking the box next to "vortxGE Scene Exporter"

## How to Use

### Exporting Your Scene

1. Open your scene in Blender
2. From the menu:
   - Go to **File > Export > vortxGE Scene (.vgs)**
3. Choose where to save the `.vgs` file and click **Export vortxGE Scene**

Blender will:
- Create a `meshes/` directory in the same folder as your `.vgs` file
- Export each mesh object into a corresponding `.glb` file inside `meshes/`
- Generate a `.vgs` file with all necessary commands for importing into vortxGE

## Custom Properties for Physics Simulation

### Bullet Collision Shapes

| Property              | Format                             | Description                                         |
|-----------------------|------------------------------------|-----------------------------------------------------|
| `bullet_only_mesh`    | `"type mass"`                      | For non-rendered physics-only meshes                |
| `bullet_box`          | `"mass (sx, sy, sz)"`              | Box-shaped collision shape                          |
| `bullet_pill`         | `"mass radius height"`             | Pill/capsule-shaped collision shape                 |
| `bullet_convexhull`   | `"mass"`                           | Convex hull-shaped collision                        |
| `bullet_trimesh`      | `"mass"`                           | Triangle mesh-based collision shape                 |

### Using Bullet Only Meshes

To create a physics-only mesh (a mesh that is not rendered but has collision properties):

1. Select the mesh object in Blender
2. In the **Object Properties** panel, go to the **Custom Properties** section
3. Add a new property named `bullet_only_mesh`
4. Set its value to `"type mass"` where:
   - `type` is one of: `convexhull`, or `trimesh`
   - `mass` is a numeric value representing the object's mass (e.g., "2.5") normally set to 0.0 for a static shape.

Example:
```
Name : bullet_only_mesh
Value : "convexhull 0.0"
```

This will export the mesh as a physics-only object that does not appear in the rendered scene but still provides collision detection.

## Output File Format Example

Here's a sample `.vgs` file generated from a scene with one light and two meshes:

```vgs
add_scene `MyScene`

add_light point `PointLight01` `MyScene` (1.000000, 2.000000, 3.000000) (0.000000, 0.000000, 0.000000) (1.000000, 1.000000, 1.000000) 500.000000

add_object `Cube` `MyScene` (0.000000, 0.000000, 0.000000) (0.000000, 0.000000, 0.000000) (1.000000, 1.000000, 1.000000) `data/meshes/Cube.glb`
add_bullet_convexhull `Cube` 50.0

add_object `Sphere` `MyScene` (5.000000, 0.000000, 0.000000) (0.000000, 0.000000, 0.000000) (1.000000, 1.000000, 1.000000) `data/meshes/Sphere.glb`
add_bullet_box `Sphere` 25.0 (1.000000, 1.000000, 1.000000)

add_bullet_only_mesh `PhysicsOnlyCube` box 1.0 `data/meshes/PhysicsOnlyCube.glb`
```

## Coordinate System Conversion

Blender uses a right-handed coordinate system where:
- +X = Right
- +Y = Forward
- +Z = Up

vortxGE expects OpenGL-style coordinates:
- +X = Right
- +Y = Up
- -Z = Forward

A transformation matrix is applied during export to convert between these systems.

## GLB Export Settings

When exporting meshes to `.glb`, the following settings are used:

| Setting             | Value      |
|---------------------|------------|
| Format              | GLB        |
| Apply Transform     | True       |
| Export Normals      | True       |
| Export Tangents     | True       |
| Export Texcoords    | True       |
| Export Materials    | EXPORT     |
| Y-Up                | True       |

These ensure compatibility with vortxGE's expected model format.

## Notes

- All mesh names must be unique within a scene to avoid conflicts during export
- If an object has multiple physics properties, only the first one encountered will be used (unless specifically supported)
- This tool assumes all mesh data is properly oriented for export — if not, adjust manually before exporting

