set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"temp_dir\"; }" EXIT # clean up temp dir on exit

echo "Testing 1000000 steps"
./run.sh -g -lj --headless engine.StartBotTests --seed=112312001 --steps 1000000 $@
