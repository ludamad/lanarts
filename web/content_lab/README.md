# Lanarts Content Lab

The Emscripten build uses `web/content_lab/shell.html` as its shell. Build it with:

```sh
./run.sh --emscripten --build
```

Then serve `build/src/lanarts.html` with a local static server from the repository root or build directory. The page exposes class, level, loadout, map generator, and seed as URL parameters, then launches the same Lua entry point used by desktop play.

The shell reads its runtime data from the exported content APIs:

- `lanarts_content_catalog_json()` returns classes, level labels, loadout presets, map generators, encounters, and saved resource presets.
- `lanarts_content_launch_args_json(config)` normalizes a selected run and returns the `engine.StartLanarts` arguments.
- `lanarts_content_map_preview_json(config)` returns generator metadata for the preview grid.
- `lanarts_content_run_summary_json(config)` resolves a selection into launch args, loadout items, pressure-scaled encounter rhythms, and balance notes.
- `lanarts_content_resource_matrix_json(config)` expands the current selection into class, pressure, map, and encounter sweeps for quick orthogonal comparisons.

Encounter launches use `--encounter` for curated 1v1/package tests, or `--monsters` for a comma-separated custom arena.
Packaged encounters can also expose rhythm metadata for the shell's timeline view, so orbit, AOE, and summon loops are visible before launch.
Rescue packages may spawn allied monsters on the player team, giving pull, taunt, and body-block tests a fragile target without requiring extra local controllers.
`--package_intensity` scales package tempo, damage, and summon counts from the same encounter definition, which keeps pressure sweeps comparable.
