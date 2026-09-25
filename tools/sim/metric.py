#!/usr/bin/env python3
"""Per-level difficulty = sum over monsters of mean_count * sim party damage / (P * player max HP), at estimated arrival level.
Usage: metric.py comp.json sim.txt [sim_override.txt ...]"""
import sys, os, json, math, statistics
from table import load
import comp as C
compj = json.load(open(sys.argv[1]))
d = {}
for f in sys.argv[2:]:
    d.update(load(f))  # later files override (re-simulated monsters)
def mdmg(cl, L, P, mon):
    def at(Li):
        v = d.get((cl, Li, P, mon))
        if not v: return None
        return max(1e-3, statistics.mean(a for a, *_ in v) / (P * v[0][1]))
    L = min(max(L, 1), 8)
    lo = int(math.floor(L)); hi = min(lo + 1, 8); t = L - lo
    a, b = at(lo), at(hi)
    if a is None: return 0.0
    if b is None or hi == lo: return a
    return math.exp(math.log(a) * (1 - t) + math.log(b) * t)
ROWS = ["Plain Valley", "Temple Entrance", "Temple Chamber", "Temple Sanctum", "Snake Pit Entrance", "Snake Pit Depths",
        "Hive Entrance", "Hive Depths", "Underdungeon", "Outpost Entrance", "Outpost Stockroom", "Ogre Lair", "Crypt",
        "Gragh's Lair", "Purple Dragon Lair", "Pixullochia Entrance", "Pixullochia Depths", "Hell"]
res = {}
for P in [p for p in ("1", "3", "4") if p in compj]:
    comp, lv = compj[P]["comp"], compj[P]["levels"]
    for cl in ("Fighter", "Stormcaller"):
        for lab in ROWS:
            L = lv.get(lab, 1)
            tot = 0; top = []
            for mon, cnt in comp.get(lab, {}).items():
                x = cnt * mdmg(cl, L, int(P), mon)
                tot += x; top.append((x, mon))
            top.sort(reverse=True)
            res[(P, cl, lab)] = (L, tot, top)
print(f"{'level':22s} {'opt':3s} | {'L1':>4s} {'F1':>6s} {'S1':>6s} | {'L3':>4s} {'F3':>6s} {'S3':>6s} | top contributors (F, 1p)")
prev = {}
for lab in ROWS:
    L1, f1, top = res[("1", "Fighter", lab)]
    _, s1, _ = res[("1", "Stormcaller", lab)]
    L3, f3, _ = res[("3", "Fighter", lab)]
    _, s3, _ = res[("3", "Stormcaller", lab)]
    opt = "opt" if lab in C.OPTIONAL else ""
    print(f"{lab:22s} {opt:3s} | {L1:4.1f} {f1:6.2f} {s1:6.2f} | {L3:4.1f} {f3:6.2f} {s3:6.2f} | " + ", ".join(f"{m} {x:.1f}" for x, m in (top if os.environ.get("FULL") else top[:3])))
