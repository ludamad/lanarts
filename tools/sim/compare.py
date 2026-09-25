#!/usr/bin/env python3
"""Compare per-level difficulty of two balance runs (see metric.py) and print step ratios along the main routes.
Usage: compare.py before/comp.json before_sim.txt after/comp.json after_sim.txt"""
import json, math, statistics, sys
from table import load
import comp as C  # noqa: F401
def build(compf, simf):
    compj = json.load(open(compf)); d = load(simf)
    def mdmg(cl, L, P, mon):
        def at(Li):
            v = d.get((cl, Li, P, mon))
            if not v: return None
            return max(1e-3, statistics.mean(a for a, *_ in v) / (P * v[0][1]))
        L = min(max(L, 1), 8); lo = int(math.floor(L)); hi = min(lo + 1, 8); t = L - lo
        a, b = at(lo), at(hi)
        if a is None: return float('nan')
        if b is None or hi == lo: return a
        return math.exp(math.log(a) * (1 - t) + math.log(b) * t)
    out = {}
    for P, cl in [c for c in (("1", "Fighter"), ("3", "Fighter"), ("4", "Fighter"), ("1", "Rogue")) if c[0] in compj]:
        comp, lv = compj[P]["comp"], compj[P]["levels"]
        for lab in comp:
            out[(P, cl, lab)] = sum(cnt * mdmg(cl, lv.get(lab, 1), int(P), m) for m, cnt in comp[lab].items())
    return out
if len(sys.argv) != 5:
    sys.exit(__doc__)
B = build(sys.argv[1], sys.argv[2]); A = build(sys.argv[3], sys.argv[4])
ROWS = ["Plain Valley", "Temple Entrance", "Temple Chamber", "Temple Sanctum", "Snake Pit Entrance", "Snake Pit Depths",
        "Hive Entrance", "Hive Depths", "Underdungeon", "Outpost Entrance", "Outpost Stockroom", "Ogre Lair", "Crypt",
        "Gragh's Lair", "Purple Dragon Lair", "Pixullochia Entrance", "Pixullochia Depths", "Hell"]
cols = (("1", "Fighter"), ("3", "Fighter"), ("4", "Fighter"), ("1", "Rogue"))
print(f"{'level':22s}" + "".join(f" | {cl[0]}{P} before  after" for P, cl in cols))
for lab in ROWS:
    print(f"{lab:22s}" + "".join(f" | {B.get((P,cl,lab),float('nan')):9.2f} {A.get((P,cl,lab),float('nan')):6.2f}" for P, cl in cols))
print("\nStep ratios (after) along temple route / snake route")
T = ["Plain Valley", "Temple Entrance", "Temple Chamber", "Temple Sanctum", "Hive Entrance", "Hive Depths", "Underdungeon", "Crypt", "Gragh's Lair", "Pixullochia Entrance", "Pixullochia Depths"]
Sn = ["Plain Valley", "Snake Pit Entrance", "Snake Pit Depths", "Hive Entrance"]
for P, cl in cols:
    for name, X in (("before", B), ("after", A)):
        r = [f"{X.get((P,cl,T[i]),math.nan)/X.get((P,cl,T[i-1]),math.nan):.2f}" for i in range(1, len(T))]
        s = [f"{X.get((P,cl,Sn[i]),math.nan)/X.get((P,cl,Sn[i-1]),math.nan):.2f}" for i in range(1, len(Sn))]
        h = X.get((P,cl,"Hell"),math.nan)/X.get((P,cl,"Crypt"),math.nan)
        print(f"{cl[0]}{P} {name:6s} temple: {' '.join(r)} | snake: {' '.join(s)} | Hell/Crypt {h:.2f}")
