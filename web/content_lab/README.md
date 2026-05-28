# Lanarts Content Lab

The Emscripten build uses `web/content_lab/shell.html` as its shell. Build it with:

```sh
./run.sh --emscripten --build
```

Then serve `build/src/lanarts.html` with a local static server from the repository root or build directory. The page exposes class, level, loadout, map generator, bot skill, and seed as URL parameters, then launches the same Lua entry point used by desktop play.

The shell reads its runtime data from the exported content APIs:

- `lanarts_content_catalog_json()` returns classes, class control metadata, bot skill profiles, level labels, loadout presets, build archetypes, micro drills, map generators, map algorithms, encounters, and saved resource presets.
- `lanarts_content_launch_args_json(config)` normalizes a selected run and returns the `engine.StartLanarts` arguments.
- `lanarts_content_loadout_options_json(config)` resolves class build metadata, known spells, preset items, class-relevant curated item choices, build archetypes, and a loadout-readiness read for the selected fight.
- `lanarts_content_build_matrix_json(config)` expands the selected class into curated build archetypes with resolved loadouts, readiness scores, launch args, and balance args.
- `lanarts_content_micro_drills_json(config)` returns launchable rescue, orb/AOE, add-cleanup, and stacked-package practice drills with isolated package components, readiness, pressure previews, scorecards, micro plans, launch args, and balance args.
- `lanarts_content_map_generation_json(config)` returns selectable map algorithms, deterministic preview geometry with enemy pins, map threat profiles, and resolved run/balance args for each preview.
- `lanarts_content_map_preview_json(config)` is kept as a compatible alias for the preview grid payload.
- `lanarts_content_map_matrix_json(config)` expands selected-seed, overworld-algorithm, and late-map probes for map-generation balance runs, including preview-derived threat profiles for quick comparison.
- `lanarts_content_enemy_options_json(config)` returns curated enemy groups, packs, selected monster lists, and `StartMonsterEvals` args.
- `lanarts_content_enemy_matrix_json(config)` expands single-enemy and enemy-pack probes for arena play, balance runs, and monster-eval sweeps.
- `lanarts_content_analysis_plan_json(config)` returns the selected launch, balance, class-sweep, monster-eval, enemy, and map-generation plan.
- `lanarts_content_run_summary_json(config)` resolves a selection into launch args, loadout items, loadout readiness, class/enemy matchup read, class control model, bot skill profile, pressure-scaled encounter rhythms, predicted pressure density/diagnosis, and balance notes.
- `lanarts_content_fight_setup_json(config)` resolves the selected fight objective, class posture, loadout readiness, class/enemy matchup read, package timing windows, predicted pressure density/diagnosis, per-class micro plan, and telemetry fields worth watching for that setup.
- `lanarts_content_package_matrix_json(config)` expands package baselines plus selected-package class, pressure, skill-buffer, and rescue sweeps with rhythm summaries.
- `lanarts_content_resource_matrix_json(config)` expands the current selection into class, pressure, loadout, build-archetype, micro-drill, bot-skill, caster-matchup, map, enemy, and encounter sweeps for quick orthogonal comparisons.
- `lanarts_content_workspace_json(config)` composes the current selection into one UI payload: loadout options, run summary, fight setup, enemy options, map previews, map matrix, analysis plan, package matrix, generated resources, bot profiles, and control model.
- `lanarts_content_presentation_json(config)` returns presentation-ready controls, command previews, map preview cards, matchup read, telemetry rows, threat windows, and grouped resource cards for the web lab.

The shell renders a content board from `lanarts_content_presentation_json(config)` so the selected objective, commands, telemetry, threat windows, pressure density preview, class-specific micro plan, map-preview threat cards, enemy probes, package probes, and orthogonal resources are visible together.
Build templates apply concrete loadout strings from the API, then immediately recompute the selected fight readiness and commands.
Drill cards are concrete launch configurations for practicing specific package reads such as perfect rescue blocks, rescue under orb pressure, orb/AOE footwork, add cleanup, and full stacked package reads. Drill scorecards name the balance metrics that prove whether the practice target is healthy or being cheesed.
The shell also exposes a Drill selector and `drill` URL parameter; choosing one applies the API's full drill config before recomputing commands, scorecards, and preview resources.
Run and fight payloads include matchup reads for red/fire, white, and black resistance checks so Pyrocaster, Stormcaller, and Necromancer comparisons can be selected before running sweeps.
Encounter launches use `--encounter` for curated 1v1/package tests, or `--monsters` for a comma-separated custom arena.
Direct map launches use `--start_map_desc`, and overworld profile selections add `--map_profile` so the same seed can compare profiles such as `open_meadow`, `broken_grove`, `river_road`, `predator_thicket`, and `ruin_crossroads`.
Packaged encounters can also expose rhythm metadata for the shell's timeline view, so orbit, AOE, and summon loops are visible before launch.
Rescue packages may spawn allied monsters on the player team, giving pull, taunt, and body-block tests a fragile target without requiring extra local controllers.
Some rescue packages also add a timed rescue line; the ally takes the heavy hit unless a player intercepts the line during the windup.
The in-game package HUD counts saves and marks blocks that start inside the final rescue window as clutch saves, with a tighter perfect-save band for few-frame body-blocks. Live rescue cues now call out that perfect band before the strike resolves.
Package HUDs show primary target HP so threat timing can be read against actual clear progress.
Package HUDs also show current aggro, with a world link from the primary enemy to its target so taunt/pull state is readable during rescue windows.
Package HUDs include a NEXT row that summarizes the active or nearest package beat, keeping AOE, orbs, summons, and rescue-line timing readable during heavy micro.
Package HUDs also include a STACK row, and balance rows expose peak stack plus overlap rate so full-package fights can be checked for actual simultaneous micro pressure.
Rescue HUDs include TAUNT uptime, and balance rows expose pull/taunt total frames plus uptime rate so rescue-package clears can be separated from forced-target-control cheese.
Balance rows also expose perfect-save counts/rates to distinguish ordinary clutch blocks from the last-few-frame saves that make rescue packages feel sharp.
Summon waves use short visible add portals before enemies appear, so add pressure has a readable cleanup beat instead of an instant spawn.
Orb and AOE rows switch from spawn countdowns to active/warning state while those package threats are live.
Rescue packages also show a live READ row for cut-lane, clutch, hold, and pull-target states while the line is active.
When a package clears or fails, the game posts a compact result banner with elapsed time and rescue stats.
`--package_intensity` scales package tempo, damage, and summon counts from the same encounter definition, which keeps pressure sweeps comparable.
`--package_components` can isolate package parts such as `orbit`, `pulse`, `summon`, or `escort,rescue`; the lab uses this for component probes without replacing full-package baselines.
`--bot_profile` selects the automated-player skill buffer (`perfect`, `skilled`, `human`, or `sloppy`) for balance runs. The lab defaults to `human`, while skill-buffer sweeps make it clear when a package only fails under lower execution or only wins under perfect automation.
