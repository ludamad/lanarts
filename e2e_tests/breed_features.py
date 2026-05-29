#!/usr/bin/env python3
"""Breed characters from movement-flow features and rank them by data.

Generates candidate feature loadouts (LANARTS_FEATURE_LOADOUT strings), runs each
on a weaving chassis (Rogue, which reliably dash-weaves) against a weave-fodder boss,
and scores the resulting character on engagement (flow), viability (a healthy win
band), and clean execution (micro). The lab becomes a generator: compose features ->
measure -> keep the combinations that breed interesting, skill-expressive characters.

Usage: python3 e2e_tests/breed_features.py [runs] [steps]
Always muted; reuses the prebuilt headless binary.
"""
import csv, os, subprocess, sys, statistics

RUNS = int(sys.argv[1]) if len(sys.argv) > 1 else 4
STEPS = int(sys.argv[2]) if len(sys.argv) > 2 else 1500
CHASSIS = "Rogue"
BOSS = "gragh-warband-package"
SEED = "51000"
OUT = "saves/experiments/breed"
EXE = "build_debug_headless/src/lanarts"

# Candidate feature loadouts to breed (composable building blocks).
CANDIDATES = [
    "flux_weave@1.5",
    "flux_weave@2.6",
    "flux_weave@3.5",
    "gambler_weave@1.6",
    "gambler_weave@2.4",
    "radiant_nova:min_tier=2",
    "radiant_nova:min_tier=1",
    "flux_weave@2.0+radiant_nova:min_tier=2",
    "gambler_weave@1.6+radiant_nova:min_tier=2",
    "flux_weave@1.5+gambler_weave@1.5",
    "flux_weave@2.0+gambler_weave@1.5+radiant_nova:min_tier=2",
]

def fnum(d, k):
    try:
        return float(d.get(k, "") or 0)
    except (TypeError, ValueError):
        return 0.0

def run_candidate(loadout):
    cell = loadout.replace("/", "_").replace(" ", "")
    agg = os.path.abspath(f"{OUT}/{cell}_classes.csv")
    env = dict(os.environ)
    env.update(SDL_AUDIODRIVER="dummy", LANARTS_MUTE_AUDIO="1", LANARTS_SMALL="1",
               LANARTS_FEATURE_LOADOUT=loadout)
    subprocess.run(
        ["../" + EXE, "engine.StartBalanceRun", "--steps", str(STEPS), "--runs", str(RUNS),
         "--classes", CHASSIS, "--levels", "5", "--loadout_preset", "late",
         "--bot_profile", "perfect", "--encounter", BOSS, "--package_intensity", "high",
         "--seed", SEED, "--aggregate_out=" + agg],
        cwd="runtime", env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        return list(csv.DictReader(open(agg)))[0]
    except Exception:
        return None

def score(row):
    flow = fnum(row, "avg_package_flow_active_rate")          # engagement (0..1)
    win = fnum(row, "win_rate")
    micro = fnum(row, "avg_package_micro_score")
    weave = fnum(row, "avg_package_weave_dodges")
    viability = max(0.0, 1.0 - abs(win - 0.55) / 0.55)        # tent peaking at 55% win
    clean = max(0.0, min(1.0, (micro + 50) / 100))
    engage = min(1.0, flow * 4)                               # flow is small; scale
    s = engage * 1.5 + viability * 1.0 + clean * 0.5
    return s, dict(flow=flow, win=win, micro=micro, weave=weave, viability=viability)

def main():
    os.makedirs(OUT, exist_ok=True)
    print(f"Breeding {len(CANDIDATES)} loadouts on {CHASSIS}/{BOSS} (runs={RUNS} steps={STEPS})...")
    results = []
    for ld in CANDIDATES:
        row = run_candidate(ld)
        if not row:
            print(f"  {ld:<52} FAILED")
            continue
        s, m = score(row)
        results.append((s, ld, m))
        print(f"  ran {ld:<52} win={m['win']*100:>3.0f}% flow={m['flow']*100:>4.1f}% weave={m['weave']:>4.1f} micro={m['micro']:>6.1f} score={s:.2f}")
    results.sort(reverse=True)
    print("\n=== BRED CHARACTERS, ranked by interestingness (engagement + viability + clean) ===")
    for s, ld, m in results:
        print(f"  {s:.2f}  {ld:<52} win={m['win']*100:>3.0f}% flow={m['flow']*100:>4.1f}% micro={m['micro']:>6.1f}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
