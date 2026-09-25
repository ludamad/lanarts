#!/usr/bin/env python3
"""Parse StartMapGenCheck logs -> mean per-level enemy composition; estimate arrival character level from XP."""
import re, sys, glob, json, math, collections
import os
RT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "runtime")
GATE_MAP = {
    "Underdungeon|GATE:dragon_gate": "Purple Dragon Lair",
    "Underdungeon|GATE:gragh_gate": "Gragh's Lair",
    "Underdungeon|GATE:hell_dungeon": "Pixullochia Entrance",
    "Underdungeon|GATE:crypt_dungeon": "Crypt",
    "Crypt|GATE:hell_dungeon": "Hell",
}
FIXED = {"Purple Dragon Lair": {"Purple Dragon": 1, "Fire Bat": 8}}
ORDER_T = ["Plain Valley", "Temple Entrance", "Temple Chamber", "Temple Sanctum"]
ORDER_S = ["Plain Valley", "Snake Pit Entrance", "Snake Pit Depths"]
ORDER_REST = ["Hive Entrance", "Hive Depths", "Underdungeon", "Outpost Entrance", "Outpost Stockroom", "Ogre Lair",
              "Crypt", "Gragh's Lair", "Purple Dragon Lair", "Pixullochia Entrance", "Pixullochia Depths"]
OPTIONAL = {"Outpost Entrance", "Outpost Stockroom", "Ogre Lair", "Purple Dragon Lair", "Hell"}

def enemy_xp():
    src = open(RT + "/enemies/Enemies.moon").read()
    xp = {}
    for block in re.split(r"DataW\.enemy_create", src)[1:]:
        m = re.search(r'name:\s*"([^"]+)"', block); x = re.search(r"\n\s*xpaward:\s*(\d+)", block)
        if m and x: xp[m.group(1)] = int(x.group(1))
    return xp

def parse(logdir, players):
    per_level = collections.defaultdict(lambda: collections.Counter())
    nseeds = collections.Counter()
    routes = collections.Counter()
    for f in sorted(glob.glob(f"{logdir}/p{players}_s*.log")):
        seen = {}
        for line in open(f):
            if not line.startswith("RESULT"): continue
            parts = line.split("|")
            # label may contain '|' for gates
            m = re.match(r"RESULT seed=(\d+) players=(\d+) \| (.*?)\s+enemies=\s*(\d+).*?\| (.*)$", line.rstrip("\n"))
            label, names = m.group(3).strip(), m.group(5)
            c = collections.Counter()
            for kv in names.split(","):
                if "=" in kv:
                    k, v = kv.rsplit("=", 1)
                    if "Lanart" not in k: c[k] += int(v)
            seen[label] = c
        route = "snake" if "Snake Pit Entrance" in seen else "temple"
        routes[route] += 1
        merged = collections.defaultdict(collections.Counter)
        for label, c in seen.items():
            merged[GATE_MAP.get(label, label)].update(c)
        for k, v in FIXED.items(): merged[k].update(v)
        for label, c in merged.items():
            per_level[label].update(c); nseeds[label] += 1
    comp = {lab: {k: v / nseeds[lab] for k, v in c.items()} for lab, c in per_level.items()}
    return comp, routes

def xpneeded(L): return round(L ** 2.5) * 75 + 125

def arrival_levels(comp, players, order):
    xp = enemy_xp(); P = players
    bonus = P / (1 + P / 2.0)
    kills = collections.Counter(); lvl, cur = 1, 0.0; out = {}
    for lab in order:
        out[lab] = lvl + cur / xpneeded(lvl)
        for name, cnt in comp.get(lab, {}).items():
            for i in range(int(round(cnt))):
                kills[name] += 1
                n = (kills[name] - 1) // P
                w = xp.get(name, 0) * max(0.05, 0.9 ** n)
                cur += math.ceil(w * bonus * bonus / P)
                while cur >= xpneeded(lvl):
                    cur -= xpneeded(lvl); lvl += 1
    return out

if __name__ == "__main__":
    logdir = sys.argv[1]
    res = {}
    for P in (1, 3, 4):
        comp, routes = parse(logdir, P)
        lv = {}
        lv.update(arrival_levels(comp, P, ORDER_S + ORDER_REST))
        lv.update(arrival_levels(comp, P, ORDER_T + ORDER_REST))  # temple route levels override shared ones
        # Hell is behind a Magentite door in the Crypt; the Magentite Key is in the Purple Dragon Lair
        lv["Hell"] = arrival_levels(comp, P, ORDER_T + ORDER_REST[:9] + ["Hell"])["Hell"]
        res[P] = {"comp": comp, "levels": lv, "routes": dict(routes)}
        print(f"P={P} routes={dict(routes)}")
        for lab in ORDER_T + ORDER_S[1:] + ORDER_REST + ["Hell"]:
            c = comp.get(lab, {})
            print(f"  {lab:24s} L={lv.get(lab, 0):5.2f} n={sum(c.values()):5.1f}  " + ", ".join(f"{k}={v:.1f}" for k, v in sorted(c.items())))
    json.dump(res, open(logdir + "/comp.json", "w"), indent=1)
