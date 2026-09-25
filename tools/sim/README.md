# Balance simulation

Headless tooling for measuring how hard each level is: generate many worlds, average what gets
placed in each level, then pit bot parties against each monster type at each character level.

## Build

    ./run.sh --headless --build        # or: cmake --preset headless && cmake --build --preset headless

The scripts use `build/headless/src/lanarts`; set `LANARTS=/path/to/lanarts` to use another
binary and `RT=/path/to/runtime` to run against another runtime directory (for instance a copy
holding the old content, to get a "before" run).

## Engine entry points

- `engine.StartMapGenCheck --seed N --players P` generates the whole campaign and prints one
  `RESULT` line per level (enemies, items, doors and a per-name tally). Gate guards placed by
  Underdungeon/Crypt vaults are attributed to the dungeon they guard (`Underdungeon|GATE:...`).
- `engine.StartCombatSim --jobs FILE --steps N` runs fights in the debug arena. Each job line is
  `class level players seed monster[:count][;monster[:count]...]`; each fight prints a `SIM` line
  with the party damage taken, and the run ends with `SIM DONE`.

## Scripts

| script | does |
| --- | --- |
| `mapgen.sh outdir seed...` | runs StartMapGenCheck for each seed (`PLIST` player counts, `JOBS` parallelism) |
| `comp.py outdir` | mean enemy composition per level and estimated arrival character level → `outdir/comp.json` |
| `runsim.py out.txt comp.json [monster...]` | combat sims for every monster in comp.json (`CLASSES`, `LEVELS`, `PLAYERS`, `JOBS`) |
| `table.py sim.txt Fighter 1` | party damage per monster per character level |
| `metric.py comp.json sim.txt` | per-level difficulty: Σ count × damage / (players × max HP) at arrival level |
| `compare.py a/comp.json a.txt b/comp.json b.txt` | before/after difficulty and step ratios along the routes |
| `cycle.sh tag` | the full loop: 40 seeds × 1/3/4 players, then Fighter sims → `out/` |

A difficulty of 1.0 means the level's monsters, fought one type at a time, deal the party's
whole health pool. It is a relative measure: use it to spot spikes (step ratios well above
~1.5 on the mandatory route) rather than as an absolute threshold.
