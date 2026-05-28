#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

temp_dir=$(mktemp -d)
trap 'rm -rf "$temp_dir"' EXIT

repo_dir=$(pwd)
headless_exe="${LANARTS_HEADLESS_EXE:-$repo_dir/build_debug_headless/src/lanarts}"
native_exe="${LANARTS_NATIVE_EXE:-$repo_dir/build_debug/src/lanarts}"

build_env=(SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1)
skip_build="${LANARTS_SKIP_BUILD:-0}"
draw_smoke="${LANARTS_DRAW_SMOKE:-0}"

if [[ "$skip_build" == "0" ]]; then
    env "${build_env[@]}" ./run.sh --headless --build >/dev/null
    if [[ "$draw_smoke" != "0" ]]; then
        env "${build_env[@]}" ./run.sh --build >/dev/null
    fi
fi

if [[ ! -x "$headless_exe" ]]; then
    echo "Native headless executable not found: $headless_exe" >&2
    echo "Run without LANARTS_SKIP_BUILD=1, or set LANARTS_HEADLESS_EXE." >&2
    exit 1
fi
if [[ "$draw_smoke" != "0" && ! -x "$native_exe" ]]; then
    echo "Native draw executable not found: $native_exe" >&2
    echo "Run without LANARTS_SKIP_BUILD=1, or set LANARTS_NATIVE_EXE." >&2
    exit 1
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
grep -q "package_micro_score" "$temp_dir/dicebinder_runs.csv"
grep -q "package_micro_counter_windows" "$temp_dir/dicebinder_runs.csv"
grep -q "package_aggro_player_rate" "$temp_dir/dicebinder_runs.csv"
grep -q "package_aggro_inferred_rate" "$temp_dir/dicebinder_runs.csv"
grep -q "package_aggro_switches" "$temp_dir/dicebinder_runs.csv"
grep -q "package_micro_punish_total" "$temp_dir/dicebinder_runs.csv"
grep -q "package_micro_punish_break_total" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_rolls_total" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_roll_animation_frames" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_roll_buildup_fx" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_roll_face_swaps" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_roll_lock_cues" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_roll_suspense_pips" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_cooldown_trim_frames" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_longest_high_streak" "$temp_dir/dicebinder_runs.csv"
grep -q "dice_dash_frames" "$temp_dir/dicebinder_runs.csv"
grep -q "avg_dice_rolls_total" "$temp_dir/dicebinder_report.csv"
grep -q "avg_dice_roll_animation_frames" "$temp_dir/dicebinder_classes.csv"
grep -q "avg_dice_roll_buildup_fx" "$temp_dir/dicebinder_classes.csv"
grep -q "avg_dice_roll_face_swaps" "$temp_dir/dicebinder_classes.csv"
grep -q "avg_dice_roll_lock_cues" "$temp_dir/dicebinder_classes.csv"
grep -q "avg_dice_roll_suspense_pips" "$temp_dir/dicebinder_classes.csv"
grep -q "avg_package_micro_score" "$temp_dir/dicebinder_report.csv"
grep -q "avg_package_micro_counter_windows" "$temp_dir/dicebinder_classes.csv"
grep -q "executioner-rescue-package" "$temp_dir/rescue_runs.json"
grep -q "package_rescue_save_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_aggro_ally_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_pull_uptime_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_timed_pull_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_pull_discipline_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_taunt_strain_rate" "$temp_dir/rescue_runs.csv"
grep -q "avg_package_rescue_timed_pull_rate" "$temp_dir/rescue_classes.csv"
grep -q "avg_package_rescue_pull_discipline_rate" "$temp_dir/rescue_classes.csv"
grep -q "avg_package_rescue_taunt_strain_rate" "$temp_dir/rescue_classes.csv"
grep -q "package_rescue_early_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_timing_score" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_late_cover_rate" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_perfect_uncovered_frames" "$temp_dir/rescue_runs.csv"
grep -q "avg_package_rescue_late_cover_rate" "$temp_dir/rescue_classes.csv"
grep -q "package_rescue_window_cues" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_counter_windows" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_counter_damage_total" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_fatal_misses" "$temp_dir/rescue_runs.csv"
grep -q "package_rescue_survived_misses" "$temp_dir/rescue_runs.csv"
grep -q "package_micro_dodges" "$temp_dir/rescue_runs.csv"
grep -q "package_sweep_active_count" "$temp_dir/rescue_runs.csv"
grep -q "sweep" "$temp_dir/rescue_runs.json"

if [[ "$draw_smoke" != "0" ]]; then
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
