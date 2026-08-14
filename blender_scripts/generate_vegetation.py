# ============================================================================
# generate_vegetation.py — example vegetation meshes for the terrain engine.
# Regenerate/extend with:
#   blender --background --python resources/vegetation/generate_vegetation.py -- <outdir>
# (Blender 5.x headless; exports Wavefront OBJ, Y-up, metres.)
# ============================================================================
# Blender 5.x headless: generate low-poly example vegetation meshes.
# Run: blender --background --python /tmp/gen_veg.py -- <outdir>
import bpy, sys, math, os
from mathutils import Vector

outdir = sys.argv[sys.argv.index("--") + 1] if "--" in sys.argv else "."
os.makedirs(outdir, exist_ok=True)

# Wipe default scene
bpy.ops.wm.read_factory_settings(use_empty=True)

def make_obj(name, verts, faces):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    return obj

def export(obj, filename):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.wm.obj_export(filepath=os.path.join(outdir, filename),
                          export_selected_objects=True,
                          export_materials=False,
                          export_normals=True,
                          export_uv=False)
    print("exported", filename, "verts:", len(obj.data.vertices), "polys:", len(obj.data.polygons))

def clear():
    for o in list(bpy.data.objects):
        bpy.data.objects.remove(o, do_unlink=True)

# ---------------------------------------------------------------- grass clump
# 5 tapered blades arranged in a fan, each a 2-quad strip with a slight bend
def grass_clump():
    verts, faces = [], []
    blades = 5
    for b in range(blades):
        ang = (b / blades) * math.pi  # fan over 180°
        lean = 0.25 + 0.15 * b
        dx, dz = math.cos(ang), math.sin(ang)
        h = 0.9 + 0.2 * math.sin(b * 2.3)      # blade height ~1m
        w0, w1 = 0.06, 0.02                    # base/tip half-widths
        px, pz = -dz, dx                        # blade width direction
        base = len(verts)
        # 3 rings × 2 verts (strip), bending along lean
        for r, (t, w) in enumerate([(0.0, w0), (0.6, (w0+w1)/2), (1.0, 0.0)]):
            bx = dx * lean * t * t
            bz = dz * lean * t * t
            by = h * t
            verts.append((bx + px*w, by, bz + pz*w))
            if w > 0:
                verts.append((bx - px*w, by, bz - pz*w))
        faces.append((base, base+1, base+2))
        faces.append((base+1, base+3, base+2))
        faces.append((base+2, base+3, base+4))
    return make_obj("grass_clump", verts, faces)

# ------------------------------------------------------------------- fern
def fern():
    verts, faces = [], []
    fronds = 7
    for f in range(fronds):
        ang = (f / fronds) * 2 * math.pi
        dx, dz = math.cos(ang), math.sin(ang)
        L = 0.7 + 0.15 * math.sin(f * 1.7)
        segs = 4
        base = len(verts)
        for s in range(segs + 1):
            t = s / segs
            cx, cz = dx * L * t, dz * L * t
            cy = 0.35 * math.sin(t * math.pi * 0.5)  # arc up then out
            w = 0.09 * (1 - t) + 0.01
            px, pz = -dz, dx
            verts.append((cx + px*w, cy, cz + pz*w))
            verts.append((cx - px*w, cy, cz - pz*w))
        for s in range(segs):
            a = base + s*2
            faces.append((a, a+1, a+2))
            faces.append((a+1, a+3, a+2))
    return make_obj("fern", verts, faces)

# ------------------------------------------------------------------- bush
def bush():
    # lumpy blob: 2 subdiv icosphere-ish via lat/long rings, squashed
    verts, faces = [], []
    rings, segs = 5, 8
    import random
    random.seed(11)
    for r in range(rings + 1):
        phi = math.pi * r / rings           # 0..pi
        for s in range(segs):
            th = 2 * math.pi * s / segs
            rad = 0.5 * (1 + 0.15 * random.uniform(-1, 1))
            x = rad * math.sin(phi) * math.cos(th)
            z = rad * math.sin(phi) * math.sin(th)
            y = rad * math.cos(phi) * 0.7 + 0.5   # squash, sit on ground
            verts.append((x, max(0.05, y), z))
    for r in range(rings):
        for s in range(segs):
            a = r*segs + s
            b = r*segs + (s+1) % segs
            c = (r+1)*segs + (s+1) % segs
            d = (r+1)*segs + s
            if r > 0: faces.append((a, b, d))
            if r < rings-1: faces.append((b, c, d))
    return make_obj("bush", verts, faces)

# ---------------------------------------------------------- broadleaf tree
def broadleaf():
    verts, faces = [], []
    # trunk: 6-sided tapered cylinder
    segs = 6
    for ring, (y, r) in enumerate([(0.0, 0.12), (1.8, 0.08)]):
        for s in range(segs):
            th = 2*math.pi*s/segs
            verts.append((r*math.cos(th), y, r*math.sin(th)))
    for s in range(segs):
        faces.append((s, (s+1)%segs, segs+s))
        faces.append(((s+1)%segs, segs+(s+1)%segs, segs+s))
    # canopy: 3 stacked squashed rings (low-poly blob)
    canopy = [(1.6, 0.9), (2.2, 1.2), (2.8, 0.8), (3.2, 0.05)]
    base_rings = 2
    for ri, (y, r) in enumerate(canopy):
        for s in range(segs):
            th = 2*math.pi*s/segs
            verts.append((r*math.cos(th), y, r*math.sin(th)))
    for ri in range(len(canopy)-1):
        r0 = (base_rings+ri)*segs
        r1 = (base_rings+ri+1)*segs
        for s in range(segs):
            faces.append((r0+s, r0+(s+1)%segs, r1+s))
            faces.append((r0+(s+1)%segs, r1+(s+1)%segs, r1+s))
    return make_obj("broadleaf", verts, faces)

# --------------------------------------------------------------- conifer
def conifer():
    verts, faces = [], []
    segs = 8
    # trunk
    for y, r in [(0.0, 0.09), (0.6, 0.07)]:
        for s in range(segs):
            th = 2*math.pi*s/segs
            verts.append((r*math.cos(th), y, r*math.sin(th)))
    for s in range(segs):
        faces.append((s, (s+1)%segs, segs+s))
        faces.append(((s+1)%segs, segs+(s+1)%segs, segs+s))
    # 3 cone tiers: each = ring + apex
    tiers = [(0.5, 1.1), (1.4, 0.85), (2.3, 0.55)]
    top = 3.4
    ring_start = 2
    for ti, (y, r) in enumerate(tiers):
        base = len(verts)
        for s in range(segs):
            th = 2*math.pi*s/segs
            verts.append((r*math.cos(th), y, r*math.sin(th)))
        apex_y = top if ti == len(tiers)-1 else y + (tiers[ti+1][0]-y)*0.95
        verts.append((0.0, apex_y, 0.0))
        ai = base + segs
        for s in range(segs):
            faces.append((base+s, base+(s+1)%segs, ai))
    return make_obj("conifer", verts, faces)

for gen, fname in [(grass_clump, "grass_clump.obj"),
                   (fern, "fern.obj"),
                   (bush, "bush.obj"),
                   (broadleaf, "broadleaf.obj"),
                   (conifer, "conifer.obj")]:
    obj = gen()
    export(obj, fname)
    clear()

print("ALL VEGETATION MESHES EXPORTED to", outdir)
