#!/usr/bin/env python3
"""
diff_frames.py — D2.2 (Plan.md): diff two VORTX_FRAME_DUMP JSON dumps.

Typical use: dump frame N once with VORTX_RENDERER=opengl and once with
VORTX_RENDERER=vulkan, then:

    tools/diff_frames.py gl.json vk.json

Identity across backends (driver-assigned GL ids vs sequential VK ids)
is carried by SIGNATURES, not raw ids:
  - programs: matched by the source-hash "sig" recorded at link time
  - textures: matched by (w, h, fmt, content-hash)
  - FBOs / VAOs: canonicalised to ordinals by order of first appearance

Uniform keys are normalised: "name[i]" element writes (GL style) are
merged with base-name array dumps (VK staging style) before comparison,
so both conventions compare equal.

Exit code: 0 = clean diff, 1 = mismatches found, 2 = usage/IO error.
"""

import argparse
import json
import sys


def load(path):
    with open(path) as f:
        return json.load(f)


def norm_uniforms(d):
    """{base: [flat floats/ints]} merging 'name[i]' element keys and
    base-name array dumps into one flat, element-ordered list."""
    bases = {}
    for key, vals in d.items():
        if key.endswith(']') and '[' in key:
            base, _, rest = key.partition('[')
            try:
                elem = int(rest[:-1])
            except ValueError:
                base, elem = key, None
        else:
            base, elem = key, None
        bases.setdefault(base, {})[elem] = vals
    out = {}
    for base, elems in bases.items():
        flat = []
        for e in sorted(elems, key=lambda x: -1 if x is None else x):
            flat.extend(elems[e])
        out[base] = flat
    return out


def tex_sig(texdb, texid):
    t = texdb.get(str(texid))
    if t is None:
        return None
    # content hash 0 = no pixel upload (FBO attachment) — geometry only
    return (t['w'], t['h'], t['fmt'], t['hash'] if t['hash'] != '0' * 16 else 0)


def canon_ordinals(draws, field):
    """Map raw id -> ordinal of first appearance in the draw stream."""
    order = {}
    for d in draws:
        raw = d[field]
        if raw not in order:
            order[raw] = len(order)
    return order


class Differ:
    def __init__(self, tol, max_report):
        self.tol = tol
        self.max_report = max_report
        self.mismatches = 0
        self.reported = 0
        self.bad_draws = set()

    def report(self, draw, msg):
        self.mismatches += 1
        self.bad_draws.add(draw)
        if self.reported < self.max_report:
            loc = f"draw {draw}" if draw is not None else "global"
            print(f"  MISMATCH {loc}: {msg}")
            self.reported += 1

    def cmp_floats(self, draw, what, a, b):
        if len(a) != len(b):
            self.report(draw, f"{what}: length {len(a)} vs {len(b)}")
            return
        for i, (x, y) in enumerate(zip(a, b)):
            if abs(x - y) > self.tol + self.tol * abs(y):
                self.report(draw, f"{what}: [{i}] {x!r} vs {y!r}")
                return  # one report per field is enough

    def cmp_ints(self, draw, what, a, b):
        if a != b:
            self.report(draw, f"{what}: {a} vs {b}")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('dump_a', help="first dump (e.g. OpenGL)")
    ap.add_argument('dump_b', help="second dump (e.g. Vulkan)")
    ap.add_argument('--tol', type=float, default=1e-5,
                    help="float tolerance (abs+rel, default 1e-5)")
    ap.add_argument('--max', type=int, default=25, dest='max_report',
                    help="max mismatch lines to print (default 25)")
    ap.add_argument('--skip-uniform', action='append', default=[],
                    metavar='NAME',
                    help="uniform to exclude from comparison (repeatable; "
                         "e.g. --skip-uniform mvpMatrix filters simulation "
                         "noise from wall-clock-driven physics objects)")
    args = ap.parse_args()

    try:
        A = load(args.dump_a)
        B = load(args.dump_b)
    except (OSError, json.JSONDecodeError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 2

    df = Differ(args.tol, args.max_report)
    print(f"A: {args.dump_a} backend={A.get('backend')} frame={A.get('frame')} "
          f"draws={len(A.get('draws', []))}")
    print(f"B: {args.dump_b} backend={B.get('backend')} frame={B.get('frame')} "
          f"draws={len(B.get('draws', []))}")

    # ---- program identity: sig <-> id per side
    sigA = {int(pid): p['sig'] for pid, p in A.get('programs', {}).items()}
    sigB = {int(pid): p['sig'] for pid, p in B.get('programs', {}).items()}
    onlyA = set(sigA.values()) - set(sigB.values())
    onlyB = set(sigB.values()) - set(sigA.values())
    for s in sorted(onlyA):
        print(f"  WARNING: program sig {s} only in A")
    for s in sorted(onlyB):
        print(f"  WARNING: program sig {s} only in B")

    texA = A.get('textures', {})
    texB = B.get('textures', {})

    drawsA = A.get('draws', [])
    drawsB = B.get('draws', [])
    if len(drawsA) != len(drawsB):
        df.report(None, f"draw count {len(drawsA)} vs {len(drawsB)}")

    fboA = canon_ordinals(drawsA, 'fbo')
    fboB = canon_ordinals(drawsB, 'fbo')
    vaoA = canon_ordinals(drawsA, 'vao')
    vaoB = canon_ordinals(drawsB, 'vao')

    # ---- per-draw comparison (aligned by sequence)
    for i, (da, db) in enumerate(zip(drawsA, drawsB)):
        for field in ('mode', 'count', 'first', 'indexed'):
            if da[field] != db[field]:
                df.report(i, f"{field}: {da[field]} vs {db[field]}")
        if da['vp'] != db['vp']:
            df.report(i, f"viewport: {da['vp']} vs {db['vp']}")
        for st_key in ('dt', 'dw', 'df', 'cull', 'cf', 'blend', 'bs', 'bd'):
            if da['st'][st_key] != db['st'][st_key]:
                df.report(i, f"state.{st_key}: {da['st'][st_key]} vs {db['st'][st_key]}")
        if fboA[da['fbo']] != fboB[db['fbo']]:
            df.report(i, f"fbo ordinal: {fboA[da['fbo']]} vs {fboB[db['fbo']]}")
        if vaoA[da['vao']] != vaoB[db['vao']]:
            df.report(i, f"vao ordinal: {vaoA[da['vao']]} vs {vaoB[db['vao']]}")
        sa = sigA.get(da['prog'])
        sb = sigB.get(db['prog'])
        if sa is not None and sb is not None and sa != sb:
            df.report(i, f"program sig: {sa} vs {sb}")
        elif (sa is None) != (sb is None):
            df.report(i, "program sig present on one side only")

        # texture bindings via signatures
        units = set(da['tex']) | set(db['tex'])
        for u in sorted(units, key=int):
            ta = da['tex'].get(u)
            tb = db['tex'].get(u)
            if ta is None or tb is None:
                df.report(i, f"tex unit {u}: bound on one side only")
                continue
            ga = tex_sig(texA, ta)
            gb = tex_sig(texB, tb)
            if ga != gb:
                df.report(i, f"tex unit {u}: sig {ga} vs {gb}")

        # uniforms (normalised)
        ua = norm_uniforms(da.get('u', {}))
        ub = norm_uniforms(db.get('u', {}))
        for name in sorted(set(ua) | set(ub)):
            if name in args.skip_uniform:
                continue
            if name not in ua:
                df.report(i, f"uniform '{name}': only in B")
            elif name not in ub:
                df.report(i, f"uniform '{name}': only in A")
            else:
                df.cmp_floats(i, f"uniform '{name}'", ua[name], ub[name])
        ia = norm_uniforms(da.get('iu', {}))
        ib = norm_uniforms(db.get('iu', {}))
        for name in sorted(set(ia) | set(ib)):
            if name not in ia:
                df.report(i, f"iuniform '{name}': only in B")
            elif name not in ib:
                df.report(i, f"iuniform '{name}': only in A")
            else:
                df.cmp_ints(i, f"iuniform '{name}'", ia[name], ib[name])

    # ---- summary
    n = min(len(drawsA), len(drawsB))
    print(f"compared {n} draws: {df.mismatches} field mismatches "
          f"in {len(df.bad_draws)} draws")
    if df.mismatches:
        if df.mismatches > df.reported:
            print(f"  ({df.mismatches - df.reported} more not shown; "
                  f"raise --max to see all)")
        print("RESULT: DIFFERENT")
        return 1
    print("RESULT: CLEAN")
    return 0


if __name__ == '__main__':
    sys.exit(main())
