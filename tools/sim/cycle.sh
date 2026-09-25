#!/bin/bash
# One balance iteration: generate 40 worlds for 1/3/4 players, average the enemy composition per level,
# then simulate a Fighter party against each monster at each character level.
# usage: cycle.sh tag   -> out/gen_<tag>/ (logs, comp.json, comp.txt) and out/sim_<tag>.txt
#        python3 metric.py out/gen_<tag>/comp.json out/sim_<tag>.txt   (difficulty table)
set -e
cd "$(dirname "$0")"
t=$1; mkdir -p out
rm -rf out/gen_$t out/sim_$t.txt
PLIST="1 3 4" ./mapgen.sh out/gen_$t $(seq 21 60)
grep -h EXIT out/gen_$t/*.log | sort | uniq -c
python3 comp.py out/gen_$t > out/gen_$t/comp.txt
CLASSES=Fighter PLAYERS="1 3 4" python3 runsim.py out/sim_$t.txt out/gen_$t/comp.json
