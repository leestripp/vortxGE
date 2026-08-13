#!/usr/bin/env python3
"""Re-export stale .vgs scenes from their .blend sources (Godot-style).

Source of truth: demos/scenes_src/<name>.blend
Output:          demos/data/<name>.vgs (+ meshes into demos/data/meshes/)

A scene is re-exported when the .blend is newer than the .vgs (or the
.vgs is missing). Set VORTX_BLENDER to override the blender binary.
"""
import glob
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "demos", "scenes_src")
DST = os.path.join(ROOT, "demos", "data")
EXPORTER = os.path.join(ROOT, "blender_scripts", "export_blend_headless.py")
BLENDER = os.environ.get("VORTX_BLENDER", "blender")


def main() -> int:
    blends = sorted(glob.glob(os.path.join(SRC, "*.blend")))
    if not blends:
        print("export_scenes: no demos/scenes_src/*.blend found")
        return 0
    rc = 0
    for b in blends:
        name = os.path.splitext(os.path.basename(b))[0]
        vgs = os.path.join(DST, f"{name}.vgs")
        if os.path.exists(vgs) and os.path.getmtime(vgs) >= os.path.getmtime(b):
            print(f"export_scenes: up-to-date {name}")
            continue
        print(f"export_scenes: {name}.blend -> data/{name}.vgs")
        r = subprocess.run(
            [BLENDER, "-b", b, "-P", EXPORTER, "--", vgs],
            capture_output=True, text=True, timeout=300)
        if r.returncode != 0 or not os.path.exists(vgs):
            print(f"export_scenes: FAILED {name}")
            print((r.stdout or "")[-1500:])
            print((r.stderr or "")[-1500:])
            rc = 1
        else:
            print(f"export_scenes: ok {name}")
    return rc


if __name__ == "__main__":
    sys.exit(main())
