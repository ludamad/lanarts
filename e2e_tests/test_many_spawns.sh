set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"temp_dir\"; }" EXIT # clean up temp dir on exit

echo "Testing 10 different spawns"
for i in {1..10} ; do
    ./run.sh -g -f --headless engine.StartBotTests "--seed=$i" --steps 1
done
