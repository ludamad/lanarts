"""Whole-body animations for creature sprites of any size (bosses and other monsters).

Class sprites are cut into traced layers (see animations.py); creatures are animated as a whole:
breathing and bobbing, leaning, squash and stretch, glows grown from the silhouette, sparks and a
dissolve. A creature needs only its sprite and a few numbers (see Creature).

    frames = Creature(rig.load(path, None), magic=animations.FIRE).all()
"""
import math
import random

import rig
from animations import FIRE, FROST, LIGHTNING, DEATH, GOLD, MAGIC  # noqa: F401 (palettes for callers)

CLEAR = rig.CLEAR


SHADOW_ALPHA = 254  # below: a translucent baked drop shadow, not the figure


def split_shadow(frame, ellipse=None, colour=None):
    """-> (figure, shadow). DCSS art bakes a drop shadow under the figure, translucent or opaque in
    one dark colour. ellipse (cx, cy, rx, ry) marks where it lies; pixels in it of the shadow colour
    (colour, or the darkest common colour there) are the shadow. Translucent pixels always are."""
    h, w = len(frame), len(frame[0])
    inside = lambda x, y: ellipse and ((x - ellipse[0]) / ellipse[2]) ** 2 + ((y - ellipse[1]) / ellipse[3]) ** 2 <= 1
    if ellipse and colour is None:
        counts = {}
        for y in range(h):
            for x in range(w):
                p = frame[y][x]
                if p[3] == 255 and sum(p[:3]) < 90 and inside(x, y):
                    counts[p] = counts.get(p, 0) + 1
        colour = max(counts, key=counts.get) if counts else None
    figure, shadow = rig.blank_like(frame), rig.blank_like(frame)
    for y, row in enumerate(frame):
        for x, p in enumerate(row):
            if p[3] > SHADOW_ALPHA and not (p == colour and inside(x, y)):
                figure[y][x] = p
            elif p[3] > 8:
                shadow[y][x] = p[:3] + (min(p[3], 150),)
    return figure, shadow


def _exterior(frame):
    """Empty pixels reachable from the frame's edge without crossing the figure (not holes in it)"""
    h, w = len(frame), len(frame[0])
    seen = [[False] * w for _ in range(h)]
    stack = [(x, y) for x in range(w) for y in (0, h - 1)] + [(x, y) for y in range(h) for x in (0, w - 1)]
    while stack:
        x, y = stack.pop()
        if 0 <= x < w and 0 <= y < h and not seen[y][x] and frame[y][x][3] <= 8:
            seen[y][x] = True
            stack += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    return seen


def _mask(frame):
    return [[p[3] > 8 for p in row] for row in frame]


def _dilate(mask, n):
    h, w = len(mask), len(mask[0])
    out = mask
    for _ in range(n):
        prev = out
        out = [[prev[y][x] or any(0 <= y + dy < h and 0 <= x + dx < w and prev[y + dy][x + dx]
                                  for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                for x in range(w)] for y in range(h)]
    return out


def glow(frame, palette, width=2, strength=1.0):
    """A halo around the silhouette, brightest nearest it, drawn behind the figure."""
    base = _mask(frame)
    out = rig.copy(frame)
    rings = [base]
    for i in range(width):
        rings.append(_dilate(rings[-1], 1))
    h, w = len(frame), len(frame[0])
    outside = _exterior(frame)
    for i in range(1, width + 1):
        colour = palette[min(i, len(palette) - 1)]
        c = colour[:3] + (round(colour[3] * strength),)
        for y in range(h):
            for x in range(w):
                if rings[i][y][x] and not rings[i - 1][y][x] and outside[y][x]:
                    out[y][x] = c
    return out


def scale(layer, sx, sy, anchor):
    """Nearest-neighbour scale around anchor (x, y): small factors for squash and stretch."""
    h, w = len(layer), len(layer[0])
    out = rig.blank_like(layer)
    ax, ay = anchor
    for y in range(h):
        for x in range(w):
            ix = round(ax + (x - ax) / sx)
            iy = round(ay + (y - ay) / sy)
            if 0 <= ix < w and 0 <= iy < h:
                out[y][x] = layer[iy][ix]
    return out


def breathe(layer, row, amount):
    """Grows (amount > 0) or shrinks the part above row by whole pixels, like a chest rising."""
    h = len(layer)
    out = rig.blank_like(layer)
    for y in range(h):
        if y >= row:
            src = y
        else:
            src = min(row, y + amount) if amount > 0 else max(0, y + amount)
        out[y] = layer[src][:] if 0 <= src < h else [CLEAR] * len(layer[0])
    return out


def sparks(frame, palette, points, seed):
    """Motes of light at points [(x, y, size)], placed over the figure."""
    rnd = random.Random(seed)
    out = rig.copy(frame)
    h, w = len(frame), len(frame[0])
    for x, y, size in points:
        x, y = round(x), round(y)
        core = palette[0]
        rim = palette[min(2, len(palette) - 1)]
        cells = [(0, 0, core)]
        if size > 1:
            cells += [(dx, dy, palette[1]) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
        if size > 2:
            cells += [(dx, dy, rim) for dx, dy in ((1, 1), (-1, -1), (1, -1), (-1, 1), (2, 0), (-2, 0), (0, 2), (0, -2))]
        for dx, dy, c in cells:
            if 0 <= x + dx < w and 0 <= y + dy < h and rnd.random() < 0.95:
                out[y + dy][x + dx] = rig.blend(out[y + dy][x + dx], c)
    return out


def burst(frame, palette, centre, radius, n_rays=8, seed=0):
    """Short rays of light from centre (an impact or a spell going off), kept inside the frame."""
    rnd = random.Random(seed)
    out = rig.copy(frame)
    h, w = len(frame), len(frame[0])
    cx, cy = centre
    for k in range(n_rays):
        a = 2 * math.pi * k / n_rays + rnd.uniform(-0.25, 0.25)
        length = radius * rnd.uniform(0.6, 1.0)
        for step in range(int(length)):
            t = step / max(1, length)
            x, y = round(cx + math.cos(a) * (radius * 0.35 + step)), round(cy + math.sin(a) * (radius * 0.35 + step) * 0.8)
            if 0 <= x < w and 0 <= y < h:
                c = palette[0] if t < 0.35 else palette[1] if t < 0.7 else palette[2]
                out[y][x] = rig.blend(out[y][x], c)
    return out


def ring(frame, palette, centre, radius, thickness=1):
    """A ring of light (a shockwave), drawn over everything."""
    out = rig.copy(frame)
    h, w = len(frame), len(frame[0])
    cx, cy = centre
    for y in range(h):
        for x in range(w):
            d = math.hypot(x - cx, (y - cy) * 1.35)  # squashed: the ground is seen at an angle
            if radius - thickness < d <= radius:
                out[y][x] = rig.blend(out[y][x], palette[1])
            elif radius < d <= radius + 1:
                out[y][x] = rig.blend(out[y][x], palette[2])
    return out


def dissolve(frame, amount, palette, seed):
    """Removes a share of the figure's pixels, some turning into drifting motes."""
    rnd = random.Random(seed)
    out = rig.blank_like(frame)
    h, w = len(frame), len(frame[0])
    for y in range(h):
        for x in range(w):
            p = frame[y][x]
            if not p[3]:
                continue
            r = rnd.random()
            if r > amount:
                out[y][x] = p
            elif r > amount * 0.85:
                ny = y - round(amount * 6 * rnd.random())
                if 0 <= ny < h:
                    out[ny][x] = palette[rnd.randrange(len(palette))]
    return out


class Creature:
    """base: the creature's frame, with room around it for motion (a few pixels each side). Its
    baked shadow (shadow: the ellipse (cx, cy, rx, ry) it lies in, see split_shadow) stays on the
    ground while the figure moves; without one a soft shadow is made. magic: glow palette (animations.FIRE, FROST, LIGHTNING, DEATH, GOLD); accent: the attack
    flash palette (defaults to magic). flying: bobs instead of stepping. heavy: slower, deeper
    motion. waist: the row the upper body breathes from (default: 55% down the figure). pixel: 2
    for art doubled with Scale2x, so moves and glows keep their size."""

    def __init__(self, base, magic=FIRE, accent=None, flying=False, heavy=False, waist=None, seed=1,
                 shadow=None, shadow_colour=None, pixel=1):
        self.figure, self.shadow = split_shadow(base, shadow, shadow_colour)
        self.px = pixel  # art pixels per sprite pixel (2 for art doubled with Scale2x): motion scales with it
        self.magic = magic
        self.accent = accent or magic
        self.flying = flying
        self.heavy = heavy
        self.seed = seed
        x0, y0, x1, y1 = rig.bounds(self.figure)
        self.box = (x0, y0, x1, y1)
        self.width = x1 - x0 + 1
        self.feet = ((x0 + x1) // 2, y1)
        self.centre = ((x0 + x1) / 2, (y0 + y1) / 2)
        self.waist = waist if waist is not None else round(y0 + (y1 - y0) * 0.55)
        if not rig.bounds(self.shadow):
            half = max(3, self.width // 2 - 2)
            cx = self.feet[0]
            rig.drop_shadow(self.shadow, [(cx - half + 2, y1 + 1, 2 * half - 3), (cx - half, y1, 2 * half + 1)])

    def _frame(self, figure):
        out = rig.copy(self.shadow)
        return rig.paste(out, figure)

    def _lean(self, layer, degrees):
        return rig.rotate(layer, degrees, self.feet) if degrees else layer

    def idle(self):
        p = self.px
        if self.flying:
            return [self._frame(rig.shift(self.figure, 0, dy * p)) for dy in (0, -1, -2, -1)]
        return [self._frame(breathe(self.figure, self.waist, grow * p)) for grow in (0, 1, 1, 0)]

    def walk(self):
        tilt = 2 if self.heavy else 3
        return [self._frame(rig.shift(self._lean(self.figure, deg), 0, dy * self.px))
                for dy, deg in [(0, -tilt), (-1, 0), (0, tilt), (-1, 0)]]

    def attack(self):
        cx, _ = self.centre
        _, _, _, y1 = self.box
        p = self.px
        windup = rig.shift(self._lean(scale(self.figure, 1.0, 1.06, self.feet), -6), 0, -p)
        lunge = rig.shift(scale(self.figure, 1.08, 0.94, self.feet), 0, 2 * p)
        impact = burst(rig.flash(lunge, 0.25), self.accent, (cx, y1 - 2 * p), min(self.width * 0.45, 14 * p), seed=self.seed)
        settle = rig.shift(self.figure, 0, p)
        return [self._frame(f) for f in (windup, windup, lunge, impact, settle)]

    def cast(self):
        cx, cy = self.centre
        rnd = random.Random(self.seed + 7)
        frames = []
        p = self.px
        for i, strength in enumerate([0.45, 0.8, 1.0]):
            f = glow(rig.shift(self.figure, 0, (-1 - i // 2) * p), self.magic, width=(1 + (i > 0)) * p, strength=strength)
            # motes gathering in towards the creature
            pts = []
            for k in range(6):
                a = rnd.uniform(0, 2 * math.pi)
                r = self.width * (0.7 - 0.2 * i)
                pts.append((cx + math.cos(a) * r, cy + math.sin(a) * r * 0.8, min(3, (1 + (i == 2)) * p)))
            frames.append(self._frame(sparks(f, self.magic, pts, self.seed + i)))
        release = glow(rig.flash(rig.shift(self.figure, 0, -2 * p), 0.35), self.magic, width=2 * p)
        frames.append(self._frame(burst(release, self.magic, (cx, cy), min(self.width * 0.6, 20 * p), 10, self.seed)))
        return frames

    def hurt(self):
        p = self.px
        return [self._frame(rig.shift(rig.flash(self.figure, 0.7), p, 0)),
                self._frame(rig.shift(rig.tint(self.figure, (255, 64, 64), 0.25), -p, 0))]

    def death(self):
        slump = scale(self.figure, 1.06, 0.9, self.feet)
        frames = [self._frame(rig.flash(slump, 0.5))]
        for i, amount in enumerate([0.25, 0.5, 0.75, 0.95]):
            f = dissolve(slump, amount, self.magic, self.seed + i)
            frames.append(self._frame(f) if i < 2 else f)
        return frames

    def all(self):
        return {name: getattr(self, name)() for name in ("idle", "walk", "attack", "cast", "hurt", "death")}


# Seconds each frame is shown, per animation (a heavy creature moves slower)
def timing(frames, heavy=False):
    slow = 1.3 if heavy else 1.0
    base = {
        "idle": [0.5, 0.2, 0.5, 0.2],
        "walk": [0.16, 0.16, 0.16, 0.16],
        "attack": [0.2, 0.08, 0.08, 0.14, 0.18],
        "cast": [0.16, 0.14, 0.14, 0.24],
        "hurt": [0.08, 0.2],
        "death": [0.2, 0.14, 0.14, 0.14, 0.24],
    }
    return {name: [t * slow for t in base[name]][:len(frames[name])] for name in frames}
