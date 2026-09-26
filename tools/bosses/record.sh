#!/bin/bash
# Records a staged boss fight (engine.StartBossShowcase, graphical build) as an animated WebP of the
# game view.
# usage: record.sh out.webp --monsters "Royal Jelly" --class Fighter --level 5 [--players 2]
#            [--tileset hive] [--steps 1500] [--every 3]    (any other StartBossShowcase options)
set -e
export SDL_AUDIODRIVER=${SDL_AUDIODRIVER:-dummy}  # silent: no sound while recording
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BIN=${LANARTS:-$ROOT/build/debug/src/lanarts}
out=$1; shift
frames=$(mktemp -d)
every=3
args=("$@")
for ((i = 0; i < ${#args[@]}; i++)); do [ "${args[$i]}" = "--every" ] && every=${args[$((i + 1))]}; done
(cd "$ROOT/runtime" && "$BIN" engine.StartBossShowcase --out "$frames" --every "$every" "$@" > "$frames/log.txt" 2>&1) \
    || { tail -20 "$frames/log.txt"; exit 1; }
PY=${PYTHON:-python3}
"$PY" - "$frames" "$out" "$every" <<'PYEOF'
import glob, sys
from PIL import Image
frames_dir, out, every = sys.argv[1], sys.argv[2], int(sys.argv[3])
paths = sorted(glob.glob(frames_dir + "/frame_*.bmp"))
# The game view: the left 640 columns, below the message log and above the action bar
images = [Image.open(p).convert("RGB").crop((0, 100, 640, 568)) for p in paths]
ms = round(1000 * every / 60)  # 60 game steps a second
images[0].save(out, save_all=True, append_images=images[1:], duration=ms, loop=0, quality=60, method=6)
print("%s: %d frames, %.1fs" % (out, len(images), len(images) * ms / 1000))
PYEOF
rm -rf "$frames"
