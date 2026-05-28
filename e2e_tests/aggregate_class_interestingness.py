#!/usr/bin/env python3
"""Aggregate the class-interestingness experiment cells into a per-class report.

Reads every *_classes.csv (one per boss x profile) under the given dir, groups
the per-class aggregate rows across all cells, and emits a per-class summary of
the signals that proxy "interestingness": outcome spread, micro/dodge execution,
range discipline (melee vs kiting), ability cadence, and pacing. Tolerant of
missing columns so it keeps working as telemetry evolves.
"""
import csv, glob, os, sys, statistics

OUT_DIR = sys.argv[1] if len(sys.argv) > 1 else "saves/experiments/class_interest"

# (column, label, good-direction) -- good-direction is informational only.
METRICS = [
    ("win_rate", "win%", "ctx"),
    ("death_rate", "death%", "low"),
    ("avg_min_hp_pct", "minHP%", "ctx"),
    ("avg_package_micro_score", "micro", "high"),
    ("avg_package_micro_hits", "avoidHit", "low"),
    ("avg_package_micro_dodges", "dodges", "high"),
    ("avg_package_micro_best_streak", "bestStreak", "high"),
    ("avg_hazard_dodge_frames", "dodgeF", "ctx"),
    ("avg_ranged_keepaway_frames", "kiteF", "ctx"),
    ("avg_ranged_close_enemy_frames", "closeF", "ctx"),
    ("avg_ability_orders_total", "abilities", "high"),
    ("avg_spell_orders_total", "spells", "high"),
    ("avg_weapon_orders", "weapon", "ctx"),
    ("kills_per_1000_combat_frames", "kills/1k", "ctx"),
    ("damage_dealt_per_1000_combat_frames", "dmg/1k", "ctx"),
    ("combat_frame_rate", "combat%", "ctx"),
]
PCT = {"win_rate", "death_rate", "avg_min_hp_pct", "combat_frame_rate"}

def fnum(v):
    try:
        return float(v)
    except (TypeError, ValueError):
        return None

def main():
    cells = sorted(glob.glob(os.path.join(OUT_DIR, "*_classes.csv")))
    if not cells:
        print(f"No *_classes.csv cells under {OUT_DIR}")
        return 1
    # class -> metric -> list of per-cell values
    data = {}
    cell_count = 0
    for path in cells:
        try:
            rows = list(csv.DictReader(open(path)))
        except Exception:
            continue
        if rows:
            cell_count += 1
        for r in rows:
            cls = r.get("class") or "?"
            d = data.setdefault(cls, {})
            for col, _, _ in METRICS:
                v = fnum(r.get(col))
                if v is not None:
                    d.setdefault(col, []).append(v)

    classes = sorted(data.keys())
    present = [m for m in METRICS if any(m[0] in data[c] for c in classes)]

    # Header
    hdr = f"{'class':<13}" + "".join(f"{lab:>11}" for _, lab, _ in present)
    print(f"\n=== CLASS INTERESTINGNESS ({cell_count} cells, dir={OUT_DIR}) ===")
    print(hdr)
    print("-" * len(hdr))
    summary_rows = []
    for c in classes:
        line = f"{c:<13}"
        row = {"class": c, "cells": cell_count}
        for col, lab, _ in present:
            vals = data[c].get(col, [])
            if vals:
                m = statistics.mean(vals)
                row[col] = round(m, 3)
                line += (f"{m*100:>10.0f}%" if col in PCT else f"{m:>11.1f}")
            else:
                line += f"{'-':>11}"
        print(line)
        summary_rows.append(row)

    # Range-discipline fingerprint: close vs kite frame share
    print("\n=== range fingerprint (kite vs close frames; high close = melee-style) ===")
    for c in classes:
        kite = statistics.mean(data[c].get("avg_ranged_keepaway_frames", [0]) or [0])
        close = statistics.mean(data[c].get("avg_ranged_close_enemy_frames", [0]) or [0])
        print(f"  {c:<13} kiteF={kite:>6.0f}  closeF={close:>6.0f}")

    out_csv = os.path.join(OUT_DIR, "class_interestingness_summary.csv")
    cols = ["class", "cells"] + [m[0] for m in present]
    with open(out_csv, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=cols)
        w.writeheader()
        for row in summary_rows:
            w.writerow({k: row.get(k, "") for k in cols})
    print(f"\nWrote {out_csv}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
