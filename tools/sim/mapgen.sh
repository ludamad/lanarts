#!/bin/bash
# Generate every level headlessly and log the placed enemies/items per level.
# usage: [PLIST="1 3 4"] [OVERWORLD=classic] mapgen.sh outdir seed...   -> outdir/p<players>_s<seed>.log
# The overworld layout is picked at random per seed unless OVERWORLD names one.
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
RUNTIME=${RT:-$ROOT/runtime}
BIN=${LANARTS:-$ROOT/build/headless/src/lanarts}
out=$1; shift
mkdir -p "$out"; out=$(cd "$out" && pwd)
for p in ${PLIST:-1 3}; do for s in "$@"; do echo "$p $s"; done; done |
  xargs -P "${JOBS:-8}" -L 1 bash -c 'cd "'"$RUNTIME"'"; perl -e "alarm 600; exec @ARGV" "'"$BIN"'" engine.StartMapGenCheck --seed $1 --players $0 '"${OVERWORLD:+--overworld $OVERWORLD}"' > "'"$out"'/p$0_s$1.log" 2>&1; echo "EXIT $?" >> "'"$out"'/p$0_s$1.log"'
