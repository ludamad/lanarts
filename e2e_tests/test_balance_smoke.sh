set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"$temp_dir\"; }" EXIT

./run.sh --headless engine.StartBalanceRun \
    --steps 250 \
    --runs 1 \
    --classes Fighter \
    --out="$temp_dir/balance_runs.csv" \
    --samples_out="$temp_dir/balance_samples.csv" \
    --spells_out="$temp_dir/balance_spells.csv" \
    --aggregate_out="$temp_dir/balance_classes.csv" \
    --spell_stats_out="$temp_dir/balance_spell_stats.csv" \
    --enemy_stats_out="$temp_dir/balance_enemy_stats.csv" \
    --report_out="$temp_dir/balance_report.csv" \
    --json_out="$temp_dir/balance_runs.json" \
    $@

test -s "$temp_dir/balance_runs.csv"
test -s "$temp_dir/balance_samples.csv"
test -s "$temp_dir/balance_spells.csv"
test -s "$temp_dir/balance_classes.csv"
test -s "$temp_dir/balance_spell_stats.csv"
test -s "$temp_dir/balance_enemy_stats.csv"
test -s "$temp_dir/balance_report.csv"
test -s "$temp_dir/balance_runs.json"
