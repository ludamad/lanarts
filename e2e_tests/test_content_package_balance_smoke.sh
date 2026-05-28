set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"$temp_dir\"; }" EXIT

env SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./run.sh --headless engine.StartBalanceRun \
    --steps 360 \
    --runs 1 \
    --classes Rogue \
    --levels 5 \
    --loadout_preset late \
    --bot_profile human \
    --encounter executioner-rescue-package \
    --package_intensity high \
    --package_components escort,orbit,pulse,sweep,rescue \
    --out="$temp_dir/balance_runs.csv" \
    --samples_out="$temp_dir/balance_samples.csv" \
    --spells_out="$temp_dir/balance_spells.csv" \
    --aggregate_out="$temp_dir/balance_classes.csv" \
    --spell_stats_out="$temp_dir/balance_spell_stats.csv" \
    --damage_stats_out="$temp_dir/balance_damage_stats.csv" \
    --damage_dealt_stats_out="$temp_dir/balance_damage_dealt_stats.csv" \
    --objective_damage_stats_out="$temp_dir/balance_objective_damage_stats.csv" \
    --enemy_stats_out="$temp_dir/balance_enemy_stats.csv" \
    --report_out="$temp_dir/balance_report.csv" \
    --json_out="$temp_dir/balance_runs.json"

test -s "$temp_dir/balance_runs.csv"
test -s "$temp_dir/balance_samples.csv"
test -s "$temp_dir/balance_classes.csv"
test -s "$temp_dir/balance_enemy_stats.csv"
test -s "$temp_dir/balance_report.csv"
test -s "$temp_dir/balance_runs.json"
grep -q "package_outcome" "$temp_dir/balance_runs.csv"
grep -q "package_components" "$temp_dir/balance_runs.csv"
grep -q "package_primary_hp_pct" "$temp_dir/balance_runs.csv"
grep -q "package_aggro_target_role" "$temp_dir/balance_runs.csv"
grep -q "package_next_threat_id" "$temp_dir/balance_runs.csv"
grep -q "package_pressure_max_count" "$temp_dir/balance_runs.csv"
grep -q "package_pressure_overlap_rate" "$temp_dir/balance_runs.csv"
grep -q "package_micro_score" "$temp_dir/balance_runs.csv"
grep -q "package_micro_dodges" "$temp_dir/balance_runs.csv"
grep -q "package_summon_warning_count" "$temp_dir/balance_runs.csv"
grep -q "package_rescue_save_rate" "$temp_dir/balance_runs.csv"
grep -q "package_rescue_perfect_rate" "$temp_dir/balance_runs.csv"
grep -q "package_rescue_early_rate" "$temp_dir/balance_runs.csv"
grep -q "package_rescue_timing_score" "$temp_dir/balance_runs.csv"
grep -q "package_orb_active_count" "$temp_dir/balance_samples.csv"
grep -q "package_pulse_active_count" "$temp_dir/balance_samples.csv"
grep -q "package_sweep_active_count" "$temp_dir/balance_samples.csv"
grep -q "package_rescue_active_count" "$temp_dir/balance_samples.csv"
grep -q "package_rescue_active_perfect" "$temp_dir/balance_samples.csv"
grep -q "package_rescue_next_frames" "$temp_dir/balance_runs.csv"
grep -q "package_rescue_pull_uptime_rate" "$temp_dir/balance_runs.csv"
grep -q "avg_package_pressure_overlap_rate" "$temp_dir/balance_classes.csv"
grep -q "avg_package_micro_score" "$temp_dir/balance_classes.csv"
grep -q "avg_package_micro_dodges" "$temp_dir/balance_classes.csv"
grep -q "avg_package_rescue_pull_uptime_rate" "$temp_dir/balance_classes.csv"
grep -q "avg_package_rescue_perfect_rate" "$temp_dir/balance_classes.csv"
grep -q "avg_package_rescue_early_rate" "$temp_dir/balance_classes.csv"
grep -q "avg_package_rescue_timing_score" "$temp_dir/balance_classes.csv"
grep -q "avg_package_rescue_save_rate" "$temp_dir/balance_classes.csv"
grep -q "avg_package_sweep_active_count" "$temp_dir/balance_classes.csv"
grep -q "package_pressure" "$temp_dir/balance_report.csv"
grep -q "drill_scorecard" "$temp_dir/balance_report.csv"
grep -q "avg_package_rescue_pull_uptime_rate" "$temp_dir/balance_report.csv"
grep -q "avg_package_rescue_timing_score" "$temp_dir/balance_report.csv"
grep -q "executioner-rescue-package" "$temp_dir/balance_runs.json"
grep -q "escort,orbit,pulse,sweep,rescue" "$temp_dir/balance_runs.json"
