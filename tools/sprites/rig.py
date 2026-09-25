"""Pixel machinery for animating lanarts' 32x32 sprites.

Frames are 32x32 grids of RGBA tuples. A sprite is cut into layers (weapon, upper body, legs, ...)
with hand-traced regions, and animations move, rotate and recombine those layers. New pixels are
placed by hand with put().
"""
import math
from PIL import Image

SIZE = 32
CLEAR = (0, 0, 0, 0)
SHADOW = (0, 0, 0, 100)


def blank():
    return [[CLEAR] * SIZE for _ in range(SIZE)]


def load(path):
    im = Image.open(path).convert("RGBA")
    assert im.size == (SIZE, SIZE), (path, im.size)
    return [[im.getpixel((x, y)) for x in range(SIZE)] for y in range(SIZE)]


def to_image(frame):
    im = Image.new("RGBA", (SIZE, SIZE))
    for y in range(SIZE):
        for x in range(SIZE):
            im.putpixel((x, y), frame[y][x])
    return im


def copy(frame):
    return [row[:] for row in frame]


def blend(under, over):
    if over[3] == 255 or under[3] == 0:
        return over
    a = over[3] / 255
    return tuple(round(over[i] * a + under[i] * (1 - a)) for i in range(3)) + (255,)


def paste(dst, layer, dx=0, dy=0):
    for y in range(SIZE):
        for x in range(SIZE):
            p = layer[y][x]
            if p[3] and 0 <= x + dx < SIZE and 0 <= y + dy < SIZE:
                dst[y + dy][x + dx] = blend(dst[y + dy][x + dx], p)
    return dst


def paste_behind(dst, layer, dx=0, dy=0):
    """Only fills pixels that are still empty (for trails and glows that pass behind the figure)."""
    for y in range(SIZE):
        for x in range(SIZE):
            p = layer[y][x]
            if p[3] and 0 <= x + dx < SIZE and 0 <= y + dy < SIZE and dst[y + dy][x + dx][3] == 0:
                dst[y + dy][x + dx] = p
    return dst


def put(dst, x, y, run, colours, behind=False):
    """Hand-placed pixels: one palette key per character of run, spaces skipped."""
    for i, ch in enumerate(run):
        if ch == " " or not (0 <= x + i < SIZE and 0 <= y < SIZE):
            continue
        if behind and dst[y][x + i][3]:
            continue
        dst[y][x + i] = blend(dst[y][x + i], colours[ch])


def cut(frame, region):
    """Moves the pixels of region ({row: (first col, last col)}) out of frame into a new layer."""
    layer = blank()
    for y, (x0, x1) in region.items():
        for x in range(x0, x1 + 1):
            layer[y][x] = frame[y][x]
            frame[y][x] = CLEAR
    return layer


def rect(x0, y0, x1, y1):
    return {y: (x0, x1) for y in range(y0, y1 + 1)}


def bounds(layer):
    pts = [(x, y) for y in range(SIZE) for x in range(SIZE) if layer[y][x][3]]
    if not pts:
        return None
    xs, ys = zip(*pts)
    return min(xs), min(ys), max(xs), max(ys)


# ---------------------------------------------------------------------------------------------
# RotSprite-style rotation: upscale 8x with Scale2x (which keeps edges crisp), rotate with
# nearest-neighbour sampling, then take each 8x8 cell's most common colour. Rotated pixel art
# keeps clean lines instead of the ragged edges of a direct nearest-neighbour rotation.
# ---------------------------------------------------------------------------------------------

def _scale2x(grid):
    h, w = len(grid), len(grid[0])
    out = [[CLEAR] * (w * 2) for _ in range(h * 2)]
    for y in range(h):
        for x in range(w):
            p = grid[y][x]
            a = grid[y - 1][x] if y > 0 else p
            b = grid[y][x + 1] if x < w - 1 else p
            c = grid[y][x - 1] if x > 0 else p
            d = grid[y + 1][x] if y < h - 1 else p
            e0 = a if (c == a and c != d and a != b) else p
            e1 = b if (a == b and a != c and b != d) else p
            e2 = c if (d == c and d != b and c != a) else p
            e3 = d if (b == d and b != a and d != c) else p
            out[2 * y][2 * x], out[2 * y][2 * x + 1] = e0, e1
            out[2 * y + 1][2 * x], out[2 * y + 1][2 * x + 1] = e2, e3
    return out


def rotate(layer, degrees, pivot):
    """Rotates a layer clockwise by degrees around pivot (x, y), in sprite pixels."""
    big = _scale2x(_scale2x(_scale2x(layer)))
    n = SIZE * 8
    rad = math.radians(degrees)
    cos, sin = math.cos(rad), math.sin(rad)
    px, py = pivot[0] * 8 + 4, pivot[1] * 8 + 4
    out = blank()
    for y in range(SIZE):
        for x in range(SIZE):
            counts = {}
            solid = 0
            for sy in range(1, 8, 2):
                for sx in range(1, 8, 2):
                    tx, ty = x * 8 + sx - px, y * 8 + sy - py
                    # inverse rotation finds where this sample comes from
                    ix = int(round(cos * tx + sin * ty + px))
                    iy = int(round(-sin * tx + cos * ty + py))
                    p = big[iy][ix] if 0 <= ix < n and 0 <= iy < n else CLEAR
                    if p[3]:
                        solid += 1
                        counts[p] = counts.get(p, 0) + 1
            # solid if at least half of the 16 samples are, in the most common solid colour
            out[y][x] = max(counts, key=counts.get) if solid >= 8 else CLEAR
    return out


def flip_x(layer):
    return [row[::-1] for row in layer]


def shift(layer, dx, dy):
    return paste(blank(), layer, dx, dy)


# ---------------------------------------------------------------------------------------------
# Effects
# ---------------------------------------------------------------------------------------------

def drop_shadow(dst, spans):
    """spans: (x0, y, width) rows of soft shadow, drawn under everything."""
    for x0, y, n in spans:
        for x in range(x0, x0 + n):
            if 0 <= x < SIZE and 0 <= y < SIZE and dst[y][x][3] == 0:
                dst[y][x] = SHADOW


def flash(frame, amount=0.6):
    """Hit flash: every pixel pushed towards white, shadows left alone."""
    return [[p if p[3] == 0 or p == SHADOW else
             tuple(round(p[i] + (255 - p[i]) * amount) for i in range(3)) + (p[3],) for p in row] for row in frame]


def tint(layer, colour, amount):
    return [[p if p[3] == 0 else tuple(round(p[i] * (1 - amount) + colour[i] * amount) for i in range(3)) + (p[3],)
             for p in row] for row in layer]


def rotate_frame_ccw(frame):
    return [[frame[x][SIZE - 1 - y] for x in range(SIZE)] for y in range(SIZE)]


def rotate_frame_cw(frame):
    return [[frame[SIZE - 1 - x][y] for x in range(SIZE)] for y in range(SIZE)]


def save_strip(frames, path):
    strip = Image.new("RGBA", (SIZE * len(frames), SIZE))
    for i, f in enumerate(frames):
        strip.alpha_composite(to_image(f), (SIZE * i, 0))
    strip.save(path)
