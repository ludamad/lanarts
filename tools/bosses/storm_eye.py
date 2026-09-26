"""Lom Lobon, the Storm Eye: a floating eye wrapped in a storm, electric blue and violet."""
import math
import os
import random

import creatures
import rig

NAME = "storm_eye"
SOURCE = "spr_enemies/bosses/lom_lobon.png"
CANVAS = 104  # 96 would crop the top of the eye once it bobs
SCALE = 2
SHADOW = (16, 44, 16.5, 4.5)
# Its storm crackles blue-violet; the cast glow is the lightning palette
STORM = [(255, 255, 255, 255), (178, 226, 255, 240), (122, 132, 255, 220), (112, 52, 196, 170)]
CREATURE = dict(magic=creatures.LIGHTNING, accent=STORM, flying=True)

RUNTIME = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "runtime")
OX, OY = 20, 4  # where SOURCE's pixel (0, 0) lands on the canvas

WHITE = (255, 255, 255, 255)
PALE = (200, 236, 255, 255)
BLUE = (110, 170, 255, 255)
VIOLET = (164, 110, 255, 255)
DEEP = (84, 36, 170, 255)
OUTLINE = (24, 6, 52, 255)

# The teal iris turns electric violet, the pupil a deep storm purple
IRIS = {
    (112, 224, 224, 255): (196, 168, 255, 255),
    (96, 192, 192, 255): (150, 104, 255, 255),
    (80, 160, 160, 255): (108, 60, 220, 255),
    (0, 32, 32, 255): (40, 6, 84, 255),
}
SPARK = (128, 255, 255, 255)  # the source's cyan motes


def _px(frame, x, y, c):
    """One source pixel (a 2x2 block on the canvas)"""
    for dy in (0, 1):
        for dx in (0, 1):
            X, Y = OX + 2 * x + dx, OY + 2 * y + dy
            if 0 <= X < len(frame[0]) and 0 <= Y < len(frame):
                frame[Y][X] = rig.blend(frame[Y][X], c)


def _arc(frame, points, glow=True):
    """A zigzag of lightning through source pixels: white core, blue-violet rim"""
    cells = []
    for (x0, y0), (x1, y1) in zip(points, points[1:]):
        n = max(abs(x1 - x0), abs(y1 - y0))
        for i in range(n + 1):
            cells.append((round(x0 + (x1 - x0) * i / max(1, n)), round(y0 + (y1 - y0) * i / max(1, n))))
    if glow:
        for x, y in cells:
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                if (x + dx, y + dy) not in cells:
                    _px(frame, x + dx, y + dy, (122, 110, 255, 120))
    for i, (x, y) in enumerate(cells):
        _px(frame, x, y, WHITE if i % 3 else PALE)


def base(frame):
    out = rig.copy(frame)
    h, w = len(out), len(out[0])
    rnd = random.Random(3)
    for y in range(h):
        for x in range(w):
            p = out[y][x]
            if p in IRIS:
                out[y][x] = IRIS[p]
            elif p == SPARK:
                # the motes flicker between electric blue, violet and white
                sx, sy = (x - OX) // 2, (y - OY) // 2
                out[y][x] = [BLUE, VIOLET, PALE, BLUE][(sx * 7 + sy * 3) % 4]
    # a glint in the pupil and a glowing rim round the iris
    _px(out, 14, 5, WHITE)
    for x, y in ((12, 5), (12, 7), (17, 8), (18, 6)):
        _px(out, x, y, (220, 200, 255, 150))
    # arcs of lightning crawling over its flanks
    _arc(out, [(0, 12), (-2, 15), (0, 17), (-3, 21)])
    _arc(out, [(31, 10), (33, 13), (31, 16), (34, 19)])
    _arc(out, [(4, 29), (2, 31), (4, 33)], glow=False)
    _arc(out, [(27, 28), (29, 30), (27, 33)], glow=False)
    return out


# --- spell effects ------------------------------------------------------------------------------

def _line(frame, pts, colour):
    h, w = len(frame), len(frame[0])
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        n = max(1, round(max(abs(x1 - x0), abs(y1 - y0))))
        for i in range(n + 1):
            x, y = round(x0 + (x1 - x0) * i / n), round(y0 + (y1 - y0) * i / n)
            if 0 <= x < w and 0 <= y < h:
                frame[y][x] = rig.blend(frame[y][x], colour)


def _bolt_points(p0, p1, rnd, jag, depth=3):
    """Midpoint displacement: a jagged path from p0 to p1"""
    pts = [p0, p1]
    for d in range(depth):
        new = [pts[0]]
        for a, b in zip(pts, pts[1:]):
            mx, my = (a[0] + b[0]) / 2, (a[1] + b[1]) / 2
            dx, dy = b[0] - a[0], b[1] - a[1]
            n = math.hypot(dx, dy) or 1
            off = rnd.uniform(-jag, jag) / (1.6 ** d)
            new += [(mx - dy / n * off, my + dx / n * off), b]
        pts = new
    return pts


def _glowed(frame, palette, width=2):
    return creatures.glow(frame, palette, width=width)


def _bolt(size, p0, p1, seed, jag=4.0, fork=True):
    rnd = random.Random(seed)
    f = rig.blank(size)
    pts = _bolt_points(p0, p1, rnd, jag)
    _line(f, pts, WHITE)
    if fork:  # a short fork off the middle
        a = pts[len(pts) // 2]
        ang = math.atan2(p1[1] - p0[1], p1[0] - p0[0]) + rnd.choice((-1, 1)) * rnd.uniform(0.6, 1.0)
        b = (a[0] + math.cos(ang) * 7, a[1] + math.sin(ang) * 7)
        _line(f, _bolt_points(a, b, rnd, 2, 2), PALE)
    return _glowed(f, STORM)


def storm_arc():
    """Segments of a lightning bolt, tiled along a path at runtime: 16 directions (frame k points
    k * 22.5 degrees clockwise from east, screen coordinates), then 16 more in a second shape."""
    frames = []
    for variant in range(2):
        for k in range(16):
            a = k * math.pi / 8
            dx, dy = math.cos(a) * 15.5, math.sin(a) * 15.5
            frames.append(_bolt(32, (15.5 - dx, 15.5 - dy), (15.5 + dx, 15.5 + dy), 100 + k * 7 + variant * 131,
                                jag=3.5, fork=variant == 1))
    return frames


def _disc(frame, cx, cy, r, colours):
    """A shaded ball: outline, body, highlight (colours darkest first)"""
    h, w = len(frame), len(frame[0])
    for y in range(h):
        for x in range(w):
            d = math.hypot(x + 0.5 - cx, y + 0.5 - cy)
            if d <= r:
                hl = math.hypot(x + 0.5 - (cx - r * 0.3), y + 0.5 - (cy - r * 0.3))
                if d > r - 1:
                    c = colours[0]
                elif hl < r * 0.35:
                    c = colours[3]
                elif hl < r * 0.7:
                    c = colours[2]
                else:
                    c = colours[1]
                frame[y][x] = c


def storm_spark():
    """The slow sparks of the Spark Spiral: a violet-cored mote with crackling rays"""
    frames = []
    for i in range(4):
        f = rig.blank(16)
        _disc(f, 8, 8, 3.6, [OUTLINE, DEEP, VIOLET, PALE])
        f = creatures.burst(f, STORM, (8, 8), 7, n_rays=4, seed=11 + i)
        f[7][7] = f[7][8] = WHITE
        frames.append(_glowed(f, [None, (122, 132, 255, 140)], width=1))
    return frames


def _recolour(img, ramp):
    """Maps a sprite's brightness onto ramp (dark to light), keeping its alpha"""
    out = rig.copy(img)
    for y, row in enumerate(img):
        for x, p in enumerate(row):
            if p[3]:
                lum = (p[0] * 0.3 + p[1] * 0.5 + p[2] * 0.2) / 255
                c = ramp[min(len(ramp) - 1, int(lum * len(ramp)))]
                out[y][x] = c[:3] + (p[3],)
    return out


def storm_orb():
    """The storm orbs of its second phase: the game's blue blast turned violet, arcs crawling on it"""
    src = rig.load(os.path.join(RUNTIME, "effects", "sprites", "blueblast.png"), None)
    ramp = [DEEP, (104, 56, 214, 255), (120, 110, 255, 255), (150, 170, 255, 255), PALE, WHITE]
    ball = _recolour(src, ramp)
    frames = []
    for i in range(4):
        f = creatures.scale(ball, 0.8 + 0.05 * (i % 2), 0.8 + 0.05 * (i % 2), (16, 16))
        rnd = random.Random(40 + i)
        for _ in range(2):
            a = rnd.uniform(0, 2 * math.pi)
            b = a + rnd.uniform(1.0, 2.0)
            p0 = (16 + math.cos(a) * 9, 16 + math.sin(a) * 9)
            p1 = (16 + math.cos(b) * 13, 16 + math.sin(b) * 13)
            _line(f, _bolt_points(p0, p1, rnd, 2.5, 2), WHITE)
        frames.append(_glowed(f, [None, (150, 170, 255, 200), (112, 52, 196, 120)]))
    return frames


def storm_blink():
    """The flash where it vanishes and reappears: the game's poof burst in the storm's colours"""
    src = rig.scale2x(rig.load(os.path.join(RUNTIME, "spells", "sprites", "poof.png"), None))
    ramp = [DEEP, VIOLET, (140, 150, 255, 255), PALE, WHITE, WHITE]
    src = [[p if p[3] >= 128 else rig.CLEAR for p in row] for row in src]  # drop its faint haze
    poof = _recolour(src, ramp)
    frames = []
    for i, (s, fade) in enumerate([(0.5, 1.0), (0.8, 1.0), (1.0, 0.9), (1.05, 0.6), (1.1, 0.35)]):
        f = creatures.scale(poof, s, s, (32, 32))
        core = rig.blank(64)  # the hollow heart of the burst flares white, fading as it spreads
        _disc(core, 32, 32, 15 * s, [VIOLET, (150, 170, 255, 255), PALE, WHITE])
        f = rig.paste(core, f)
        f = creatures.ring(f, STORM, (32, 32), 10 + 7 * i, 2)
        frames.append([[p[:3] + (round(p[3] * fade),) if p[3] else p for p in row] for row in f])
    return frames


def storm_mark():
    """Marks the first target of a Chain Lightning: a violet ring closing in, sparks on it"""
    frames = []
    for i in range(4):
        f = rig.blank(32)
        f = creatures.ring(f, [None, VIOLET, (178, 226, 255, 200)], (16, 18), 13 - 2 * i, 1)
        pts = []
        for k in range(4):
            a = k * math.pi / 2 + i * math.pi / 8
            pts.append((16 + math.cos(a) * (13 - 2 * i), 18 + math.sin(a) * (13 - 2 * i) / 1.35, 2))
        f = creatures.sparks(f, STORM, pts, i)
        frames.append(f)
    return frames


def effects():
    return {
        "storm_spark": storm_spark(),
        "storm_arc": storm_arc(),
        "storm_orb": storm_orb(),
        "storm_blink": storm_blink(),
        "storm_mark": storm_mark(),
    }
