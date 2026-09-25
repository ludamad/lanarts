#!/usr/bin/env python3
"""Count map generation retries per level in StartMapGenCheck logs. Usage: retries.py logdir..."""
import sys, glob, collections
for d in sys.argv[1:]:
    fails, reasons, worlds = collections.Counter(), collections.Counter(), 0
    for f in glob.glob(f"{d}/*.log"):
        worlds += 1; level = "?"
        for line in open(f, errors="replace"):
            line = line.strip()
            if line.startswith("LEVEL "): level = line[6:]
            elif "FAILED, RETRYING" in line: fails[level] += 1
            elif line.endswith(" fail") or line.endswith("failed") or line.startswith("GEN ERROR"): reasons[(level, line)] += 1
    print(f"{d}: {sum(fails.values())} retries over {worlds} worlds")
    for lvl, n in fails.most_common():
        why = ", ".join(f"{r} x{c}" for (l, r), c in reasons.most_common() if l == lvl)
        print(f"  {lvl:28s} {n:5d}  {why}")
