"""The Royal Jelly: the hive's queen, a trembling mass of royal jelly.

Its own look: the DCSS royal jelly, doubled, with a queen bee curled in its dark core, honey
drips down its flanks and a glossier shine. Its spells are built from its own jelly and the
game's poof: a lobbed glob of jelly, a honey pool, a brood burst and a slam ring.
"""
import math
import os
import random

import creatures
import rig

NAME = "royal_jelly"
SOURCE = "spr_enemies/bosses/royal_jelly.png"
CANVAS = 112  # the doubled art is 64x96: room above it to breathe and rise when casting
SCALE = 2
SHADOW = None
CREATURE = dict(magic=creatures.GOLD, heavy=True)

RUNTIME = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "runtime"))

# The source's own colours, darkest first
OUTLINE = (0, 0, 0, 255)
BROWN = (127, 18, 4, 255)
RUST = (180, 46, 3, 255)
ORANGE = (236, 128, 0, 255)
AMBER = (244, 180, 2, 255)
YELLOW = (248, 214, 3, 255)
PALE = (254, 245, 8, 255)
WHITE = (255, 255, 255, 255)
HONEY = [PALE, YELLOW, AMBER, ORANGE, RUST, BROWN]


def _load(path):
    return rig.load(os.path.join(RUNTIME, path), None)


def _canvas_origin():
    return (CANVAS - 64) // 2, CANVAS - 96 - 4


def _queen(frame):
    """The queen bee, dark in the jelly's core: DCSS queen_bee, shrunk and recoloured to a
    silhouette seen through amber (dark body, dull gold stripes, pale wings, a dim red eye)."""
    bee = _load("spr_enemies/animals/queen_bee.png")
    fig, _ = creatures.split_shadow(bee, (16, 27, 9, 3))
    x0, y0, x1, y1 = rig.bounds(fig)
    fig = [row[x0:x1 + 1] for row in fig[y0:y1 + 1]]
    ox, oy = _canvas_origin()
    cx, cy = ox + 16 * 2 + 1, oy + 19 * 2  # the source's dark core, about (12..20, 14..23)
    h, w = len(fig), len(fig[0])
    s = 0.8
    out = rig.copy(frame)
    for y in range(round(h * s)):
        for x in range(round(w * s)):
            p = fig[min(h - 1, int(y / s))][min(w - 1, int(x / s))]
            if not p[3]:
                continue
            tx, ty = cx - round(w * s / 2) + x, cy - round(h * s / 2) + y
            under = out[ty][tx]
            if not under[3] or under == OUTLINE:
                continue
            r, g, b = p[:3]
            if r > 180 and g < 90:
                c = (200, 30, 10, 200)            # eye
            elif abs(r - g) < 30 and abs(g - b) < 30 and r > 120:
                c = (255, 236, 170, 110)          # wings
            elif r > 150 and g > 110 and b < 90:
                c = (176, 92, 8, 215)             # stripes and legs
            else:
                c = (58, 20, 2, 215)              # body
            out[ty][tx] = rig.blend(under, c)
    return out


def _drip(frame, x, y, length):
    """A honey drip running down the body from (x, y): a pale streak with a rust shadow on its
    right, ending in a bead with a highlight."""
    for i in range(length):
        rig.put(frame, x, y + i, "pyr", {"p": PALE, "y": YELLOW, "r": RUST})
    rig.put(frame, x - 1, y + length, "ryyar", {"r": RUST, "y": YELLOW, "a": AMBER})
    rig.put(frame, x - 1, y + length + 1, "rwyar", {"r": RUST, "w": WHITE, "y": YELLOW, "a": AMBER})
    rig.put(frame, x - 1, y + length + 2, " rrr", {"r": RUST})


def base(frame):
    out = _queen(frame)
    ox, oy = _canvas_origin()
    # honey running down from the crown of the dome
    for sx, sy, n in ((10, 9, 7), (21, 7, 5), (24, 12, 9)):
        _drip(out, ox + sx * 2, oy + sy * 2, n)
    return out


# ---------------------------------------------------------------------------------------------
# Spell effects
# ---------------------------------------------------------------------------------------------

def _outline(frame, colour=OUTLINE):
    """A 1 px dark outline around the figure, like the DCSS art."""
    out = rig.copy(frame)
    h, w = len(frame), len(frame[0])
    for y in range(h):
        for x in range(w):
            if frame[y][x][3]:
                continue
            if any(0 <= x + dx < w and 0 <= y + dy < h and frame[y + dy][x + dx][3] > 128
                   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                out[y][x] = colour
    return out


def _jelly_texture():
    """The jelly's own body pixels (source, below its highlight, above its tentacles)."""
    src = _load(SOURCE)
    return [row[6:26] for row in src[18:36]]


def _blob(size, squash=1.0, seed=0, shine=True):
    """A round blob of the jelly's own texture, outlined, with a highlight top left."""
    tex = _jelly_texture()
    th, tw = len(tex), len(tex[0])
    rnd = random.Random(seed)
    f = rig.blank(size)
    c = (size - 1) / 2
    r = size / 2 - 1.5
    ox, oy = rnd.randrange(0, 4), rnd.randrange(0, 4)
    for y in range(size):
        for x in range(size):
            dx, dy = (x - c) / squash, (y - c) * squash
            if dx * dx + dy * dy <= r * r:
                p = tex[(y + oy) % th][(x + ox) % tw]
                if not p[3] or sum(p[:3]) < 60:
                    p = ORANGE
                # darker toward the bottom right, like the source's lighting
                if dx + dy > r * 0.9:
                    p = rig.blend(p, (140, 30, 4, 110))
                f[y][x] = p
    f = _outline(f)
    if shine:
        hx, hy = round(c - r * 0.45 / squash), round(c - r * 0.55 * squash)
        rig.put(f, hx, hy, "ww", {"w": WHITE})
        rig.put(f, hx - 1, hy + 1, "wp", {"w": WHITE, "p": PALE})
    return f


def glob():
    """The lobbed glob: a wobbling ball of jelly with a drip trailing below."""
    frames = []
    for i, (sq, drip) in enumerate([(1.0, 2), (1.12, 3), (1.0, 2), (0.9, 1)]):
        f = _blob(20, sq, seed=3)
        # the trailing drip
        for k in range(drip):
            rig.put(f, 9, 17 + k, "ya", {"y": YELLOW, "a": AMBER}, behind=True)
        rig.put(f, 8, 17 + drip, " kk", {"k": OUTLINE}, behind=True)
        frames.append(f)
    return frames


def _puddle_mask(w, h, cx, cy, rx, ry, seed, wobble):
    """An irregular ellipse: its radius varies with a few gentle waves."""
    rnd = random.Random(seed)
    waves = [(k, rnd.uniform(0, 2 * math.pi), rnd.uniform(0.3, 1.0) * wobble / k) for k in (2, 3, 5, 7)]
    m = [[0.0] * w for _ in range(h)]  # 0 outside, else 1 at the centre down to ~0 at the rim
    for y in range(h):
        for x in range(w):
            ex, ey = (x - cx) / rx, (y - cy) / ry
            a = math.atan2(ey, ex)
            k = 1 + sum(amp * math.cos(n * a + ph) for n, ph, amp in waves)
            d = math.hypot(ex, ey) / k
            if d <= 1:
                m[y][x] = max(0.001, 1 - d)
    return m


def pool():
    """The honey pool on the ground: a flat, glossy puddle of the jelly, glinting and bubbling.
    Shaded in the source's ramp, rim to centre, dithered with the jelly's own texture."""
    w, h = 64, 40
    mask = _puddle_mask(w, h, 31.5, 19.5, 29, 16, seed=11, wobble=0.18)
    tex = _jelly_texture()
    th, tw = len(tex), len(tex[0])
    ramp = [BROWN, RUST, ORANGE, AMBER, YELLOW]
    base_f = rig.blank(w, h)
    for y in range(h):
        for x in range(w):
            v = mask[y][x]
            if not v:
                continue
            # light from the top left, like the source
            v = v + 0.12 * ((31.5 - x) / 29 + (19.5 - y) / 16)
            t = tex[(y + 3) % th][x % tw]
            jitter = ((t[0] + t[1]) / 510 - 0.5) * 0.25 if t[3] else 0
            i = max(0, min(len(ramp) - 1, int((v + jitter) * 5.5)))
            base_f[y][x] = ramp[i][:3] + (230,)
    base_f = _outline(base_f, (50, 14, 0, 230))
    frames = []
    glints = [(20, 10), (38, 12), (27, 22), (46, 20), (14, 18), (32, 8)]
    for i in range(4):
        f = rig.copy(base_f)
        for j, (gx, gy) in enumerate(glints):
            if (j + i) % 3 == 0:
                rig.put(f, gx, gy, " ww", {"w": WHITE})
                rig.put(f, gx, gy + 1, "wp", {"w": WHITE, "p": PALE})
            elif (j + i) % 3 == 1:
                rig.put(f, gx, gy, "p", {"p": PALE})
        # a bubble swelling and popping
        bx, by = [(24, 16), (40, 18), (30, 13), (44, 15)][i]
        k = {"k": RUST, "y": PALE, "w": WHITE}
        rig.put(f, bx, by, " kk ", k)
        rig.put(f, bx, by + 1, "kwyk", k)
        rig.put(f, bx, by + 2, "kyyk", k)
        rig.put(f, bx, by + 3, " kk ", k)
        frames.append(f)
    return frames


def _recolour(frame, ramp):
    """Maps a sprite's brightness onto ramp (darkest first), keeping its alpha."""
    out = rig.blank_like(frame)
    for y, row in enumerate(frame):
        for x, p in enumerate(row):
            if p[3]:
                lum = (p[0] * 3 + p[1] * 6 + p[2]) / 10 / 255
                c = ramp[min(len(ramp) - 1, int(lum * len(ramp)))]
                out[y][x] = c[:3] + (p[3],)
    return out


def brood():
    """The brood burst: the game's poof in honey colours, jelly droplets flung out, a ring."""
    poof = _load("spells/sprites/poof.png")
    poof = _recolour(poof, [BROWN, RUST, ORANGE, AMBER, YELLOW, PALE, WHITE])
    big = rig.scale2x(poof)  # 64x64
    size = 64
    rnd = random.Random(21)
    drops = [(2 * math.pi * k / 9 + rnd.uniform(-0.3, 0.3), rnd.uniform(0.7, 1.0)) for k in range(9)]
    drop = _blob(7, seed=2)
    frames = []
    for i, t in enumerate([0.2, 0.45, 0.7, 0.9, 1.0]):
        f = rig.blank(size)
        # the poof grows from the centre, then fades
        s = 0.5 + 0.5 * t
        layer = creatures.scale(big, s, s, (32, 32))
        fade = 1.0 if i < 3 else (0.55 if i == 3 else 0.25)
        layer = [[p[:3] + (round(p[3] * fade),) if p[3] else p for p in row] for row in layer]
        f = rig.paste(f, layer)
        if i >= 1:
            f = creatures.ring(f, creatures.GOLD, (32, 34), 10 + 18 * t, thickness=2)
        # droplets flung outward
        for a, sp in drops:
            r = 6 + 24 * t * sp
            x, y = round(32 + math.cos(a) * r), round(34 + math.sin(a) * r * 0.8 - 6 * math.sin(math.pi * t))
            if i < 4:
                f = rig.paste(f, drop, x - 3, y - 3)
        frames.append(f)
    return frames


def slam():
    """The Engulf telegraph and impact: a honey-coloured ring on the ground (drawn scaled)."""
    size = 64
    f = rig.blank(size, 48)
    f = creatures.ring(f, [PALE, (255, 200, 60, 200), (190, 90, 10, 150)], (32, 24), 28, thickness=3)
    return [f]


def splash():
    """A glob landing: the jelly splashing outward."""
    frames = []
    rnd = random.Random(8)
    drops = [(rnd.uniform(0, 2 * math.pi), rnd.uniform(0.6, 1.0)) for _ in range(7)]
    for i, t in enumerate([0.3, 0.65, 1.0]):
        f = rig.blank(40, 32)
        blob = _blob(16, 1.0 + 0.6 * t, seed=4, shine=False)
        f = rig.paste(f, blob, 12, 10 + round(3 * t))
        for a, sp in drops:
            r = 4 + 14 * t * sp
            x, y = round(20 + math.cos(a) * r), round(18 + math.sin(a) * r * 0.6 - 5 * math.sin(math.pi * t))
            rig.put(f, x, y, "ky", {"k": OUTLINE, "y": YELLOW})
            rig.put(f, x, y + 1, "ka", {"k": OUTLINE, "a": ORANGE})
        frames.append(f)
    return frames


def effects():
    return {
        "royal_jelly_glob": glob(),
        "royal_jelly_pool": pool(),
        "royal_jelly_brood": brood(),
        "royal_jelly_slam": slam(),
        "royal_jelly_splash": splash(),
    }
