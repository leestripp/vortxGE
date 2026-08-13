import bpy, math

# --- clean scene: remove factory-default objects (Cube/Light/Camera) ---
for o in list(bpy.data.objects):
    bpy.data.objects.remove(o, do_unlink=True)

def mat(name, rgb, rough=0.9):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = (*rgb, 1.0)
    b.inputs["Roughness"].default_value = rough
    return m

m_floor = mat("FloorMat", (0.75, 0.75, 0.72))
m_wall  = mat("WallMat",  (0.92, 0.90, 0.85))
m_pil   = mat("PillarMat",(0.45, 0.46, 0.50))
m_sph   = mat("SphereMat",(0.80, 0.25, 0.20))

def cube(name, loc, scale, material, mesh_name=None):
    bpy.ops.mesh.primitive_cube_add(location=loc)
    o = bpy.context.view_layer.objects.active
    o.name = name
    o.scale = scale
    # bake scale only into mesh data (5.2: pass all flags explicitly,
    # otherwise location is applied too and the object jumps to origin)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if mesh_name: o.data.name = mesh_name
    o.data.materials.append(material)
    return o

# floor 40x40, wall at back (+Y in Blender = -Z in vortxGE)
floor = cube("Floor", (0, 0, 0), (20, 20, 0.1), m_floor, "GymFloor")
wall  = cube("Wall",  (0, 8, 3),  (8, 0.15, 3), m_wall, "GymWall")

# pillars share one mesh data (linked) -> single glb, 4 scene objects
p1 = cube("Pillar_near",  (-3.0, 7.5, 1.5), (0.25, 0.25, 1.5), m_pil, "Pillar")
for name, y in [("Pillar_mid", 6.5), ("Pillar_far", 5.0), ("Pillar_xfar", 2.0)]:
    bpy.ops.mesh.primitive_cube_add(location=(0.0, 0.0, 0.0))
    o = bpy.context.view_layer.objects.active
    o.name = name
    o.data = p1.data  # link mesh data
    o.location = (p1.location.x + 2.0 + len(name)*0.0, y, 1.5)  # spread along X too
# fix: distinct X so shadows don't overlap on the wall
xs = {"Pillar_near": -3.0, "Pillar_mid": -1.0, "Pillar_far": 1.0, "Pillar_xfar": 3.0}
for name, x in xs.items():
    bpy.data.objects[name].location.x = x

# sphere resting on the floor (contact hardening check), well in front
bpy.ops.mesh.primitive_uv_sphere_add(radius=0.5, location=(0.0, 1.0, 0.6))
sph = bpy.context.view_layer.objects.active
sph.name = "ContactSphere"
sph.data.name = "ContactSphere"
sph.data.materials.append(m_sph)

# sun: rays travel toward +Y (engine -Z, onto the wall), 35 deg elevation
sun_data = bpy.data.lights.new("Sun", type='SUN')
sun_data.energy = 3.0
sun = bpy.data.objects.new("Sun", sun_data)
bpy.context.scene.collection.objects.link(sun)
sun.rotation_euler = (math.radians(70), 0, 0)  # ~20 deg elevation: long shadows for penumbra testing

# spot for spot-PCSS checks, aimed at the floor in front of the wall
sp_data = bpy.data.lights.new("Spot", type='SPOT')
sp_data.energy = 800.0
sp_data.spot_size = math.radians(50)
spot = bpy.data.objects.new("Spot", sp_data)
bpy.context.scene.collection.objects.link(spot)
spot.location = (0.0, 2.0, 6.0)
spot.rotation_euler = (0, 0, 0)  # straight down

out = "/workspace/Games/vortxGE/demos/scenes_src/pcss_gym.blend"
bpy.ops.wm.save_as_mainfile(filepath=out)
print("GYM SAVED:", out, "objects:", len(bpy.context.scene.objects))
