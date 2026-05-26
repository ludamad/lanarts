set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"$temp_dir\"; }" EXIT

./run.sh --headless engine.StartMonsterEvals \
    --steps 300 \
    --runs 1 \
    --level 1 \
    --monster GiantRat,Hound \
    --pack "RatPack=GiantRat*2" \
    --configs BaseFighter \
    --out="$temp_dir/monster_runs.csv" \
    --aggregate_out="$temp_dir/monster_aggregate.csv" \
    --report_out="$temp_dir/monster_report.csv" \
    --json_out="$temp_dir/monster_eval.json" \
    $@

test -s "$temp_dir/monster_runs.csv"
test -s "$temp_dir/monster_aggregate.csv"
test -s "$temp_dir/monster_report.csv"
test -s "$temp_dir/monster_eval.json"
