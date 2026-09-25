# Animated class sprites

Players get per-state animations (idle, walk, attack, cast, hurt, death) built from the game's own
32x32 class sprites. Each sprite is cut into layers (weapon, upper body, legs) and those layers are
moved, rotated and recombined; new pixels (swing trails, magic, flashes) are generated in the
sprite's style.

    python3 tools/sprites/build.py                     # regenerate everything under runtime/
    python3 tools/sprites/build.py --no-write --only dwarf --sheet /tmp/dwarf.png   # review one rig

Needs Pillow. The outputs are committed, so building the game doesn't need Python packages.

## Layout

| file | what it holds |
| --- | --- |
| `rig.py` | pixel machinery: layers, cut/paste, hand-placed pixels, RotSprite-style rotation, effects |
| `animations.py` | the animations every rig gets; attack and cast are driven by each rig's weapon data |
| `rigs/<name>.py` | one per sprite: traced layer regions, weapon grip, swing angles, tweaks |
| `rigs/__init__.py` | `RIGS`: class sprite name → rig module |
| `build.py` | writes `runtime/spr_animations/<rig>/<animation>.32x32.png` and `runtime/classes/AnimationSets.lua` |

## In the game

- `compile_images.py` registers every `spr_animations/<rig>/` strip as `spr_animations.<rig>.<animation>`,
  frame-indexed (speed 1).
- `AnimationSets.lua` calls `Data.animation_set_create` for each class sprite: the clip for each state,
  each frame's duration in game steps, and whether it loops.
- `PlayerInst::draw` picks the clip: death while it plays (then the usual ghost), else the most recent
  attack or cast, else a hurt flinch, else walk or idle. The timing marks live in
  `CombatGameInst::anim_marks`, outside the serialized state, so animations never affect gameplay.
- Sprites without an animation set draw exactly as before.

## Adding a sprite

1. Dump its pixels to trace regions: `python3 tools/sprites/dump.py runtime/<sprite>.png` prints a
   symbol per colour with a column ruler.
2. Write `rigs/<name>.py` (copy the closest existing rig). Regions are `{row: (first col, last col)}`,
   or a list of spans for a row. `ATTACK` angles are degrees clockwise around `GRIP`.
3. Add it to `RIGS`, review with `--sheet`, then run `build.py` and `compile_images.py`.

## Weapon styles

A rig's `WEAPON_KIND` picks its attack and cast:

| kind | `WEAPON` region is | attack | cast |
| --- | --- | --- | --- |
| `blade` (default) | the weapon and fist | wind-up, swing, strike along `ATTACK` poses, with a trail | weapon raised, glow at its tip |
| `caster` | the free hand | lean back, palm thrust with a spark | hand lifts, glow builds and bursts |
| `bow` | the bow and hand | arrow nocked, loosed as a streak | bow raised, glow at its tip |

`MAGIC` sets the glow colours: `FIRE`, `FROST`, `LIGHTNING`, `DEATH` and `GOLD` are defined in
`animations.py`. Sprites that share a layout share a base module (`rigs/_mage_elf.py`,
`rigs/_mage_hat.py`) and only set `SOURCE` and `MAGIC`. Robes with no visible feet (empty leg
regions) glide and bob instead of stepping.
