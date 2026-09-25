"""Animations shared by every rig: idle, walk, attack, cast, hurt and death.

A rig module describes one sprite (see rigs/*.py):
  SOURCE        path of the 32x32 sprite, relative to runtime/
  WEAPON        region holding the weapon (and the fist gripping it)
  GRIP          (x, y) the weapon rotates around
  ATTACK        [(degrees, body_dx, body_dy[, hand_dx, hand_dy])] for wind-up, swing and strike:
                the weapon's angle (clockwise, around GRIP) and where the hand has moved to. Moving
                the hand turns a wrist flick into an arm swing.
  LEG_LEFT, LEG_RIGHT   regions of each leg, which lift in turn when walking
  UPPER_TO_ROW  last row of the upper body; everything above moves together
  WEAPON_KIND   "blade" (swung; the default), "caster" (WEAPON is the free hand, which thrusts and
                raises to cast) or "bow" (drawn and loosed)
  MAGIC         four colours for cast glows, brightest first (defaults to frost blue)
  optional: KEEP_UPPER (regions below UPPER_TO_ROW that move with the upper body, e.g. a shield),
            SHADOW (drop-shadow spans for sprites without their own), LYING_DY (death pose offset)
"""
import math
import rig

SMEAR_BRIGHT = (236, 246, 255, 220)
SMEAR_FAINT = (190, 214, 240, 120)
ARROW_HEAD = (220, 224, 230, 255)
ARROW_SHAFT = (138, 96, 52, 255)
ARROW_FLETCH = (230, 230, 220, 255)
MAGIC = [(255, 255, 255, 255), (174, 233, 255, 240), (74, 168, 238, 210), (31, 91, 176, 150)]

# Glow colours by school of magic, brightest first
FIRE = [(255, 246, 190, 255), (255, 196, 64, 240), (255, 106, 26, 220), (184, 40, 16, 160)]
FROST = MAGIC
LIGHTNING = [(255, 255, 255, 255), (255, 251, 176, 240), (232, 224, 112, 220), (138, 138, 216, 160)]
DEATH = [(232, 255, 224, 255), (156, 240, 122, 240), (63, 176, 74, 220), (74, 30, 110, 170)]
GOLD = [(255, 250, 220, 255), (255, 224, 140, 240), (214, 170, 70, 200), (130, 90, 40, 150)]


def _regions(*rs):
    merged = {}
    for r in rs:
        for y, spans in r.items():
            spans = spans if isinstance(spans, list) else [spans]
            merged.setdefault(y, []).extend(spans)
    return merged


def _cut(frame, region):
    layer = rig.blank()
    for y, spans in region.items():
        for x0, x1 in (spans if isinstance(spans, list) else [spans]):
            for x in range(x0, x1 + 1):
                layer[y][x] = frame[y][x]
                frame[y][x] = rig.CLEAR
    return layer


class Layers:
    def __init__(self, r, root):
        g = rig.load(f"{root}/{r.SOURCE}")
        self.weapon = _cut(g, r.WEAPON)
        self.leg_left = _cut(g, r.LEG_LEFT)
        self.leg_right = _cut(g, r.LEG_RIGHT)
        self.keep_upper = _cut(g, _regions(*getattr(r, "KEEP_UPPER", [])))
        self.upper = _cut(g, rig.rect(0, 0, 31, r.UPPER_TO_ROW))
        self.lower = g


class Animator:
    def __init__(self, r, root):
        self.r = r
        self.L = Layers(r, root)
        self.grip = r.GRIP
        self.tip = self._farthest(self.L.weapon, self.grip)
        self.kind = getattr(r, "WEAPON_KIND", "blade")
        self.magic = getattr(r, "MAGIC", MAGIC)

    @staticmethod
    def _farthest(layer, pivot):
        pts = [(x, y) for y in range(rig.SIZE) for x in range(rig.SIZE) if layer[y][x][3]]
        return max(pts, key=lambda p: (p[0] - pivot[0]) ** 2 + (p[1] - pivot[1]) ** 2)

    def _rotated_tip(self, degrees, dx=0, dy=0):
        """Where the weapon tip is at this angle, with the grip moved by (dx, dy)."""
        a = math.radians(degrees)
        tx, ty = self.tip[0] - self.grip[0], self.tip[1] - self.grip[1]
        return (self.grip[0] + dx + math.cos(a) * tx - math.sin(a) * ty,
                self.grip[1] + dy + math.sin(a) * tx + math.cos(a) * ty)

    # ---- composition ------------------------------------------------------------------------

    def frame(self, dx=0, dy=0, left=(0, 0), right=(0, 0), weapon=None, lower_dx=0):
        """weapon: None for the resting weapon, a finished layer, or False for none."""
        L = self.L
        f = rig.blank()
        rig.drop_shadow(f, getattr(self.r, "SHADOW", []))
        rig.paste(f, L.lower, lower_dx)
        rig.paste(f, L.leg_left, *left)
        rig.paste(f, L.leg_right, *right)
        rig.paste(f, L.upper, dx, dy)
        rig.paste(f, L.keep_upper, dx, dy)
        if weapon is None:
            rig.paste(f, L.weapon, dx, dy)
        elif weapon is not False:
            rig.paste(f, weapon, dx, dy)
        return f

    def weapon_at(self, degrees, raise_by=0):
        w = rig.rotate(self.L.weapon, degrees, self.grip) if degrees else self.L.weapon
        return rig.shift(w, 0, -raise_by) if raise_by else w

    # ---- animations -------------------------------------------------------------------------

    def idle(self):
        return [self.frame(), self.frame(), self.frame(dy=1), self.frame(dy=1)]

    def walk(self):
        if not self.r.LEG_LEFT and not self.r.LEG_RIGHT:     # long robes: glide and bob
            return [self.frame(), self.frame(dy=-1), self.frame(), self.frame(dy=-1)]
        return [
            self.frame(dy=1),
            self.frame(left=(0, -2)),
            self.frame(dy=1),
            self.frame(right=(0, -2)),
        ]

    @staticmethod
    def _pose(p):
        degrees, bdx, bdy = p[:3]
        hdx, hdy = p[3:5] if len(p) >= 5 else (0, 0)
        return degrees, bdx, bdy, hdx, hdy

    def weapon_posed(self, degrees, hdx, hdy):
        return rig.shift(self.weapon_at(degrees), hdx, hdy)

    def _trail(self, f, p0, p1, faint=False):
        """Swing trail along the path the weapon tip took between two poses, behind the figure.
        The grip moves with the hand, so the tip traces the arm's arc rather than a wrist flick."""
        a0, bdx0, bdy0, hdx0, hdy0 = p0
        a1, bdx1, bdy1, hdx1, hdy1 = p1
        steps = 24
        for i in range(steps + 1):
            t = i / steps
            a = a0 + (a1 - a0) * t
            gx = bdx0 + hdx0 + (bdx1 + hdx1 - bdx0 - hdx0) * t
            gy = bdy0 + hdy0 + (bdy1 + hdy1 - bdy0 - hdy0) * t
            x, y = self._rotated_tip(a, gx, gy)
            # direction from the grip to the tip, to thicken the trail inwards along the blade
            ux, uy = x - (self.grip[0] + gx), y - (self.grip[1] + gy)
            n = math.hypot(ux, uy) or 1
            for k in range(1 + (2 * i) // steps):     # thicker towards the end of the swing
                px, py = int(round(x - ux / n * k)), int(round(y - uy / n * k))
                if 0 <= px < rig.SIZE and 0 <= py < rig.SIZE and f[py][px][3] == 0:
                    f[py][px] = SMEAR_FAINT if (faint or k == 0) else SMEAR_BRIGHT

    def attack(self):
        if self.kind == "caster":
            return self._attack_caster()
        if self.kind == "bow":
            return self._attack_bow()
        wind, swing, strike = (self._pose(p) for p in self.r.ATTACK)

        def posed(p):
            a, bdx, bdy, hdx, hdy = p
            return self.frame(bdx, bdy, weapon=self.weapon_posed(a, hdx, hdy))

        f_wind = posed(wind)
        f_swing = posed(swing)
        self._trail(f_swing, wind, swing)
        f_strike = posed(strike)
        self._trail(f_strike, swing, strike)
        f_follow = posed(strike)
        self._trail(f_follow, swing, strike, faint=True)
        return [f_wind, f_swing, f_strike, f_follow, self.frame()]

    def _attack_caster(self):
        """Lean back, then thrust the open hand forward with a spark off the palm."""
        side = -1 if getattr(self.r, "WEAPON_SIDE", "left") == "left" else 1
        wind = self.frame(dx=-side, weapon=rig.shift(self.L.weapon, 0, -1))
        thrust_hand = rig.shift(self.L.weapon, 2 * side, 0)
        b = rig.bounds(thrust_hand)
        sx = (b[0] - 1) if side < 0 else (b[2] + 1)
        sy = self.grip[1]
        release = self.frame(dx=side, weapon=thrust_hand)
        self._glow(release, sx + side, sy, 2)
        follow = self.frame(dx=side, weapon=thrust_hand)
        self._glow(follow, sx + 2 * side, sy, 1)
        return [wind, release, follow, self.frame()]

    def _attack_bow(self):
        """Draw with an arrow nocked across the bow, then loose it: a bright streak off the bow."""
        side = -1 if getattr(self.r, "WEAPON_SIDE", "left") == "left" else 1
        bow = rig.shift(self.L.weapon, side, 0)
        b = rig.bounds(bow)
        y = (b[1] + b[3]) // 2
        mid = (b[0] + b[2]) // 2
        draw = self.frame(dx=-side, weapon=bow)
        # the arrow lies across the bow, pointing away from the body, fletching towards the hand
        head = mid + 2 * side
        for i in range(7):
            x = head - i * side
            if 0 <= x < rig.SIZE:
                draw[y][x] = ARROW_HEAD if i == 0 else (ARROW_FLETCH if i >= 5 else ARROW_SHAFT)
        release = self.frame(weapon=bow)
        self._streak(release, head + 3 * (-side), y, side, faint=False)
        follow = self.frame(weapon=bow)
        self._streak(follow, head, y, side, faint=True)
        return [draw, release, follow, self.frame()]

    @staticmethod
    def _streak(f, x0, y, direction, faint):
        """An arrow in flight: a bright line from x0 to the edge of the frame, drawn over everything,
        with a flare where it leaves the bow."""
        x, i = x0, 0
        while 0 <= x < rig.SIZE:
            f[y][x] = rig.blend(f[y][x], SMEAR_FAINT if faint else SMEAR_BRIGHT)
            if i < 2 and not faint:
                for dy in (-1, 1):
                    if 0 <= y + dy < rig.SIZE:
                        f[y + dy][x] = rig.blend(f[y + dy][x], SMEAR_FAINT)
            x += direction
            i += 1

    def cast(self):
        if self.kind == "caster":
            hand = rig.shift(self.L.weapon, 0, -getattr(self.r, "CAST_RAISE", 3))
            b = rig.bounds(hand)
            tx, ty = (b[0] + b[2]) // 2, max(b[1] - 3, 2)
            frames = [self.frame(weapon=hand)]
            for stage in range(1, 4):
                f = self.frame(weapon=hand, dy=-1 if stage == 3 else 0)
                self._glow(f, tx, ty - (1 if stage == 3 else 0), stage)
                frames.append(f)
            return frames
        raise_by = getattr(self.r, "CAST_RAISE", 5)
        raised = self.weapon_at(getattr(self.r, "CAST_ANGLE", 0), raise_by)
        top = min((y, x) for y in range(rig.SIZE) for x in range(rig.SIZE) if raised[y][x][3])
        tx, ty = top[1], max(top[0], 2)
        frames = [self.frame(weapon=raised)]
        for stage in range(1, 4):
            w = rig.tint(raised, self.magic[1], 0.25 * stage)
            f = self.frame(weapon=w)
            self._glow(f, tx, ty, stage)
            frames.append(f)
        return frames

    def _glow(self, f, cx, cy, stage):
        core, bright, mid, edge = self.magic
        rings = {1: [(0, bright)], 2: [(0, core), (1, bright), (2, mid)],
                 3: [(0, core), (1, core), (2, bright), (3, mid), (4, edge)]}[stage]
        for r, colour in reversed(rings):
            for y in range(cy - r, cy + r + 1):
                for x in range(cx - r, cx + r + 1):
                    if abs(x - cx) + abs(y - cy) == r and 0 <= x < rig.SIZE and 0 <= y < rig.SIZE:
                        f[y][x] = rig.blend(f[y][x], colour)
        if stage == 3:                                  # rays
            for d in range(4, 7):
                for x, y in [(cx - d, cy), (cx + d, cy), (cx, cy - d), (cx, cy + d)]:
                    if 0 <= x < rig.SIZE and 0 <= y < rig.SIZE:
                        f[y][x] = rig.blend(f[y][x], edge)

    def hurt(self):
        away = -1 if getattr(self.r, "WEAPON_SIDE", "left") == "left" else 1
        return [rig.flash(self.frame(dx=-away)), self.frame(dx=-away)]

    def death(self):
        stagger = self.frame(dx=1, dy=1)
        caster = self.kind == "caster"          # a caster's "weapon" is their hand, which stays on
        droop = None if caster else self.weapon_at(getattr(self.r, "DROOP", 20))
        kneel = self.frame(dy=4, left=(0, 2), right=(0, 2), weapon=droop)
        body = self.frame(weapon=None if caster else False)
        lying = rig.rotate_frame_ccw(body)
        f = rig.blank()
        rig.drop_shadow(f, [(3, 27, 26), (2, 28, 28), (4, 29, 24)])
        rig.paste(f, lying, 0, getattr(self.r, "LYING_DY", 8))
        dropped = rig.rotate(self.L.weapon, 90 if getattr(self.r, "WEAPON_SIDE", "left") == "left" else -90, self.grip)
        b = rig.bounds(dropped)
        if b and not caster:
            rig.paste(f, dropped, 4 - b[0], 30 - b[3])
        return [stagger, kneel, f]

    def timing(self):
        """Seconds per frame of each animation, for this rig's weapon style."""
        t = dict(TIMING)
        t.update(TIMING_BY_KIND.get(self.kind, {}))
        return t

    def all(self):
        return {"idle": self.idle(), "walk": self.walk(), "attack": self.attack(),
                "cast": self.cast(), "hurt": self.hurt(), "death": self.death()}


TIMING = {
    "idle": [0.5, 0.2, 0.5, 0.2],
    "walk": [0.15] * 4,
    "attack": [0.2, 0.07, 0.12, 0.1, 0.2],
    "cast": [0.16, 0.12, 0.12, 0.24],
    "hurt": [0.08, 0.22],
    "death": [0.2, 0.3, 1.2],
}

# Attacks that aren't swings have their own frame counts and rhythm
TIMING_BY_KIND = {
    "caster": {"attack": [0.14, 0.08, 0.14, 0.2]},
    "bow": {"attack": [0.2, 0.06, 0.12, 0.2]},
}
