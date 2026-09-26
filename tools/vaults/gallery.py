#!/usr/bin/env python3
"""Builds a page showing every vault of runtime/maps/vaults, drawn with the game's own tiles.

usage: gallery.py outdir/vaults.json [outdir/placements.txt] out.html
(check.sh writes vaults.json and placements.txt.) Needs Pillow.
"""
import base64, io, json, os, re, sys
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RUNTIME = os.path.join(ROOT, "runtime")
HERE = os.path.dirname(os.path.abspath(__file__))

# Sprites drawn over the tiles for spawns and decorations
SPRITES = {
    "decor:statue": "features/sprites/statues/statue3.png",
    "decor:anvil": "features/sprites/anvil.png",
    "decor:tombstone": "features/sprites/tombstone.png",
    "decor:skull": "features/sprites/warning.png",
    "decor:shrub": "features/sprites/shrub.png",
    "spawn:door": "features/sprites/closed_door.png",
    "spawn:portal": "spr_gates/enter_vaults_open.png",
    "spawn:player": "classes/sprites/fighter.png",
    "spawn:gold": "items/misc/sprites/gold.png",
    "spawn:item": "items/misc/sprites/potion.png",
    "spawn:exp": "items/scrolls/sprites/scroll_mag.png",
    "spawn:enemy": "enemies/animals/sprites/rat.png",
    "spawn:brute": "enemies/undead/sprites/skeleton.png",
    "spawn:store": "features/sprites/shop.png",
}


def tilesets():
    """{tileset: {key: image pattern}} from tiles/Tilesets.lua, with its fallbacks filled in"""
    src = open(os.path.join(RUNTIME, "tiles/Tilesets.lua")).read()
    sets = {}
    for name, body in re.findall(r"M\.(\w+)\s*=\s*(?:Data\.tileset_create\s*)?\{(.*?)\n\}", src, re.S):
        keys = sets.setdefault(name, {})
        for key, pattern in re.findall(r'(\w+)\s*=\s*(?:Data\.tile_create\s*)?\{\s*images\s*=\s*Display\.images_load\s*\(?\s*"([^"]+)"', body):
            keys[key] = pattern
        for key, other, other_key in re.findall(r"(\w+)\s*=\s*M\.(\w+)\.(\w+)", body):
            keys[key] = sets[other][other_key]
    for keys in sets.values():
        keys.setdefault("wall_alt", keys.get("wall"))
        keys.setdefault("floor_alt", keys.get("floor"))
        keys.setdefault("floor_tunnel", keys.get("floor_alt"))
    return sets


def first_frame(pattern):
    path = pattern.split("%")[0]
    path = re.sub(r"\((\d+)-\d+\)", r"\1", path)
    im = Image.open(os.path.join(RUNTIME, path)).convert("RGBA")
    return im.crop((0, 0, 32, 32))


def main():
    data = json.load(open(sys.argv[1]))
    placements = {}
    if len(sys.argv) > 3:
        for line in open(sys.argv[2]):
            parts = line.split()
            placements[parts[0]] = parts[1]
    out = sys.argv[-1]

    sets = tilesets()
    refs = {"grass.floor"}
    for v in data["vaults"]:
        for spec in v["legend"].values():
            if spec.get("tile"):
                refs.add(spec["tile"])
    images = {}
    for ref in sorted(refs):
        ts, key = ref.split(".", 1)
        images[ref] = first_frame(sets[ts][key])
    for key, path in SPRITES.items():
        images[key] = first_frame(path)
    keys = sorted(images)
    cols = 16
    atlas = Image.new("RGBA", (cols * 32, ((len(keys) + cols - 1) // cols) * 32))
    for i, key in enumerate(keys):
        atlas.paste(images[key], ((i % cols) * 32, (i // cols) * 32))
    buf = io.BytesIO()
    atlas.save(buf, "PNG", optimize=True)

    payload = {
        "vaults": data["vaults"],
        "themes": data["themes"],
        "dungeons": data["dungeons"],
        "atlas": {k: i for i, k in enumerate(keys)},
        "atlasCols": cols,
        "placements": placements,
    }
    page = open(os.path.join(HERE, "gallery.template.html")).read()
    page = page.replace("/*ATLAS*/", "data:image/png;base64," + base64.b64encode(buf.getvalue()).decode())
    page = page.replace("/*DATA*/null", json.dumps(payload, separators=(",", ":")))
    open(out, "w").write(page)
    print("%s: %d vaults, %d layouts, atlas %d images" % (out, len(data["vaults"]), sum(len(v["layouts"]) for v in data["vaults"]), len(keys)))


if __name__ == "__main__":
    main()
