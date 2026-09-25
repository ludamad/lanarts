#!/usr/bin/env python3
"""Run combat sims for monsters x classes x levels x players x seeds in parallel. Usage: runsim.py out.txt comp.json [monster ...]"""
import sys, json, subprocess, os, concurrent.futures as cf
import tempfile
ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
RUNTIME = os.environ.get("RT", os.path.join(ROOT, "runtime"))
BINARY = os.environ.get("LANARTS", os.path.join(ROOT, "build", "headless", "src", "lanarts"))
CLASSES = os.environ.get("CLASSES", "Fighter Stormcaller").split()
LEVELS = [int(x) for x in os.environ.get("LEVELS", "1 2 3 4 5 6 7 8").split()]
SEEDS = [1, 2, 3]
out, compf = sys.argv[1], sys.argv[2]
mons = sys.argv[3:]
if not mons:
    c = json.load(open(compf))
    mons = sorted({m for P in c.values() for lab in P["comp"].values() for m in lab})
jobs = [f"{cl} {L} {P} {s} {m}" for m in mons for cl in CLASSES for P in [int(x) for x in os.environ.get("PLAYERS", "1 3").split()] for L in LEVELS for s in SEEDS]
N = int(os.environ.get("JOBS", os.cpu_count() or 8))
chunks = [jobs[i::N] for i in range(N)]
def run(i):
    jf = os.path.join(tempfile.gettempdir(), f"lanarts_jobs_{os.getpid()}_{i}.txt")
    open(jf, "w").write("\n".join(chunks[i]) + "\n")
    r = subprocess.run(["perl", "-e", "alarm 3000; exec @ARGV", BINARY, "engine.StartCombatSim", "--jobs", jf, "--steps", "12000"],
                       cwd=RUNTIME, capture_output=True, text=True, errors="replace")
    lines = [l for l in r.stdout.splitlines() if l.startswith("SIM ")]
    if "SIM DONE" not in lines: print("chunk", i, "incomplete", len(lines), r.stderr[-500:], r.stdout[-800:])
    os.remove(jf)
    return [l for l in lines if l != "SIM DONE"]
with cf.ThreadPoolExecutor(N) as ex:
    res = list(ex.map(run, range(N)))
with open(out, "a") as f:
    for r in res:
        for l in r: f.write(l + "\n")
print(len(jobs), "jobs", sum(map(len, res)), "results")
