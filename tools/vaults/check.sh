#!/bin/bash
# Checks every vault of runtime/maps/vaults: engine.StartVaultCheck's static checks, then each layout
# forced into generated worlds (engine.StartMapGenCheck --vault) once per seed, in its own overworld
# theme (entrances and vaults for any theme go round the rotation).
# usage: [SEEDS="1 2"] [ONLY=regex] check.sh outdir
#   -> outdir/vaults.json (for gallery.py), outdir/placements.txt ("name:layout placed/tries"), logs
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BIN=${LANARTS:-$ROOT/build/headless/src/lanarts}
out=$1
mkdir -p "$out/logs"; out=$(cd "$out" && pwd)
cd "$ROOT/runtime"
"$BIN" engine.StartVaultCheck --export "$out/vaults.json" | grep -E "^(BAD|VAULTS)" | tee "$out/static.txt"
python3 - "$out/vaults.json" "${SEEDS:-1 2}" "${ONLY:-.}" > "$out/jobs.txt" <<'PY'
import json, re, sys
data = json.load(open(sys.argv[1]))
seeds = [int(s) for s in sys.argv[2].split()]
rotation = data["themes"]
for v in data["vaults"]:
    if not re.search(sys.argv[3], v["name"]):
        continue
    own = [t for t in v["themes"] if t in rotation]
    for i in range(1, len(v["layouts"]) + 1):
        for s in seeds:
            world = own[s % len(own)] if own else rotation[(s + i) % len(rotation)]
            print(v["name"], i, s, world)
PY
xargs -P "${JOBS:-8}" -L 1 bash -c 'perl -e "alarm 600; exec @ARGV" "'"$BIN"'" engine.StartMapGenCheck --seed $2 --overworld $3 --vault $0:$1 > "'"$out"'/logs/$0_$1_s$2.log" 2>&1; echo "EXIT $?" >> "'"$out"'/logs/$0_$1_s$2.log"' < "$out/jobs.txt"
python3 - "$out" > "$out/placements.txt" <<'PY'
import collections, sys
out = sys.argv[1]
tally = collections.OrderedDict()
for line in open(out + "/jobs.txt"):
    name, i, s, world = line.split()
    log = open("%s/logs/%s_%s_s%s.log" % (out, name, i, s)).read()
    t = tally.setdefault("%s:%s" % (name, i), [0, 0, []])
    t[1] += 1
    if ("VAULT %s:%s |" % (name, i)) in log and "EXIT 0" in log:
        t[0] += 1
    elif "EXIT 0" not in log:
        t[2].append("world failed s%s %s" % (s, world))
for key, (ok, n, notes) in tally.items():
    print(key, "%d/%d" % (ok, n), *notes)
PY
awk '{split($2, a, "/"); if (a[1] == 0) bad++; n++} END {printf "%d layouts, %d never placed\n", n, bad}' "$out/placements.txt"
grep -v " \([1-9][0-9]*\)/" "$out/placements.txt" || true
