#!/usr/bin/env python3
"""Measure per-class SKILL DEPTH from a multi-profile experiment run.

Skill depth = how much outcome/expression changes between sloppy and perfect
play. Reads cells named {boss}__{profile}_classes.csv, averages each skill
signal per (class, profile) across bosses, then reports the sloppy->perfect
progression and the depth (perfect - sloppy spread). Large spread = skill
matters a lot (deep); flat = skill barely matters (shallow, e.g. spam-wins).
"""
import csv, glob, os, sys, statistics

OUT_DIR = sys.argv[1] if len(sys.argv) > 1 else "saves/experiments/skill_depth"
PROFILE_ORDER = ["sloppy", "human", "skilled", "perfect"]

# Signals where higher = better play. Depth = perfect - sloppy.
SIGNALS = [
    ("win_rate", "win%", True),
    ("avg_package_micro_score", "micro", False),
    ("avg_package_flow_active_rate", "flow%", True),
    ("avg_package_graze_rate", "graze%", True),
    ("avg_package_micro_hits", "avoidHit(low=good)", False),
]

def fnum(v):
    try:
        return float(v)
    except (TypeError, ValueError):
        return None

def profile_of(path):
    base = os.path.basename(path)
    # {boss}__{profile}_classes.csv
    if "__" not in base:
        return None
    tail = base.split("__", 1)[1]
    return tail.rsplit("_classes.csv", 1)[0]

def main():
    cells = sorted(glob.glob(os.path.join(OUT_DIR, "*_classes.csv")))
    if not cells:
        print(f"No cells under {OUT_DIR}")
        return 1
    # class -> profile -> signal -> [values]
    data = {}
    profiles_seen = set()
    for path in cells:
        prof = profile_of(path)
        if not prof:
            continue
        profiles_seen.add(prof)
        for r in csv.DictReader(open(path)):
            cls = r.get("class") or "?"
            pd = data.setdefault(cls, {}).setdefault(prof, {})
            for col, _, _ in SIGNALS:
                v = fnum(r.get(col))
                if v is not None:
                    pd.setdefault(col, []).append(v)

    profiles = [p for p in PROFILE_ORDER if p in profiles_seen] or sorted(profiles_seen)
    classes = sorted(data.keys())
    pct_cols = {"win_rate", "avg_package_flow_active_rate", "avg_package_graze_rate"}

    print(f"\n=== SKILL DEPTH ({len(cells)} cells, profiles={profiles}, dir={OUT_DIR}) ===")
    print("Per signal: sloppy -> ... -> perfect, then DEPTH (perfect minus sloppy; sign = good direction).\n")

    def avg(cls, prof, col):
        vals = data.get(cls, {}).get(prof, {}).get(col, [])
        return statistics.mean(vals) if vals else None

    depth_scores = {}
    for col, label, higher_better in SIGNALS:
        print(f"-- {label} --")
        for cls in classes:
            cells_str = " -> ".join(
                ("{:.0f}%".format(avg(cls, p, col) * 100) if avg(cls, p, col) is not None and col in pct_cols
                 else ("{:.1f}".format(avg(cls, p, col)) if avg(cls, p, col) is not None else "-"))
                for p in profiles)
            lo = avg(cls, profiles[0], col)
            hi = avg(cls, profiles[-1], col)
            depth = (hi - lo) if (lo is not None and hi is not None) else None
            if depth is not None and not higher_better:
                depth = -depth  # flip so positive depth always = "perfect plays better"
            dstr = ("{:+.3f}".format(depth) if depth is not None else "-")
            if depth is not None:
                depth_scores.setdefault(cls, {})[col] = depth
            print(f"    {cls:<12} {cells_str:<34}  DEPTH {dstr}")
        print()

    # Composite: normalize each signal's depth across classes, average -> a single depth score.
    print("=== composite skill-depth ranking (higher = skill matters more) ===")
    norm = {}
    for col, _, _ in SIGNALS:
        col_vals = {c: depth_scores.get(c, {}).get(col) for c in classes if depth_scores.get(c, {}).get(col) is not None}
        if not col_vals:
            continue
        mx = max(abs(v) for v in col_vals.values()) or 1.0
        for c, v in col_vals.items():
            norm.setdefault(c, []).append(v / mx)
    ranking = sorted(((statistics.mean(v), c) for c, v in norm.items()), reverse=True)
    for score, c in ranking:
        print(f"    {c:<12} {score:+.2f}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
