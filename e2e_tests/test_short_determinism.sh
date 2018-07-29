set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"temp_dir\"; }" EXIT # clean up temp dir on exit

echo "Creating log of 1000 steps"
./run.sh -g --headless engine.StartBotTests --steps 1000 --event_log="$temp_dir/out.log" $@
echo "Comparing 1000 step playthrough"
./run.sh -g --headless engine.StartBotTests --steps 1000 --comparison_event_log="$temp_dir/out.log" $@
