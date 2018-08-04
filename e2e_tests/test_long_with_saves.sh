set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"temp_dir\"; }" EXIT # clean up temp dir on exit

echo "Testing 100 x 1000 steps"

# Run the initial run
./run.sh -g --headless engine.StartBotTests --seed=10001 --steps 1000 \
    --save "$temp_dir/save1.save" $@

# Run the followup 99 runs
for i in {2..100} ; do
    # report the file size:
    load_file="$temp_dir/save$((i-1)).save"
    save_file="$temp_dir/save$i.save"
    step_goal="$((i*1000))"
    echo "$load_file has size: `du -h $load_file`"
    # Run the followup run
    ./run.sh -g --headless engine.StartBotTests --seed=10001 --steps "$step_goal" \
        --load "$load_file" --save "$save_file" $@
done
