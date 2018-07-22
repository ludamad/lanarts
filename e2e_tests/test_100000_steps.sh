set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"temp_dir\"; }" EXIT # clean up temp dir on exit

echo "Testing 100000 steps"
./run.sh -g --headless engine.StartBotTests --steps 100000 
