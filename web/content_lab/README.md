# Lanarts Content Lab

The Emscripten build uses `web/content_lab/shell.html` as its shell. Build it with:

```sh
./run.sh --emscripten --build
```

Then serve `build/src/lanarts.html` with a local static server from the repository root or build directory. The page exposes class, level, loadout, map generator, and seed as URL parameters, then launches the same Lua entry point used by desktop play.

The shell reads its runtime data from the exported content APIs:

- `lanarts_content_catalog_json()` returns classes, level labels, loadout presets, map generators, map algorithms, encounters, and saved resource presets.
- `lanarts_content_launch_args_json(config)` normalizes a selected run and returns the `engine.StartLanarts` arguments.
- `lanarts_content_loadout_options_json(config)` resolves class build metadata, known spells, preset items, and class-relevant curated item choices.
- `lanarts_content_map_generation_json(config)` returns selectable map algorithms, preview metadata, and resolved run/balance args for each preview.
- `lanarts_content_map_preview_json(config)` is kept as a compatible alias for the preview grid payload.
- `lanarts_content_enemy_options_json(config)` returns curated enemy groups, packs, selected monster lists, and `StartMonsterEvals` args.
- `lanarts_content_enemy_matrix_json(config)` expands single-enemy and enemy-pack probes for arena play, balance runs, and monster-eval sweeps.
- `lanarts_content_analysis_plan_json(config)` returns the selected launch, balance, class-sweep, monster-eval, enemy, and map-generation plan.
- `lanarts_content_run_summary_json(config)` resolves a selection into launch args, loadout items, pressure-scaled encounter rhythms, and balance notes.
- `lanarts_content_resource_matrix_json(config)` expands the current selection into class, pressure, map, and encounter sweeps for quick orthogonal comparisons.

Encounter launches use `--encounter` for curated 1v1/package tests, or `--monsters` for a comma-separated custom arena.
Direct map launches use `--start_map_desc`, and overworld profile selections add `--map_profile` so the same seed can compare `open_meadow`, `broken_grove`, and `old_stones`.
Packaged encounters can also expose rhythm metadata for the shell's timeline view, so orbit, AOE, and summon loops are visible before launch.
Rescue packages may spawn allied monsters on the player team, giving pull, taunt, and body-block tests a fragile target without requiring extra local controllers.
Some rescue packages also add a timed rescue line; the ally takes the heavy hit unless a player intercepts the line during the windup.
The in-game package HUD counts saves and marks blocks that start inside the final rescue window as clutch saves.
When a package clears or fails, the game posts a compact result banner with elapsed time and rescue stats.
`--package_intensity` scales package tempo, damage, and summon counts from the same encounter definition, which keeps pressure sweeps comparable.
