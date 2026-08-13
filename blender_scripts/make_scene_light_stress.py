"""L5.4 light-stress scene: 64 small colored point lights in a grid
over a floor with scattered pillars. Point lights are cast_shadows=0
(non-casting; L5.3 keeps casters few). One sun for base shadows.

Run: blender -b --factory-startup -P blender_scripts/make_scene_light_stress.py
"""
import bpy, math, colorsys

for o in list(bpy.data.objects):
    bpy.data.objects.remove(o, do_unlink=True)

def mat(name, rgb, rough=0.9):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = (*rgb, 1.0)
    b.inputs["Roughness"].default_value = rough
    return m

m_floor = mat("StressFloorMat", (0.55, 0.55, 0.55))
m_pil   = mat("StressPillarMat", (0.35, 0.36, 0.40))

def cube(name, loc, scale, material, mesh_name=None):
    bpy.ops.mesh.primitive_cube_add(location=loc)
    o = bpy.context.view_layer.objects.active
    o.name = name
    o.scale = scale
    # 5.2: pass all flags or location gets baked to origin
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if mesh_name: o.data.name = mesh_name
    o.data.materials.append(material)
    return o

cube("Floor", (0, 0, 0), (15, 15, 0.1), m_floor, "StressFloor")

# scattered pillars (one shared mesh data)
p = cube("Pillar_0", (-6, -6, 1.0), (0.4, 0.4, 1.0), m_pil, "StressPillar")
for i, (x, y) in enumerate([(-2, 3), (3, -4), (6, 6), (-7, 5), (5, -7),
                            (0, -8), (-4, -1), (7, 0), (1, 7), (-6, -7)]):
    bpy.ops.mesh.primitive_cube_add()
    o = bpy.context.view_layer.objects.active
    o.name = f"Pillar_{i+1}"
    o.data = p.data
    o.location = (x, y, 1.0)

# 8x8 grid of small point lights at 2.5m height, rainbow colors.
# Blender Z-up: engine (x, y=height, z=-y_blender).
n = 0
for ix in range(8):
    for iy in range(8):
        x = -10.5 + ix * 3.0
        y = -10.5 + iy * 3.0
        ld = bpy.data.lights.new(f"Stress_{n:02d}", type='POINT')
        hue = (ix + iy * 8) / 64.0
        ld.color = colorsys.hsv_to_rgb(hue, 0.85, 1.0)
        ld.energy = 4.0
        lo = bpy.data.objects.new(f"Stress_{n:02d}", ld)
        bpy.context.scene.collection.objects.link(lo)
        lo.location = (x, y, 2.5)
        lo["cast_shadows"] = False   # L5.3: non-casting
        n += 1

# one sun for base lighting + shadows
sun_data = bpy.data.lights.new("Sun", type='SUN')
sun_data.energy = 1.5
sun = bpy.data.objects.new("Sun", sun_data)
bpy.context.scene.collection.objects.link(sun)
sun.rotation_euler = (math.radians(50), 0, math.radians(-20))

out = "/workspace/Games/vortxGE/demos/scenes_src/light_stress.blend"
bpy.ops.wm.save_as_mainfile(filepath=out)
print("STRESS SAVED:", out, "objects:", len(bpy.context.scene.objects),
      "point lights:", n)
