#!/bin/bash
# Fuzzes the game with bot parties (engine.StartFuzz). Each seed plays a fresh world with 1-4 bots of
# random classes in a random overworld of the rotation, saving at random frames, then a second run
# loads that save and plays on. Crashes, Lua errors and hangs are grouped by their first error
# line. With LANARTS=build/headless-sanitize/src/lanarts, AddressSanitizer errors and UBSan reports
# are grouped too.
# usage: [STEPS=20000] [JOBS=8] [LANARTS=binary] fuzz.sh outdir seed...   eg fuzz.sh /tmp/fuzz $(seq 1 64)
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BIN=${LANARTS:-$ROOT/build/headless/src/lanarts}
out=$1; shift
mkdir -p "$out"; out=$(cd "$out" && pwd)
OVERWORLDS=(classic ringwood farmland crossroads craters spiral rootways patchwork)
STEPS=${STEPS:-20000}
for s in "$@"; do
    echo "$s $(( s % 4 + 1 )) ${OVERWORLDS[$(( s % ${#OVERWORLDS[@]} ))]}"
done | xargs -P "${JOBS:-8}" -L 1 bash -c '
    s=$0; p=$1; o=$2; base="'"$out"'/s$s"
    cd "'"$ROOT"'/runtime"
    run() { perl -e "alarm 900; exec @ARGV" "'"$BIN"'" engine.StartFuzz --seed $s --players $p "$@" > "$log" 2>&1; echo "EXIT $?" >> "$log"; }
    log=$base.fresh.log; run --overworld $o --steps '"$STEPS"' --save $base.save
    classes=$(sed -n "s/^FUZZ START .*classes=\(.*\) overworld=.*/\1/p" $log)
    if grep -q "^FUZZ DONE" $log; then
        log=$base.resume.log; run --classes "$classes" --load $base.save --steps $(( '"$STEPS"' * 2 )) --save $base.save2
    fi
    echo "s$s p$p $o $classes"'
python3 - "$out" <<'PY'
import glob, os, re, sys, collections
out = sys.argv[1]
fails = collections.OrderedDict()
n = 0
for log in sorted(glob.glob(out + "/s*.log")):
    n += 1
    text = re.sub(r"\x1b\[[0-9;]*m", "", open(log, errors="replace").read())
    # UBSan reports and carries on, so it can flag runs that otherwise finish
    for ub in sorted(set(re.findall(r"^(\S+:\d+:\d+: runtime error: .*)$", text, re.M))):
        where, what = ub.split(": runtime error: ", 1)
        what = re.sub(r"-?\d+(\.\d+)?", "N", what)  # one group per kind of report, whatever the values
        fails.setdefault("UBSan %s: %s" % (re.sub(r"^.*/(src|libs|dependencies)/", r"\1/", where), what), []).append(os.path.basename(log))
    if "FUZZ DONE" in text:
        continue
    code = re.findall(r"^EXIT (\d+)", text, re.M)
    code = code[-1] if code else "?"
    m = re.search(r"ERROR: AddressSanitizer: (.*)", text) or re.search(r"^@ (.*)$", text, re.M) or re.search(r"^(Uncaught exception: .*)$", text, re.M) or re.search(r"FUZZ ERROR (.*)", text) or re.search(r"(terminating due to .*|Assertion failed.*|Segmentation fault.*)", text)
    if m:
        why = m.group(1).strip()
    elif code == "142":
        why = "hang (timed out)"
    else:
        why = "exit %s without an error message" % code
    start = re.search(r"^FUZZ START (.*)$", text, re.M)
    fails.setdefault(why, []).append("%s  [%s]" % (os.path.basename(log), start.group(1) if start else "did not start"))
print("%d runs, %d problems, %d distinct" % (n, sum(len(v) for v in fails.values()), len(fails)))
for why, runs in fails.items():
    print("\n%dx %s" % (len(runs), why))
    for r in runs[:5]:
        print("   ", r)
PY
