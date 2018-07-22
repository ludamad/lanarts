set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"temp_dir\"; }" EXIT # clean up temp dir on exit

echo "Creating log of 100000 steps"
./run.sh -g --headless engine.StartBotTests --steps 100000 --event_log="$temp_dir/out.log"
echo "Comparing 100000 step playthrough"
./run.sh -g --headless engine.StartBotTests --steps 100000 --comparison_event_log="$temp_dir/out.log"
