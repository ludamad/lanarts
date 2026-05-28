#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

usage() {
    cat <<'USAGE'
Usage:
  ./e2e_tests/run_content_lab_native.sh [options] [-- extra StartBalanceRun args]

Runs a selected content-lab encounter through the native binary, not Emscripten.
Audio is always muted. Outputs are written under saves/content_lab_native by default.

Common options:
  --draw                         Use the normal native binary and call GameState.draw().
  --class NAME                   Class to simulate. Default: Dicebinder.
  --level N                      Starting level. Default: 5.
  --loadout_preset NAME          Loadout preset. Default: late.
  --loadout ITEMS                Extra comma-separated loadout items.
  --bot_profile NAME             perfect, skilled, human, or sloppy. Default: human.
  --encounter ID                 Content-lab encounter id. Default: gragh-duel.
  --package_intensity NAME       Package pressure preset. Default: high.
  --package_components LIST      Optional package components to isolate.
  --runs N                       Runs per class. Default: 1.
  --steps N                      Maximum frames per run. Default: 900.
  --seed N                       First simulation seed. Default: 12345678.
  --sample_interval N            Sample interval. Default: 120.
  --draw_interval N              Draw interval when --draw is set. Default: 60.
  --label NAME                   Output directory suffix.
  --out_dir DIR                  Output directory. Overrides generated directory.
  --no-build                     Reuse existing native binary.

Examples:
  LANARTS_SKIP_BUILD=1 ./e2e_tests/run_content_lab_native.sh \
    --class Rogue --encounter executioner-rescue-package \
    --package_components escort,orbit,pulse,sweep,rescue

  ./e2e_tests/run_content_lab_native.sh --draw --class Dicebinder --encounter gragh-duel --steps 240
USAGE
}

mode="headless"
class_name="Dicebinder"
level="5"
loadout_preset="late"
loadout=""
bot_profile="human"
encounter="gragh-duel"
package_intensity="high"
package_components=""
runs="1"
steps="900"
seed="12345678"
sample_interval="120"
draw_interval="60"
label=""
out_dir=""
skip_build="${LANARTS_SKIP_BUILD:-0}"
extra_args=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --help|-h)
            usage
            exit 0
            ;;
        --draw)
            mode="draw"
            shift
            ;;
        --headless)
            mode="headless"
            shift
            ;;
        --class|--classes)
            class_name="$2"
            shift 2
            ;;
        --level|--levels)
            level="$2"
            shift 2
            ;;
        --loadout_preset)
            loadout_preset="$2"
            shift 2
            ;;
        --loadout)
            loadout="$2"
            shift 2
            ;;
        --bot_profile)
            bot_profile="$2"
            shift 2
            ;;
        --encounter)
            encounter="$2"
            shift 2
            ;;
        --package_intensity)
            package_intensity="$2"
            shift 2
            ;;
        --package_components)
            package_components="$2"
            shift 2
            ;;
        --runs)
            runs="$2"
            shift 2
            ;;
        --steps)
            steps="$2"
            shift 2
            ;;
        --seed)
            seed="$2"
            shift 2
            ;;
        --sample_interval)
            sample_interval="$2"
            shift 2
            ;;
        --draw_interval)
            draw_interval="$2"
            shift 2
            ;;
        --label)
            label="$2"
            shift 2
            ;;
        --out_dir)
            out_dir="$2"
            shift 2
            ;;
        --no-build)
            skip_build="1"
            shift
            ;;
        --)
            shift
            extra_args+=("$@")
            break
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

build_env=(SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1)

if [[ "$skip_build" == "0" ]]; then
    if [[ "$mode" == "draw" ]]; then
        env "${build_env[@]}" ./run.sh --build >/dev/null
    else
        env "${build_env[@]}" ./run.sh --headless --build >/dev/null
    fi
fi

repo_dir="$(pwd)"
if [[ "$mode" == "draw" ]]; then
    exe="${LANARTS_NATIVE_EXE:-$repo_dir/build_debug/src/lanarts}"
else
    exe="${LANARTS_HEADLESS_EXE:-$repo_dir/build_debug_headless/src/lanarts}"
fi

if [[ ! -x "$exe" ]]; then
    echo "Native executable not found: $exe" >&2
    echo "Run without --no-build, or set LANARTS_HEADLESS_EXE/LANARTS_NATIVE_EXE." >&2
    exit 1
fi

if [[ -z "$out_dir" ]]; then
    stamp="$(date +%Y%m%d-%H%M%S)"
    suffix="${label:-${mode}_${class_name}_${encounter}}"
    suffix="${suffix//[^A-Za-z0-9_.-]/_}"
    out_dir="saves/content_lab_native/${stamp}_${suffix}"
fi
mkdir -p "$out_dir"
out_dir="$(cd "$out_dir" && pwd)"

balance_args=(
    engine.StartBalanceRun
    --steps "$steps"
    --runs "$runs"
    --seed "$seed"
    --classes "$class_name"
    --levels "$level"
    --loadout_preset "$loadout_preset"
    --bot_profile "$bot_profile"
    --encounter "$encounter"
    --package_intensity "$package_intensity"
    --sample_interval "$sample_interval"
)

if [[ -n "$loadout" ]]; then
    balance_args+=(--loadout "$loadout")
fi
if [[ -n "$package_components" ]]; then
    balance_args+=(--package_components "$package_components")
fi
if [[ "$mode" == "draw" ]]; then
    balance_args+=(--draw_interval "$draw_interval")
fi
if [[ "${#extra_args[@]}" -gt 0 ]]; then
    balance_args+=("${extra_args[@]}")
fi

echo "Running native content lab:"
echo "  mode: $mode"
echo "  exe: $exe"
echo "  out: $out_dir"

(
    cd runtime
    env "${build_env[@]}" "$exe" "${balance_args[@]}" \
        --out="$out_dir/runs.csv" \
        --samples_out="$out_dir/samples.csv" \
        --spells_out="$out_dir/spells.csv" \
        --aggregate_out="$out_dir/classes.csv" \
        --spell_stats_out="$out_dir/spell_stats.csv" \
        --damage_stats_out="$out_dir/damage_stats.csv" \
        --damage_dealt_stats_out="$out_dir/damage_dealt_stats.csv" \
        --objective_damage_stats_out="$out_dir/objective_damage_stats.csv" \
        --enemy_stats_out="$out_dir/enemy_stats.csv" \
        --report_out="$out_dir/report.csv" \
        --json_out="$out_dir/runs.json"
)

if command -v ruby >/dev/null 2>&1; then
    ruby -rcsv -e '
        row = CSV.read(ARGV[0], headers: true).first || {}
        keys = %w[outcome frames package_outcome package_micro_score package_micro_counter_windows package_micro_counter_damage_total package_micro_counter_stagger_frames package_micro_punish_total package_micro_punish_break_total package_aggro_player_rate package_aggro_ally_rate package_aggro_inferred_rate package_aggro_switches dice_rolls_total dice_roll_buildup_fx dice_roll_face_swaps dice_roll_lock_cues dice_roll_surge_cues dice_roll_finale_fx dice_roll_suspense_pips dice_roll_reveal_cues dice_cooldown_trim_frames dice_high_rolls dice_longest_high_streak dice_repeat_rolls dice_anti_repeat_rerolls dice_dash_effective_refreshes dice_power_effective_refreshes package_rescue_save_rate package_rescue_early_rate package_rescue_timing_score package_rescue_avg_clutch_margin package_rescue_avg_perfect_margin package_rescue_avg_timed_pull_margin package_rescue_window_cues package_rescue_perfect_window_cues package_rescue_late_window_frames package_rescue_late_uncovered_frames package_rescue_late_cover_rate package_rescue_perfect_window_frames package_rescue_perfect_uncovered_frames package_rescue_perfect_cover_rate package_rescue_taunt_strain_rate package_rescue_taunt_strain_breaks package_rescue_counter_windows package_rescue_counter_damage_total package_rescue_fatal_misses package_rescue_survived_misses package_pressure_overlap_rate package_pressure_sync_rate package_pressure_sync_bursts package_pressure_sync_peak]
        puts keys.map { |key| "#{key}=#{row[key] || ""}" }.join(" ")
    ' "$out_dir/runs.csv"
fi

echo "Native content lab outputs: $out_dir"
