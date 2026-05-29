#!/usr/bin/env bash
# Large-scale class-interestingness experiment.
#
# Sweeps every playable class x every late boss x bot profile through the native
# balance runner and writes one aggregate classes.csv per (boss, profile) cell.
# A companion aggregator (aggregate_class_interestingness.py) combines them into a
# per-class interestingness report.
#
# Always run muted. Reuses prebuilt binaries (LANARTS_SKIP_BUILD=1).
#
# Env knobs (defaults = "comprehensive"):
#   RUNS (6), STEPS (1800), PROFILES ("human perfect"), JOBS (4), LEVEL (5),
#   LOADOUT (late), INTENSITY (high), OUT_DIR (saves/experiments/class_interest)
set -uo pipefail
cd "$(dirname "$0")/.."

RUNS="${RUNS:-6}"
STEPS="${STEPS:-1800}"
PROFILES="${PROFILES:-human perfect}"
JOBS="${JOBS:-4}"
LEVEL="${LEVEL:-5}"
LOADOUT="${LOADOUT:-late}"
INTENSITY="${INTENSITY:-high}"
SEED="${SEED:-51000}"
OUT_DIR="${OUT_DIR:-saves/experiments/class_interest}"

CLASSES="${CLASSES:-Fighter,Necromancer,Pyrocaster,Stormcaller,Templar,Rogue,Dicebinder}"
if [[ -n "${BOSSES:-}" ]]; then
  read -r -a BOSSES <<< "$BOSSES"
else
  BOSSES=(
    gragh-duel
    red-dragon-duel
    executioner-blade-package
    hell-warrior-flame-package
    mana-sapper-ritual-package
    stara-pain-star-package
    pixulloch-web-package
    waxanarian-venom-package
    nesso-arrow-storm-package
    crypt-keeper-aura-package
  )
fi

exe="${LANARTS_HEADLESS_EXE:-build_debug_headless/src/lanarts}"
if [[ ! -x "$exe" ]]; then
  echo "Headless binary not found at $exe; build with ./run.sh --headless --build" >&2
  exit 1
fi

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"
env_run=(SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1)

run_cell() {
  local boss="$1" profile="$2"
  local cell="${boss}__${profile}"
  ( cd runtime && env "${env_run[@]}" "../$exe" engine.StartBalanceRun \
      --steps "$STEPS" --runs "$RUNS" --classes "$CLASSES" --levels "$LEVEL" \
      --loadout_preset "$LOADOUT" --bot_profile "$profile" \
      --encounter "$boss" --package_intensity "$INTENSITY" --seed "$SEED" \
      --aggregate_out="../$OUT_DIR/${cell}_classes.csv" \
      --out="../$OUT_DIR/${cell}_runs.csv" \
      --report_out="../$OUT_DIR/${cell}_report.csv" \
      >"../$OUT_DIR/${cell}.log" 2>&1 )
  echo "done $cell (exit $?)"
}

echo "Class-interestingness experiment: ${#BOSSES[@]} bosses x $(echo $PROFILES|wc -w) profiles, classes=$CLASSES, runs=$RUNS steps=$STEPS jobs=$JOBS"
active=0
for boss in "${BOSSES[@]}"; do
  for profile in $PROFILES; do
    run_cell "$boss" "$profile" &
    active=$((active+1))
    if (( active % JOBS == 0 )); then
      wait -n 2>/dev/null || wait
    fi
  done
done
wait

cells=$(ls "$OUT_DIR"/*_classes.csv 2>/dev/null | wc -l | tr -d ' ')
echo "EXPERIMENT COMPLETE: $cells cells written to $OUT_DIR"
python3 e2e_tests/aggregate_class_interestingness.py "$OUT_DIR" || true
