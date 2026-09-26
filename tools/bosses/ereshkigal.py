"""Ereshkigal, Queen Below: ruler of the dead, a hooded shade in a robe of drifting souls, a bone
halo behind her and a verdigris crown of the underworld set with grave-light.

Built from DCSS's Ereshkigal: black robe turned deathly green-black, white bone halo and claws
turned bone white, the robe's starfield turned to green soul-lights; a crown, glowing eyes and a
few soul motes added at the art's own resolution before it is doubled.

Spell effects:
    ereshkigal_grave       a tombstone (features/sprites/tombstone) rising out of burst earth
    ereshkigal_grave_open  the grave bursting open, a soul light climbing out
    ereshkigal_chain       a soul chain link (spr_effects/constricted, recoloured)
    ereshkigal_soul        a soul drawn along the chain (spr_spells/death_channel's flame)
    ereshkigal_wail        a wailing spirit riding the wail's ring (spr_spells/cause_fear)
    ereshkigal_shimmer     souls streaming off her in her spectral form
"""
import math
import os
import random

import rig
import creatures
from creatures import DEATH

NAME = "ereshkigal"
SOURCE = "spr_enemies/bosses/ereshkigal.png"
CANVAS = 112  # 96 would cut the halo's tips and leave no room for the crown
SCALE = 2
SHADOW = None
CREATURE = dict(magic=DEATH, accent=DEATH, flying=True)

RUNTIME = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "runtime"))


def _load(path):
    return rig.load(os.path.join(RUNTIME, path), None)


# ---------------------------------------------------------------------------------------------
# Her look
# ---------------------------------------------------------------------------------------------

OUTLINE = (10, 22, 16, 255)
LINE = (18, 32, 26, 255)
ROBE_TOP = (46, 70, 58)
ROBE_BOTTOM = (28, 44, 36)
BONE = (228, 226, 200, 255)
BONE_SHADE = (170, 184, 156, 255)
BONE_LIGHT = (250, 250, 234, 255)
SOULS = [(40, 70, 54, 255), (84, 170, 104, 255), (150, 232, 150, 255), (214, 255, 216, 255)]

CROWN = {
    "k": (14, 24, 18, 255),     # outline
    "b": (70, 96, 70, 255),     # tarnished verdigris bronze
    "l": (130, 160, 108, 255),  # its highlight
    "e": (120, 230, 120, 255),  # grave-light gem
    "E": (236, 255, 226, 255),  # its core
}


def _is(p, rgb):
    return p[3] and p[:3] == rgb


def _dress(src):
    """Recolours the DCSS art (at its own resolution) and adds the crown, eyes and motes"""
    h, w = len(src), len(src[0])
    out = rig.blank_like(src)
    at = lambda x, y: src[y][x] if 0 <= x < w and 0 <= y < h else rig.CLEAR
    for y in range(h):
        for x in range(w):
            p = src[y][x]
            if not p[3]:
                continue
            nbrs = [at(x + 1, y), at(x - 1, y), at(x, y + 1), at(x, y - 1)]
            if _is(p, (0, 0, 0)):
                if any(not q[3] for q in nbrs):
                    out[y][x] = OUTLINE
                elif any(_is(q, (255, 255, 255)) for q in nbrs):
                    out[y][x] = LINE
                else:
                    t = y / h
                    out[y][x] = tuple(round(a + (b - a) * t) for a, b in zip(ROBE_TOP, ROBE_BOTTOM)) + (255,)
            elif _is(p, (255, 255, 255)):
                below, above = at(x, y + 1), at(x, y - 1)
                if not _is(below, (255, 255, 255)):
                    out[y][x] = BONE_SHADE
                elif not _is(above, (255, 255, 255)):
                    out[y][x] = BONE_LIGHT
                else:
                    out[y][x] = BONE
            else:  # the robe's starfield: souls
                lum = 0.3 * p[0] + 0.59 * p[1] + 0.11 * p[2]
                out[y][x] = SOULS[0 if lum < 70 else 1 if lum < 110 else 2 if lum < 160 else 3]
    # Crown on the hood (the head's crown is rows 5-6, x 19-24)
    for dy, run in enumerate(["l  E  l",
                              "bk e kb",
                              "blkekbl",
                              "bbbEbbb",
                              "kkkkkkk"]):
        rig.put(out, 18, dy, run, CROWN)
    # Eyes: the face's two pale pixels glow
    rig.put(out, 21, 7, "eE", CROWN)
    # Soul motes drifting near her
    solid = lambda x, y: at(x, y)[3] or out[y][x][3] if 0 <= x < w and 0 <= y < h else False
    spots = [(x, y) for y in range(8, h - 6) for x in range(1, w - 1)
             if not solid(x, y) and not any(solid(x + dx, y + dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1))
             and any(solid(x + dx, y + dy) for dx in (-2, 2) for dy in (-2, 0, 2))]
    for i, (x, y) in enumerate(random.Random(9).sample(spots, min(7, len(spots)))):
        out[y][x] = SOULS[2 + i % 2]
    return out


def base(frame):
    """Redrawn from SOURCE: the additions are made at the art's resolution so Scale2x smooths them too"""
    big = rig.scale2x(_dress(_load(SOURCE)))
    h, w = len(big), len(big[0])
    return rig.paste(rig.blank(CANVAS), big, (CANVAS - w) // 2, CANVAS - h - 4)


# ---------------------------------------------------------------------------------------------
# Effects
# ---------------------------------------------------------------------------------------------

DIRT = [(34, 24, 16, 255), (70, 52, 34, 255), (108, 82, 56, 255), (140, 112, 80, 255)]


def _outlined(layer, colour):
    """Adds a 1px outline around a layer's pixels"""
    out = rig.copy(layer)
    h, w = len(layer), len(layer[0])
    for y in range(h):
        for x in range(w):
            if not layer[y][x][3] and any(0 <= x + dx < w and 0 <= y + dy < h and layer[y + dy][x + dx][3]
                                          for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                out[y][x] = colour
    return out


def _crop(layer):
    x0, y0, x1, y1 = rig.bounds(layer)
    return [row[x0:x1 + 1] for row in layer[y0:y1 + 1]]


def _tombstone():
    """The tombstone in cold grave stone, its grass dead"""
    src = _load("features/sprites/tombstone.png")
    out = rig.blank_like(src)
    for y, row in enumerate(src):
        for x, p in enumerate(row):
            if not p[3]:
                continue
            r, g, b = p[:3]
            if r == g == b:  # stone: a little green in the grey
                out[y][x] = (max(0, r - 6), min(255, g + 4), max(0, b - 4), 255)
            elif g > r and g > b:  # grass: withered
                lum = g
                out[y][x] = (round(lum * 0.45 + 30), round(lum * 0.5 + 24), round(lum * 0.25 + 12), 255)
            else:  # earth
                out[y][x] = DIRT[0] if r + g < 70 else DIRT[1]
    return _crop(out)


def _clod(frame, x, y, size, shade):
    """A clod of earth: a small outlined lump"""
    x, y = round(x), round(y)
    h, w = len(frame), len(frame[0])
    cells = [(0, 0)] if size == 1 else [(0, 0), (1, 0), (0, 1), (1, 1)]
    for dx, dy in cells:
        for ox, oy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            px, py = x + dx + ox, y + dy + oy
            if 0 <= px < w and 0 <= py < h and not frame[py][px][3]:
                frame[py][px] = DIRT[0]
    for dx, dy in cells:
        if 0 <= x + dx < w and 0 <= y + dy < h:
            frame[y + dy][x + dx] = DIRT[shade] if (dx, dy) != (0, 0) else DIRT[min(3, shade + 1)]


def _mound(frame, cx, cy, rx, ry, glow=0.0):
    """A heap of broken earth, a crack of grave-light across it when glow > 0"""
    h, w = len(frame), len(frame[0])
    rnd = random.Random(3)
    for y in range(cy - ry - 1, cy + ry + 2):
        for x in range(cx - rx - 1, cx + rx + 2):
            if not (0 <= x < w and 0 <= y < h):
                continue
            d = ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2
            if d <= 1:
                top = y < cy - ry * 0.3
                c = DIRT[2] if top and rnd.random() < 0.7 else DIRT[1]
                if rnd.random() < 0.12:
                    c = DIRT[3]
                frame[y][x] = c
            elif d <= 1.45:
                frame[y][x] = DIRT[0]
    if glow > 0:
        for i, x in enumerate(range(cx - rx + 4, cx + rx - 3)):
            y = cy + round(math.sin(i * 0.9) * 1.2)
            c = DEATH[1] if (i + round(glow * 4)) % 3 else DEATH[0]
            frame[y][x] = rig.blend(frame[y][x], c[:3] + (round(255 * min(1, glow)),))


def _grave_rise():
    stone = _tombstone()
    sh, sw = len(stone), len(stone[0])
    ground = 42
    frames = []
    rnd = random.Random(5)
    throws = [(rnd.uniform(-1.6, 1.6), rnd.uniform(1.8, 3.2), rnd.choice((1, 2)), rnd.choice((1, 2))) for _ in range(12)]
    for i, t in enumerate([0.0, 0.2, 0.45, 0.7, 0.9, 1.0]):
        f = rig.blank(48)
        top = ground - round(sh * t) - 1
        layer = rig.blank(48)
        rig.paste(layer, stone, (48 - sw) // 2, top)
        for y in range(ground - 1, 48):  # still below the ground
            layer[y] = [rig.CLEAR] * 48
        rig.paste(f, layer)
        _mound(f, 24, ground - 1, 14 + (i > 0) * 2, 3 + (i > 1), glow=0.4 if i else 0.8)
        if 0 < i < 5:
            for vx, vy, size, shade in throws:
                s = i * 2.2
                _clod(f, 24 + vx * s * 2.4, ground - 4 - vy * s + 0.45 * s * s, size, shade)
        frames.append(f)
    return frames


def _soul_flame():
    """The flame of spr_spells/death_channel, cut from its icon and turned grave-green"""
    src = _load("spr_spells/death_channel.png")
    out = rig.blank_like(src)
    for y, row in enumerate(src):
        for x, p in enumerate(row):
            r, g, b, a = p
            if a and g - r > 20 and b > 20:
                out[y][x] = (min(255, round(g * 0.62 + r * 0.4)), g, round(g * 0.55 + r * 0.2), 255)
    return _outlined(_crop(out), (6, 26, 16, 255))


def _grave_open():
    stone = _tombstone()
    sh, sw = len(stone), len(stone[0])
    ground = 42
    soul = _soul_flame()
    frames = []
    rnd = random.Random(11)
    throws = [(rnd.uniform(-2.2, 2.2), rnd.uniform(2.4, 3.8), rnd.choice((1, 2)), rnd.choice((1, 2, 2))) for _ in range(16)]
    for i in range(6):
        f = rig.blank(48)
        rig.paste(f, stone, (48 - sw) // 2 + (1 if i == 0 else 0), ground - sh - 1)
        # the earth in front of it bursts, grave-light welling up
        _mound(f, 24, ground, 15, 4, glow=1.0 if i < 4 else 0.5)
        f = creatures.burst(f, DEATH, (24, ground), 10 + i * 2, n_rays=9, seed=i) if i < 3 else f
        if i < 5:
            for vx, vy, size, shade in throws:
                s = (i + 1) * 2.0
                _clod(f, 24 + vx * s * 2.2, ground - 2 - vy * s + 0.45 * s * s, size, shade)
        # a soul climbing out and away
        if 1 <= i:
            sl = soul if i < 4 else rig.tint(soul, (255, 255, 255), 0.3)
            rig.paste(f, sl, (48 - len(soul[0])) // 2, ground - len(soul) - (i - 1) * 5)
        frames.append(f)
    return frames


def _chain():
    """spr_effects/constricted's chain, turned to spectral green"""
    src = _crop(_load("spr_effects/constricted.png"))
    frames = []
    for bright in (0, 1):
        out = rig.blank_like(src)
        for y, row in enumerate(src):
            for x, p in enumerate(row):
                if not p[3]:
                    continue
                if p[:3] == (0, 0, 0):
                    out[y][x] = (6, 22, 14, 255)
                    continue
                t = p[1] / 220  # the orange's green channel: its shading
                lo, hi = ((60, 150, 90), (200, 250, 200)) if not bright else ((110, 210, 130), (236, 255, 230))
                out[y][x] = tuple(round(a + (b - a) * t) for a, b in zip(lo, hi)) + (255,)
        frames.append(out)
    return frames


def _soul():
    soul = _soul_flame()
    h, w = len(soul), len(soul[0])
    size = max(h, w) + 2
    frames = []
    for i in range(4):
        f = rig.blank(size)
        s = soul if i % 2 == 0 else creatures.scale(soul, 0.92, 1.08, (w // 2, h - 1))
        if i == 2:
            s = rig.tint(s, (230, 255, 230), 0.25)
        rig.paste(f, s, (size - w) // 2, (size - h) // 2)
        frames.append(f)
    return frames


def _wail():
    src = _load("spr_spells/cause_fear.png")
    colours = {
        (27, 4, 100): (16, 42, 30),
        (44, 9, 159): (52, 104, 74),
        (62, 11, 229): (120, 190, 140),
        (110, 73, 232): (196, 238, 204),
        (141, 124, 248): (244, 255, 240),
        (0, 0, 0): (14, 36, 24),
    }
    out = rig.blank_like(src)
    for y, row in enumerate(src):
        for x, p in enumerate(row):
            if p[3] and p[:3] in colours:
                out[y][x] = colours[p[:3]] + (255,)
    ghost = _crop(out)
    h, w = len(ghost), len(ghost[0])
    frames = []
    for i, (sx, sy) in enumerate([(1.0, 1.0), (0.94, 1.1), (1.0, 1.0), (1.06, 0.94)]):
        f = rig.blank(32)
        g = creatures.scale(ghost, sx, sy, (w // 2, h // 2))
        rig.paste(f, g, (32 - w) // 2, (32 - h) // 2)
        frames.append(f)
    return frames


def _shimmer():
    """Souls streaming upward off her spectral form: a loop drawn over her"""
    w, h = 64, 96
    rnd = random.Random(21)
    motes = [(rnd.uniform(6, w - 6), rnd.uniform(0, h), rnd.choice((1, 1, 2, 3)), rnd.uniform(0.6, 1.2)) for _ in range(20)]
    frames = []
    n = 6
    for i in range(n):
        f = rig.blank(w, h)
        for x, y, size, speed in motes:
            yy = (y - i * h / n * speed) % h
            xx = x + math.sin((yy / h) * 6 + x) * 2
            f = creatures.sparks(f, DEATH, [(xx, yy, size)], seed=i)
        frames.append(f)
    return frames


def effects():
    return {
        "ereshkigal_grave": _grave_rise(),
        "ereshkigal_grave_open": _grave_open(),
        "ereshkigal_chain": _chain(),
        "ereshkigal_soul": _soul(),
        "ereshkigal_wail": _wail(),
        "ereshkigal_shimmer": _shimmer(),
    }
