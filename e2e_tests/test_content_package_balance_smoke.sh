set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"$temp_dir\"; }" EXIT

./run.sh --headless engine.StartBalanceRun \
    --steps 360 \
    --runs 1 \
    --classes Rogue \
    --levels 5 \
    --loadout_preset late \
    --bot_profile human \
    --encounter executioner-blade-package \
    --package_intensity high \
    --package_components orbit,pulse \
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
test -s "$temp_dir/balance_runs.json"
grep -q "package_outcome" "$temp_dir/balance_runs.csv"
grep -q "package_components" "$temp_dir/balance_runs.csv"
grep -q "package_primary_hp_pct" "$temp_dir/balance_runs.csv"
grep -q "package_aggro_target_role" "$temp_dir/balance_runs.csv"
grep -q "package_rescue_save_rate" "$temp_dir/balance_runs.csv"
grep -q "package_orb_active_count" "$temp_dir/balance_samples.csv"
grep -q "package_pulse_active_count" "$temp_dir/balance_samples.csv"
grep -q "package_rescue_active_count" "$temp_dir/balance_samples.csv"
grep -q "package_rescue_next_frames" "$temp_dir/balance_runs.csv"
grep -q "avg_package_rescue_save_rate" "$temp_dir/balance_classes.csv"
grep -q "executioner-blade-package" "$temp_dir/balance_runs.json"
grep -q "orbit,pulse" "$temp_dir/balance_runs.json"
