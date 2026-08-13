"""Headless .blend -> .vgs export (Godot-style asset pipeline).

Usage:
    blender -b scene.blend -P blender_scripts/export_blend_headless.py -- /abs/out.vgs

Imports the exporter module directly (no addon registration needed) and
calls write_vortxge_scene on the loaded file's context.
"""
import os
import sys

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import export_scene

argv = sys.argv[sys.argv.index("--") + 1:]
out = os.path.abspath(argv[0])
os.makedirs(os.path.dirname(out), exist_ok=True)

ok = export_scene.write_vortxge_scene(bpy.context, out)
print("HEADLESS-EXPORT", "OK" if ok else "FAILED", out)
sys.exit(0 if ok else 1)
