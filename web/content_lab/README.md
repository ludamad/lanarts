# Lanarts Content Lab

The Emscripten build uses `web/content_lab/shell.html` as its shell. Build it with:

```sh
./run.sh --emscripten --build
```

Then serve `build/src/lanarts.html` with a local static server from the repository root or build directory. The page exposes class, level, loadout, map generator, bot skill, and seed as URL parameters, then launches the same Lua entry point used by desktop play.

For fast boss/package iteration, use the native runner before rebuilding Emscripten:

```sh
LANARTS_SKIP_BUILD=1 ./e2e_tests/run_content_lab_native.sh --class Dicebinder --encounter gragh-duel
LANARTS_SKIP_BUILD=1 ./e2e_tests/run_content_lab_native.sh --draw --class Rogue --encounter executioner-rescue-package --package_components escort,orbit,pulse,sweep,rescue
```

The runner accepts the same class, level, loadout, bot profile, encounter, package intensity, package component, seed, and draw options surfaced by the web lab. It mutes audio and writes persistent CSV/JSON outputs under `saves/content_lab_native/`.

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
Packaged encounters can also expose rhythm metadata for the shell's timeline view, so orbit, AOE, sweep, and summon loops are visible before launch.
The catalog includes solo full-package 1v1s such as Mana Sapper ritual, where a support enemy gets orbit, AOE, sweep, and add rhythms instead of relying on a second real enemy bodyguard. Hell Warrior also has a red melee package that caps added Imp portals so its fast-contact balance can be read against visible ember, lane, and brand beats. Stara has a black pain-star package for comparing aura/projectile pressure against visible skull lanes and Skeleton cleanup. Waxanarian adds a green venom package with fast bee pursuit, rotating stinger knots, crossing venom lanes, poison bloom timing, and Poison Bee cleanup.
Level-5 package arenas use larger 30-32 tile rooms so stacked orb, sweep, rescue, and add pressure has room to read without turning the fight into corridor pinball.
Rescue packages may spawn allied monsters on the player team, giving pull, taunt, and body-block tests a fragile target without requiring extra local controllers.
Some rescue packages also add a timed rescue line; the ally takes the heavy hit unless a player intercepts the line during the windup. When the line enters the final save windows, an in-world danger clock shows the remaining late/perfect frames and whether the cut is currently covered.
The in-game package HUD counts saves and marks blocks that start inside the final rescue window as clutch saves, with a tighter perfect-save band for few-frame body-blocks. Live rescue cues now burst on the playfield when clutch and perfect windows open before the strike resolves, and clutch/perfect saves can open a short boss counter window.
Package HUDs show primary target HP so threat timing can be read against actual clear progress.
Package HUDs also show current aggro plus the current target-role uptime, with a world link from the primary enemy to its target so taunt/pull state is readable during rescue windows.
Package HUDs include NEXT and BEAT rows that summarize the active or nearest package beat, keeping AOE, orbs, sweeps, summons, portals, and rescue-line timing readable during heavy micro.
Package HUDs also include a STACK row, and balance rows expose peak stack plus overlap rate so full-package fights can be checked for actual simultaneous micro pressure.
Package HUDs include a compact beat wheel in the title band; its orbit, AOE, sweep, add, and rescue pips fill toward the next rhythm and flare when a beat is warning or active.
Boss package draws now use a reusable procedural pressure-field aura that grows and spins harder when overlapping package beats stack up.
Dicebinder's live D6 badge animates through a 48-frame rolling anticipation window with orbiting die ghosts, staged pip bursts, three lock-in beats, a late reveal cue for high rolls, then the resolved roll role and remaining buff arc so cooldown-refresh windows are readable during boss-package drills. Rolls use a fair six-face bag before refilling, keeping the class varied without letting jackpot streaks dominate balance reads.
Package HUDs include a MICRO row that scores clean hazard dodges, rescue saves, streaks, threaded dodge openings, and avoidable hits while the fight is happening. Near-miss lane reads now throw immediate THREAD bursts; clean threaded dodges briefly open the primary with small damage/stagger, and clean dodge streaks add small MICRO xN bursts so package footwork feels like active execution rather than delayed accounting.
Clean package footwork now draws a small micro-streak halo around the player, while recent avoidable hits flash a short broken-ring state, keeping streak pressure readable without adding new controls.
Rescue HUDs include TAUNT uptime, and balance rows expose pull/taunt total frames, uptime rate, and pull discipline so rescue-package clears can be separated from forced-target-control cheese.
Balance rows also expose primary aggro player/ally/other/none rates, inferred-target rate, and target switches for every package, so forced-target cheese can be audited even outside rescue-only drills.
Balance rows also expose clutch/perfect pull counts, timed-pull rates, pull-discipline rates, clutch/perfect rescue window cues, late/perfect cover rates, uncovered danger-window frames, and clutch/perfect pull snap bursts, so a few-frame snap-pull save can be modeled separately from broad taunt uptime. Clutch/perfect pull tethers now throw a snap burst at both endpoints, add moving tether sparks, and lightly flash the screen edge.
Balance rows also expose perfect-save counts/rates to distinguish ordinary clutch blocks from the last-few-frame saves that make rescue packages feel sharp.
Balance rows expose early-save rate, rescue timing score, rescue counter windows/damage/stagger, and fatal-vs-buffered misses so few-frame saves can be modeled separately from players camping the rescue line before the strike becomes urgent.
Balance rows expose package micro score, best streak, threaded avoid count, threaded opening damage/stagger, and avoidable mechanic-hit count so lab runs can separate clean execution from brute-force clears. Orb and sweep avoids only score when the player was actually close to the hazard lane; expired off-screen hazards no longer inflate dodge counts.
Package reports and drill scorecards flag clears with negative micro score so a primitive automated player surviving the fight does not automatically count as healthy content.
Gragh packages also expose `package_micro_punish_total`: avoidable package hits feed Gragh Fury, healing the primary up to an overheal cap so brute-force melee clears have an in-fight cost. In-game, the package HUD shows a FURY row and Gragh flashes with a short red burst when the punish triggers; clutch/perfect rescue saves can break some Fury back off and emit `package_micro_punish_break_total`.
Dicebinder runs expose skill-dice telemetry such as `dice_rolls_total`, roll-face counts, roll animation frames, buildup FX, lock cues, suspense pips, reveal cues, high-roll streaks, repeat rolls, bag refills, effective Dash/Power refreshes, jackpot rolls, dash-window frames, and buff active frames so RNG windows can be separated from baseline package execution. Dicebinder drill scorecards now watch cadence, animation frames, lock cues, variety switches, repeat caps, and buff uptime directly.
Summon waves use short visible add portals before enemies appear, so add pressure has a readable cleanup beat instead of an instant spawn.
Orb, sweep, and AOE rows switch from spawn countdowns to active/warning state while those package threats are live.
Rescue packages also show a live READ row for cut-lane, clutch, hold, and pull-target states while the line is active.
When a package clears or fails, the game posts a compact result banner with elapsed time and rescue stats.
`--package_intensity` scales package tempo, damage, and summon counts from the same encounter definition, which keeps pressure sweeps comparable.
`--package_components` can isolate package parts such as `orbit`, `pulse`, `sweep`, `summon`, or `escort,rescue`; the lab uses this for component probes without replacing full-package baselines.
`--bot_profile` selects the automated-player skill buffer (`perfect`, `skilled`, `human`, or `sloppy`) for balance runs. The lab defaults to `human`, while skill-buffer sweeps make it clear when a package only fails under lower execution or only wins under perfect automation.
Pixulloch has a larger final-boss web package in the lab with native black summons, rotating web knots, crossing venom lanes, a close web nova, and Giant Spider portals so late 1v1 testing includes a distinct final-boss pressure shape.
