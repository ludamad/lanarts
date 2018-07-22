set -e

temp_dir=`mktemp -d`
trap "{ rm -rf \"temp_dir\"; }" EXIT # clean up temp dir on exit

echo "Testing 1000 different spawns"
for i in {1..1000} ; do
    ./run.sh -g --headless engine.StartBotTests "--seed=$i" --steps 1
done
