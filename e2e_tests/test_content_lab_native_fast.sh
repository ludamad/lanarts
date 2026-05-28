#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

temp_dir=$(mktemp -d)
trap 'rm -rf "$temp_dir"' EXIT

repo_dir=$(pwd)
headless_exe="${LANARTS_HEADLESS_EXE:-$repo_dir/build_debug_headless/src/lanarts}"
native_exe="${LANARTS_NATIVE_EXE:-$repo_dir/build_debug/src/lanarts}"

build_env=(SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1)

if [[ "${LANARTS_SKIP_BUILD:-0}" == "0" ]]; then
    env "${build_env[@]}" ./run.sh --headless --build >/dev/null
    if [[ "${LANARTS_DRAW_SMOKE:-0}" != "0" ]]; then
        env "${build_env[@]}" ./run.sh --build >/dev/null
    fi
fi

run_balance() {
    local exe="$1"
    local name="$2"
    shift 2
    (
        cd runtime
        env "${build_env[@]}" "$exe" "$@" \
        --out="$temp_dir/${name}_runs.csv" \
        --samples_out="$temp_dir/${name}_samples.csv" \
        --spells_out="$temp_dir/${name}_spells.csv" \
        --aggregate_out="$temp_dir/${name}_classes.csv" \
        --spell_stats_out="$temp_dir/${name}_spell_stats.csv" \
        --damage_stats_out="$temp_dir/${name}_damage_stats.csv" \
        --damage_dealt_stats_out="$temp_dir/${name}_damage_dealt_stats.csv" \
        --objective_damage_stats_out="$temp_dir/${name}_objective_damage_stats.csv" \
        --enemy_stats_out="$temp_dir/${name}_enemy_stats.csv" \
        --report_out="$temp_dir/${name}_report.csv" \
        --json_out="$temp_dir/${name}_runs.json"
    )
}

run_balance "$headless_exe" dicebinder \
    engine.StartBalanceRun \
    --steps 700 \
    --runs 1 \
    --classes Dicebinder \
    --levels 5 \
    --loadout_preset late \
    --bot_profile human \
    --encounter gragh-duel \
    --package_intensity high

run_balance "$headless_exe" rescue \
    engine.StartBalanceRun \
    --steps 700 \
    --runs 1 \
    --classes Rogue \
    --levels 5 \
    --loadout_preset late \
    --bot_profile human \
    --encounter executioner-rescue-package \
    --package_intensity high \
    --package_components escort,orbit,pulse,sweep,rescue

test -s "$temp_dir/dicebinder_runs.csv"
test -s "$temp_dir/rescue_runs.csv"
grep -q "Dicebinder" "$temp_dir/dicebinder_runs.csv"
grep -q "gragh-duel" "$temp_dir/dicebinder_runs.json"
grep -q "package_pressure_max_count" "$temp_dir/dicebinder_runs.csv"
grep -q "executioner-rescue-package" "$temp_dir/rescue_runs.json"
grep -q "package_rescue_save_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_pull_uptime_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_sweep_active_count" "$temp_dir/rescue_runs.csv"
grep -q "sweep" "$temp_dir/rescue_runs.json"

if [[ "${LANARTS_DRAW_SMOKE:-0}" != "0" ]]; then
    run_balance "$native_exe" dicebinder_draw \
        engine.StartBalanceRun \
        --steps 180 \
        --runs 1 \
        --classes Dicebinder \
        --levels 5 \
        --loadout_preset late \
        --bot_profile human \
        --encounter gragh-duel \
        --package_intensity high \
        --draw_interval 60

    run_balance "$native_exe" rescue_draw \
        engine.StartBalanceRun \
        --steps 500 \
        --runs 1 \
        --classes Rogue \
        --levels 5 \
        --loadout_preset late \
        --bot_profile human \
        --encounter executioner-rescue-package \
        --package_intensity high \
        --package_components escort,orbit,pulse,sweep,rescue \
        --draw_interval 60
fi

echo "Native content lab smoke complete: $temp_dir"
