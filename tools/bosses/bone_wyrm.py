"""The Bone Wyrm: an ancient dragon of bleached bone, cold blue soulfire burning in its eyes and ribs.

Its spell effects are cut from its own art: the shards it breathes and rains are its small back
spike, the spikes that erupt from the ground are its dorsal spikes.
"""
import math
import os
import random

import creatures
import rig

NAME = "bone_wyrm"
SOURCE = "spr_enemies/bosses/skulldragon.png"
CANVAS = 144
SCALE = 2  # doubled with Scale2x: it towers over the party
SHADOW = None  # no baked shadow: a soft one is made
CREATURE = dict(magic=creatures.FROST, heavy=True)

RUNTIME = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "runtime"))
ART = 64 * SCALE
OX, OY = (CANVAS - ART) // 2, CANVAS - ART - 4  # where build.py pastes the (scaled) SOURCE

# Grey of the source -> bleached bone: warm ivory highlights, cool blue-grey shadows
BONE = {
    255: (252, 250, 238), 240: (240, 236, 220), 224: (226, 222, 204), 208: (208, 204, 188),
    176: (172, 172, 164), 144: (134, 138, 144), 112: (98, 104, 118), 80: (72, 78, 96),
    64: (58, 62, 80), 48: (44, 48, 66), 32: (30, 32, 48), 16: (14, 14, 24),
}
# Soulfire, brightest first
SOUL = [(226, 248, 255, 255), (150, 214, 255, 255), (76, 146, 232, 255), (40, 84, 176, 255), (24, 42, 104, 255)]
# Attack and impact flashes: bone dust with a blue edge
DUST = [(255, 255, 248, 255), (226, 222, 204, 235), (140, 180, 230, 200), (50, 80, 150, 150)]

RIBS = (21, 25, 10.5, 8.5)  # ellipse (cx, cy, rx, ry) of the ribcage, SOURCE pixels


def bone(p):
    if p[3] == 0 or p[0] != p[1] or p[1] != p[2]:
        return p
    grey = min(BONE, key=lambda g: abs(g - p[0]))
    return BONE[grey] + (p[3],)


def recolour(frame):
    return [[bone(p) for p in row] for row in frame]


def load_source():
    return rig.load(os.path.join(RUNTIME, SOURCE), None)


def base(frame):
    src = load_source()
    for _ in range(SCALE // 2):
        src = rig.scale2x(src)
    out = recolour(frame)
    h, w = len(src), len(src[0])
    solid = lambda x, y: 0 <= x < w and 0 <= y < h and src[y][x][3] > 0
    # Soulfire in the ribcage: the dark gaps between the ribs burn blue, brightest at the heart
    cx, cy, rx, ry = (v * SCALE for v in RIBS)
    for y in range(h):
        for x in range(w):
            p = src[y][x]
            if not p[3] or p[0] != p[1]:
                continue
            d = math.hypot((x - cx) / rx, (y - cy) / ry)
            interior = all(solid(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            if d > 1 or not interior:
                continue
            if p[0] in (32, 48):
                c = SOUL[1] if d < 0.25 else SOUL[2] if d < 0.5 else SOUL[3] if d < 0.8 else SOUL[4]
            elif p[0] == 16 and d < 0.45:
                c = SOUL[4]
            else:
                continue
            out[y + OY][x + OX] = c
    # Eyes (the source's pink): cold blue flames, a faint lick of fire trailing up from each
    eyes = [(x, y) for y in range(h) for x in range(w) if src[y][x][3] and src[y][x][0] != src[y][x][1]]
    for x, y in eyes:
        out[y + OY][x + OX] = SOUL[0] if src[y][x][0] > 128 else SOUL[2]
    for x, y in eyes:
        if src[y][x][0] > 128 and (x, y - 1) not in eyes:
            for dx, dy, c in ((SCALE, -SCALE, SOUL[2][:3] + (200,)), (2 * SCALE, -2 * SCALE, SOUL[3][:3] + (120,))):
                out[y + dy + OY][x + dx + OX] = rig.blend(out[y + dy + OY][x + dx + OX], c)
    return out


# ---------------------------------------------------------------------------------------------
# Effects
# ---------------------------------------------------------------------------------------------

def _flood(src, seed, box):
    """The connected figure pixels from seed inside box (x0, y0, x1, y1), as a layer"""
    x0, y0, x1, y1 = box
    out = rig.blank(x1 - x0 + 1, y1 - y0 + 1)
    stack, seen = [seed], set()
    while stack:
        x, y = stack.pop()
        if (x, y) in seen or not (x0 <= x <= x1 and y0 <= y <= y1) or not src[y][x][3]:
            continue
        seen.add((x, y))
        out[y - y0][x - x0] = src[y][x]
        stack += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    return out


def _centred(layer, size):
    """layer pasted in the middle of a size (w, h) frame"""
    x0, y0, x1, y1 = rig.bounds(layer)
    w, h = size
    out = rig.blank(w, h)
    return rig.paste(out, layer, (w - (x1 + x0 + 1)) // 2, (h - (y1 + y0 + 1)) // 2)


def _outline(layer, colour=BONE[16]):
    """Closes a cut-out part with a dark rim where it was cut from the body"""
    h, w = len(layer), len(layer[0])
    out = rig.copy(layer)
    for y in range(h):
        for x in range(w):
            if layer[y][x][3]:
                continue
            if any(0 <= x + dx < w and 0 <= y + dy < h and layer[y + dy][x + dx][3] and layer[y + dy][x + dx][:3] != colour
                   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                out[y][x] = colour + (255,)
    return out


def dorsal_spike(turn=0):
    """The Wyrm's big back spike (leaning right), doubled, pointing up (then turned clockwise by turn)"""
    src = recolour(load_source())
    spike = rig.scale2x(_flood(src, (39, 10), (32, 10, 40, 17)))
    spike = _centred(spike, (32, 32))
    return _outline(rig.rotate(spike, -36 + turn, (16, 16)))


def small_spike():
    """Its small back spike, pointing up: a bone shard"""
    src = recolour(load_source())
    spike = rig.scale2x(_flood(src, (44, 15), (39, 15, 46, 18)))
    spike = _centred(spike, (24, 24))
    return _outline(rig.rotate(spike, -40, (12, 12)))


def splinter():
    """The small back spike at its own size, for flying splinters"""
    src = recolour(load_source())
    return _outline(_centred(_flood(src, (44, 15), (39, 15, 46, 18)), (16, 16)))


def shard_frames(n=16):
    """A bone shard (the dorsal spike) with a frost glow, one 32x32 frame per direction: frame i
    points at angle 2*pi*i/n (0 = east, clockwise, as on screen)"""
    # up -> east is a quarter turn clockwise
    return [creatures.glow(dorsal_spike(90 + 360 * i / n), creatures.FROST, width=1, strength=0.7) for i in range(n)]


def _ellipse_pts(cx, cy, r, squash=1.35):
    return lambda x, y: math.hypot(x - cx, (y - cy) * squash)


def marker_frames(w=40, h=32):
    """Ground telegraph: a frost ring on the ground with bone-white ticks, an inner ring closing in"""
    cx, cy = w / 2 - 0.5, h / 2 - 0.5
    frames = []
    for i in range(6):
        f = rig.blank(w, h)
        d = _ellipse_pts(cx, cy, 0)
        inner = 15 - i * 2.2
        pulse = 1 if i % 2 else 0
        for y in range(h):
            for x in range(w):
                r = d(x, y)
                if 16.5 <= r < 18:
                    f[y][x] = SOUL[1][:3] + (230,)
                elif 18 <= r < 19.2:
                    f[y][x] = SOUL[3][:3] + (170,)
                elif r < 16.5:
                    a = round(40 + 30 * pulse * (1 - r / 16.5))
                    f[y][x] = SOUL[2][:3] + (a,)
                if inner - 1 <= r < inner and inner > 1:
                    f[y][x] = rig.blend(f[y][x], SOUL[0][:3] + (200,))
        # four bone ticks pointing inwards
        for ang in (0, 90, 180, 270):
            a = math.radians(ang + 45)
            for k in range(3):
                rr = 16 - k * 1.5
                x, y = round(cx + math.cos(a) * rr), round(cy + math.sin(a) * rr / 1.35)
                f[y][x] = DUST[0] if k else DUST[1]
        frames.append(f)
    return frames


def impact_frames(w=48, h=40):
    """Shards hitting the ground: a flash, a burst of bone dust and flying splinters"""
    rnd = random.Random(5)
    cx, cy = w // 2, h // 2 + 4
    shard = splinter()
    frames = []
    for i in range(5):
        f = rig.blank(w, h)
        if i < 3:
            f = creatures.burst(f, DUST, (cx, cy), 8 + i * 5, 10, seed=3 + i)
        if i == 0:
            f = creatures.sparks(f, creatures.FROST, [(cx, cy, 3), (cx - 3, cy + 1, 2), (cx + 3, cy - 1, 2)], 1)
        f = creatures.ring(f, [None, SOUL[1][:3] + (200 - i * 40,), SOUL[3][:3] + (140 - i * 25,)], (cx, cy), 5 + i * 4)
        # splinters flying out
        for k in range(5):
            a = 2 * math.pi * k / 5 + 0.4
            r = 4 + i * 4
            x = round(cx + math.cos(a) * r) - 8
            y = round(cy + math.sin(a) * r * 0.75 - (6 * i - i * i)) - 8
            s = rig.rotate(shard, (k * 70 + i * 50) % 360, (8, 8)) if i < 4 else rig.blank(16, 16)
            rig.paste(f, s, x, y)
        # dust hanging on after
        pts = [(cx + rnd.uniform(-10, 10), cy + rnd.uniform(-5, 4), 1 + (i < 3)) for _ in range(4 + 2 * i)]
        f = creatures.sparks(f, DUST[1:], pts, i)
        frames.append(f)
    return frames


def ring_frames(w=160, h=128, n=6):
    """A shockwave on the ground: a ring of frost and bone dust racing out"""
    cx, cy = w / 2 - 0.5, h / 2 - 0.5
    frames = []
    for i in range(n):
        f = rig.blank(w, h)
        r = 14 + (i + 1) * (74 - 14) / n
        fade = 1 - i / n
        pal = [None, SOUL[0][:3] + (round(230 * fade),), SOUL[2][:3] + (round(170 * fade),)]
        f = creatures.ring(f, pal, (cx, cy), r, thickness=3)
        f = creatures.ring(f, [None, SOUL[3][:3] + (round(110 * fade),), (0, 0, 0, 0)], (cx, cy), r - 5, thickness=2)
        # bone dust kicked up along the front
        rnd = random.Random(i)
        pts = []
        for k in range(18):
            a = rnd.uniform(0, 2 * math.pi)
            pts.append((cx + math.cos(a) * (r - 2), cy + math.sin(a) * (r - 2) / 1.35, 1 + (rnd.random() < 0.4)))
        f = creatures.sparks(f, DUST, pts, i)
        frames.append(f)
    return frames


def sweep_mark(w=176, h=136, r=80):
    """Tail Sweep telegraph: the reach of the tail marked on the ground"""
    cx, cy = w / 2 - 0.5, h / 2 - 0.5
    f = rig.blank(w, h)
    for y in range(h):
        for x in range(w):
            d = math.hypot(x - cx, (y - cy) * 1.35)
            ang = math.atan2((y - cy) * 1.35, x - cx)
            if r - 2 <= d < r:
                dash = int((ang + math.pi) / (2 * math.pi) * 48) % 2 == 0
                f[y][x] = SOUL[1][:3] + (230 if dash else 120,)
            elif r - 5 <= d < r - 2:
                f[y][x] = SOUL[3][:3] + (90,)
            elif d < r - 5:
                f[y][x] = SOUL[3][:3] + (round(20 + 35 * (d / r) ** 2),)
    return [f]


def spike_frames(w=32, h=40):
    """A bone spike bursting from the ground: a crack, the tip, the spike, then it crumbles"""
    big = dorsal_spike()
    big = big  # 32x32
    x0, y0, x1, y1 = rig.bounds(big)
    sw = x1 - x0 + 1
    gx, gy = w // 2, h - 7  # ground point
    small = small_spike()
    frames = []

    def crack(f, size):
        for dx in range(-size, size + 1):
            y = gy + (1 if abs(dx) % 3 == 1 else 0)
            if 0 <= gx + dx < w:
                f[y][gx + dx] = BONE[16] + (255,)
        for dx in range(-size // 2, size // 2 + 1):
            if 0 <= gx + dx < w and abs(dx) % 2 == 0:
                f[gy - 1][gx + dx] = SOUL[2][:3] + (200,)
        return f

    def rising(layer, px, rise):
        """layer with its base at (px, ground), risen by rise (0..1) of its height, cut at the ground"""
        bx0, by0, bx1, by1 = rig.bounds(layer)
        out = rig.blank(w, h)
        height = round((by1 - by0 + 1) * rise)
        rig.paste(out, layer, px - (bx0 + bx1) // 2, gy + 2 - by0 - height)
        for y in range(gy + 1, h):
            out[y] = [rig.CLEAR] * w
        return out

    for i, rise in enumerate([0, 0.4, 1.0, 0.92, 0.92, -1]):
        f = rig.blank(w, h)
        f = crack(f, 3 + min(i, 3) * 2)
        if rise > 0:
            layer = rising(big, gx + 2, rise)
            if i >= 2:  # a smaller spike beside it
                layer = rig.paste(rising(small, gx - 8, rise * 0.9), layer)
            if i == 2:
                layer = rig.flash(layer, 0.3)
            f = rig.paste(f, layer)
            if i == 2:
                f = creatures.burst(f, DUST, (gx, gy - 2), 12, 8, seed=2)
        elif rise < 0:
            f = rig.paste(f, creatures.dissolve(frames[-1], 0.6, DUST, 9))
        frames.append(f)
    return frames


def gather_frames(size=32):
    """Soulfire gathering in the Wyrm's jaws before the breath"""
    c = size / 2 - 0.5
    frames = []
    rnd = random.Random(4)
    for i in range(4):
        f = rig.blank(size, size)
        pts = []
        for k in range(7):
            a = rnd.uniform(0, 2 * math.pi)
            r = 13 - i * 3 + rnd.uniform(-1, 1)
            pts.append((c + math.cos(a) * r, c + math.sin(a) * r, 2 if k % 2 else 1))
        pts.append((c, c, 1 + min(2, i)))
        f = creatures.sparks(f, creatures.FROST, pts, i)
        if i >= 2:
            f = creatures.glow(f, creatures.FROST, width=1, strength=0.6)
        frames.append(f)
    return frames


def effects():
    return {
        "bone_wyrm_shard": shard_frames(),
        "bone_wyrm_marker": marker_frames(),
        "bone_wyrm_impact": impact_frames(),
        "bone_wyrm_ring": ring_frames(),
        "bone_wyrm_sweep_mark": sweep_mark(),
        "bone_wyrm_spike": spike_frames(),
        "bone_wyrm_gather": gather_frames(),
    }
