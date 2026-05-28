# Claude Continuation Prompt: Lanarts Boss Micro, Content Lab, and Balance

Copy this prompt into Claude when continuing the current Lanarts workstream.

## Prompt

You are working in the Lanarts repository at:

```text
/Users/adamdomurad/sources/lanarts
```

Continue the current branch as a senior pragmatic game/engine engineer. Read the codebase before editing, prefer existing patterns, keep changes scoped, and commit small coherent slices. The user wants a series of commits that make Lanarts more appealing and playable inside the existing game template, with better boss pressure, better high-skill micro, better content-lab tooling, and more rigorous balance modeling.

The user is not asking for a different game. Preserve the Lanarts feel and improve the current actions, encounters, procedural effects, and balance infrastructure.

## Current Branch Context

The recent branch has been focused on boss polish, automated balance tests, the content lab, multiplayer rescue/pull timing, package telemetry, and the Dicebinder test character.

Recent commits at the time this prompt was written:

```text
f78a695f Add Claude continuation prompt
c32654df Expand Crypt Keeper package pressure
f882b511 Polish Dicebinder roll balance
81e5e153 Track rescue cut commitment
2a4a4a2d Polish Dicebinder roll pacing
43e03aec Track package beat variety
5f373f94 Clamp procedural FX to map bounds
eae3ccb5 Add package sync beat feedback
fd6054d7 Expose rescue timing margins
215fabf9 Polish Dicebinder roll animation
afd59f20 Add Nesso arrow-storm package
```

Start by checking:

```sh
git status --short
git log --oneline -10
```

Do not revert unrelated or user-authored changes. If the working tree is dirty, inspect the changed files and work with them unless they block the task.

Update note from the handoff pass:

- The prompt file itself was added in `f78a695f`.
- A later Codex pass began inspecting rescue snap timing, HUD rows, taunt strain, and telemetry, but the user stopped that implementation pass before any gameplay files were edited.
- As of that stop, no new rescue snap telemetry or HUD changes had been committed after `f78a695f`.
- The existing rescue system already has late/perfect saves, pull snap bursts, danger-window coverage, pull discipline, taunt strain, and counter-window telemetry. The remaining gap is making "snap opportunity now" a first-class active state before the save resolves.
- Before implementing anything, re-check `git status --short` and the latest `git log --oneline -10`; this prompt may be older than the current branch head.

Session update (Claude, 2026-05-28) — three committed slices on top of `d83f9314`:

- `b6930556` Clarify rescue snap timing and expose anti-cheese telemetry. The snap (precision pull) is now a first-class active rescue state, suppressed when an early camper has pre-solved the line. Added a SNAP HUD row + closing snap-aperture draw, `package_rescue_active_snap_*` live fields, `snap_window_frames` / `snap_ready_frames` / `snap_ready_rate`, a `broad_pull_index` (pull uptime weighted by non-timed pulls), and a `broad-pull` scorecard check. Taunt was not strengthened; camping still resolves as a generic save.
- `88718649` Give FURY/DREAD punish a non-rescue break path. Clean dodge-streak milestones (MICRO x3/x5+) now claw back un-broken `micro_punish` healing, capped at the heal earned, so duels and other non-rescue packages keep a skill-gated counterplay. `streak_break` / `streak_break_label` are catalog-tunable (Gragh 7, Crypt Keeper 4).
- `0923cece` Flag instrument-limited and illegible package losses. New `package_difficulty` non-lethal-stall warning (win <=5%, death <=5%, >=75% step-limit) and `package_micro` legibility warning (micro <=-20 with >=5 avoidable hits/run), operationalizing "if everyone loses because warnings are unreadable, improve readability before tuning numbers."

Highest-signal balance finding (do not lose this): a fixed-seed `gragh-duel` level-5 sweep clears 0% for Fighter/Pyrocaster/Stormcaller and ~33% for Necromancer, but across human AND skilled AND perfect bot profiles every class sits at step_limit with -20 to -71 micro, 6-8 avoidable hits/run, best dodge streak ~3, and 56-85 HP of FURY healing. This is instrument-limited: the scripted bots cannot dodge Gragh's overlapping orbit+pulse+sweep (pressure dominance ~0.85, overlap ~0.8) well enough to build streaks or out-damage the heal in the step budget. Do NOT nerf Gragh damage off this. The real next investigations are (a) Gragh mechanic readability / spacing of overlapping beats, and (b) whether the automated dodge/pathing logic is a usable measurement instrument for dense late packages. Validate readability changes with draw smokes, not bot win rate.

Possible next slices: reduce gragh-duel pressure dominance/overlap for legibility (variety, not damage); add a non-Gragh full package (Workstream 2); or improve the bot's hazard-avoidance so late packages become measurable.

## User Goal

The broad goal is to make late-game Lanarts content more fun, legible, difficult in the right ways, and measurable.

The key design direction:

- Level 5 player characters are late game.
- Level 8 is very high and should not be the default balance target.
- Late-game content should not be beaten reliably by a primitive bot unless the bot is actually demonstrating strong micro.
- If a dumb or "human" automated player clears a boss at a high rate with poor micro scores, the enemy package is probably too easy or the player class/tooling is too strong.
- Fighter and Necromancer can be close-range classes.
- Pyrocaster, Stormcaller, and other ranged classes should prefer distance and should not be balanced around face-tanking.
- Necromancer damage taken is misleading because regeneration can convert raw incoming damage into a false sense of danger. Use regen-adjusted reads.
- The user is especially worried about enemy balance, not only player class power.
- The boss fights should be juicy, readable, and exciting, with procedural animation and clear pressure beats.
- Multiplayer rescue/pull moments should create few-frame clutch saves without making taunt a dominant cheese strategy.
- The content lab should let the user choose class, level, loadout, map generation, encounter, package intensity, package components, and bot profile, then preview or test those choices.

## Existing State To Preserve

The repo now has a content-lab workflow and automated native balance loop. Preserve and extend that workflow.

Important existing capabilities:

- A web content lab shell under `web/content_lab/`.
- Content API functions exposed through `runtime/engine/ContentApi.lua`.
- Curated content catalog under `runtime/engine/ContentCatalog.lua`.
- Native content-lab runner scripts under `e2e_tests/`.
- Automated player skill buffers such as `perfect`, `skilled`, `human`, and `sloppy`.
- Package encounters with orbit, pulse/AOE, sweep, summon, escort, rescue, sync, beat variety, micro scoring, taunt strain, and micro punish telemetry.
- Full-package 1v1 or boss-package encounters for enemies such as Gragh, Executioner, Hell Warrior, Mana Sapper, Stara, Pixulloch, Waxanarian, Nesso, and Crypt Keeper.
- Dicebinder as a test character with skill dice that roll over time and grant small bonus/cooldown windows.

Do not remove telemetry just because it is noisy. Prefer tightening the interpretation and scorecards.

## Hard Operational Rules

Never start the game or tests with audio enabled. Use these environment variables for local runs:

```sh
SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1
```

For native iteration when the binaries are already built, also use:

```sh
LANARTS_SKIP_BUILD=1
```

Prefer native compiles/tests while iterating. Emscripten is slow and should be used only for final web verification when needed.

Always run:

```sh
git diff --check
```

before committing.

Do not leave dev servers or long-running game processes running after your turn.

## Important Files

Use this map before editing:

- `runtime/engine/ContentCatalog.lua`
  - Class specs, loadout presets, resource definitions, encounter definitions, map previews, package configs, analysis plans, and catalog-facing metadata.
- `runtime/objects/ContentLabHazards.lua`
  - Package encounter runtime, procedural boss effects, rescue line logic, pull/taunt logic, pressure HUD, beat wheel, micro scoring, micro punish, active hazard draws.
- `runtime/engine/StartBalanceRun.lua`
  - Automated balance runner, telemetry collection, report rows, scorecard output, CSV/JSON fields.
- `runtime/engine/ContentApi.lua`
  - Web/API payloads, scorecards, readiness summaries, fight setup summaries, content workspace output.
- `web/content_lab/shell.html`
  - Web-based interface and presentation layer. Keep it clean, dense, and usable. No marketing landing page.
- `web/content_lab/README.md`
  - User-facing content lab notes and capability summary.
- `e2e_tests/README.md`
  - Test and balance runner docs.
- `e2e_tests/test_content_api_smoke.sh`
  - API/catalog/UI smoke for content lab changes.
- `e2e_tests/run_content_lab_native.sh`
  - Main native content run wrapper.
- `e2e_tests/test_content_lab_native_fast.sh`
  - Fast native smoke for shared content-lab behavior.
- `e2e_tests/test_content_package_balance_smoke.sh`
  - Package balance smoke.
- `runtime/spells/Effects.moon`
  - MoonScript source for some effects.
- `runtime/compiled_moon/spells/Effects.lua`
  - Compiled copy. If you change the MoonScript, update the compiled Lua.

## Current Implemented Design Notes

Package fights should be measured as complete pressure packages, not just enemy DPS. Healthy late-game package pressure usually has:

- A readable primary enemy.
- A larger arena, usually around 30-34 tiles for level-5 package tests.
- Orbiting pressure that demands pathing.
- Pulse/AOE pressure with warnings and active windows.
- Sweep or lane pressure to create routing commitments.
- Summon/add pressure to create cleanup decisions.
- Optional escort/rescue pressure for multiplayer save moments.
- Beat variety, sync moments, and overlapping pressure windows.
- In-world and HUD cues that show what is about to happen.
- Telemetry that separates brute-force clears from clean execution.

Recent examples:

- Dicebinder roll anticipation was extended to 84 frames with richer badge animation, fair six-face bag logic, repeat protection, high/dry streak guards, scorecards, and telemetry.
- Crypt Keeper was upgraded into a fuller black package with a 32x32 room, crossing bone-scythe lanes, DREAD micro-punish healing, reduced orb dominance, and package-specific punish labels.
- Rescue telemetry includes save rates, clutch/perfect saves, early saves, timing score, danger-window coverage, pull timing, pull discipline, taunt strain, snap bursts, counter windows, and fatal/survived misses.
- Package pressure telemetry includes max active count, overlap rate, sync rate, sync bursts, unique components, signature switches, dominant component share, micro score, best streak, avoidable hits, threaded dodges, and micro-punish totals.

## Balance Interpretation Rules

Use these rules when deciding whether content is healthier:

- A high clear rate is not automatically bad if the bot shows good micro, positive package micro score, clutch/perfect saves, strong dodge streaks, and low avoidable hits.
- A high clear rate with negative micro score, high avoidable hits, or high broad taunt uptime is bad. It means brute force or cheese is winning.
- Fighter clearing content sometimes is fine. Fighter clearing late packages at 100 percent with a primitive bot is suspicious.
- Stormcaller may be overperforming if it can kite or burst while ignoring package demands.
- Pyrocaster being weaker than Stormcaller may be real class imbalance, not necessarily enemy balance.
- Necromancer survival must be judged with regeneration context, not raw damage taken.
- For ranged classes, track whether the bot is maintaining distance. Forced close-range frames should be treated as pressure only when they are intentional and readable, not pathing failure.
- For rescue packages, distinguish timed clutch pulls from permanent taunt uptime. The user wants few-frame save excitement, not taunt cheese.
- For package fights, avoid tuning only by damage. Rhythm, readability, overlap, commitment windows, and counterplay matter more.

## Preferred Workflow

For each slice:

1. Inspect the relevant files and current telemetry fields.
2. Make a small design change that improves either gameplay, readability, telemetry, or a concrete encounter.
3. Update the content lab API/UI/docs if the new feature or metric should be visible.
4. Run the smallest native smoke or targeted balance batch that proves the slice.
5. Run `git diff --check`.
6. Commit with a specific message.

Use `rg` first for searches. For example:

```sh
rg -n "rescue|pull|taunt|snap|package_rescue" runtime/objects/ContentLabHazards.lua runtime/engine/StartBalanceRun.lua runtime/engine/ContentApi.lua web/content_lab/shell.html
rg -n "micro_punish|package_micro|DREAD|FURY" runtime/objects/ContentLabHazards.lua runtime/engine/ContentCatalog.lua runtime/engine/StartBalanceRun.lua
rg -n "dice_roll|Dicebinder|roll_face|skill dice" runtime/engine runtime/objects runtime/spells web/content_lab e2e_tests
```

## Recommended Next Workstream 1: Rescue Snap UX And Anti-Cheese

This is likely the best next slice. The code already tracks many rescue and pull metrics, but the user goal needs the actual play moment to feel sharper.

Inspect these areas in `runtime/objects/ContentLabHazards.lua`:

- Rescue strike creation and state updates.
- Pull tether creation and draw.
- Rescue window state.
- Package HUD draw.
- Taunt/pull strain logic.
- Any functions or local helpers named around:
  - `create_rescue_strike`
  - `create_pull_tether`
  - `rescue_window_state`
  - `rescue_commit_hint`
  - `draw_package_hud`

Design target:

- The player should understand when a rescue line is open, when it is dangerous, when a clutch save is possible, and when a perfect snap pull is possible.
- A late pull should feel visibly different from broad taunt uptime.
- The bot and reports should expose whether saves came from precision or cheese.

Possible implementation:

- Add a `SNAP` row or stronger `READ` state in the package HUD.
- Expose active snap readiness as telemetry:
  - `package_rescue_active_snap_ready`
  - `package_rescue_active_snap_distance`
  - `package_rescue_pull_snap_window_frames`
  - `package_rescue_pull_snap_ready_frames`
- Draw small snap-aperture ticks on the rescue line during clutch/perfect windows.
- Make perfect snap burst visuals brighter and shorter than generic pull tether visuals.
- Add report rows or scorecard checks that flag high broad pull uptime with low clutch/perfect pull rate.
- Keep taunt useful but strained. Do not make permanent taunt the optimal solution.

Verification commands:

```sh
env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/test_content_api_smoke.sh

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --class Rogue --level 5 --loadout_preset late --encounter executioner-rescue-package --package_intensity high --package_components escort,orbit,pulse,sweep,rescue --bot_profile human --runs 3 --steps 700 --seed 33000 --label rescue_snap --no-build

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --draw --class Rogue --level 5 --loadout_preset late --encounter executioner-rescue-package --package_intensity high --package_components escort,orbit,pulse,sweep,rescue --bot_profile human --runs 1 --steps 360 --draw_interval 20 --seed 33000 --label rescue_snap_draw --no-build
```

Commit message example:

```text
Clarify rescue snap timing
```

## Recommended Next Workstream 2: More Full-Package 1v1 Bosses

The content lab should keep growing horizontally through varied full-package encounters. Do not only improve Gragh. Add or upgrade one enemy package at a time.

Good candidates:

- An underdeveloped late enemy that currently acts like a stat block.
- A package that has only one or two pressure types and lacks rhythm variety.
- An enemy with a distinct fantasy that can imply a unique package shape.

Package design checklist:

- Arena size around 30-34 tiles for level-5 late-game package testing.
- Primary enemy pressure remains readable.
- At least four orthogonal components from:
  - orbit
  - pulse/AOE
  - sweep/lane
  - summon/add
  - rescue/escort
  - micro punish
- The pressure should create choices, not just random damage.
- Warnings should be visible before active damage.
- The package should have a unique color/read shape so it does not feel like a reskin.
- Add scorecards and docs so the lab understands it.

Files likely touched:

- `runtime/engine/ContentCatalog.lua`
- `runtime/objects/ContentLabHazards.lua`
- `runtime/engine/ContentApi.lua`
- `web/content_lab/shell.html`
- `web/content_lab/README.md`
- `e2e_tests/README.md`
- smoke scripts if they grep expected encounter names or telemetry fields

Verification pattern:

```sh
env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/test_content_api_smoke.sh

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --class Stormcaller --level 5 --loadout_preset late --encounter <encounter-id> --package_intensity high --bot_profile human --runs 3 --steps 900 --seed 42000 --label <label> --no-build

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --draw --class Stormcaller --level 5 --loadout_preset late --encounter <encounter-id> --package_intensity high --bot_profile human --runs 1 --steps 360 --draw_interval 20 --seed 42000 --label <label>_draw --no-build
```

Commit message example:

```text
Add <enemy> full package pressure
```

## Recommended Next Workstream 3: Current-Action UX

The user explicitly wants the current game actions to feel better, not a replacement combat model.

Look for small upgrades that improve decision readability:

- Ability readiness cues for key class actions.
- In-world safe re-entry hints after a boss package beat.
- Ranged keep-away readability.
- Melee re-entry timing for Fighter and Necromancer.
- Better feedback when a clean dodge opens a counter window.
- More distinctive package pressure fields around bosses.

Constraints:

- Use existing HUD/procedural FX patterns.
- Avoid introducing a new overlay framework.
- Do not add long tutorial text.
- Keep visual cues compact and legible during combat.

Possible implementation:

- Add a short "re-entry glint" after sweep/orbit hazard expiration if the player is close enough to punish.
- Add a small class-posture cue in the HUD or near player:
  - close for Fighter/Necromancer
  - range for Pyrocaster/Stormcaller
- Feed this into telemetry as distance discipline rather than only visual state.

Verification:

- Draw smoke for the changed encounter/class.
- Native run for at least one melee and one ranged class.
- API smoke if new telemetry or catalog-facing fields are added.

## Recommended Next Workstream 4: Enemy And Class Balance Sweeps

The balance model should answer: "Is the enemy package healthy, or is one class/tool trivializing it?"

Run fixed-seed class sweeps on the same encounter. Compare:

- clear rate
- death/step-limit rate
- package micro score
- avoidable hit count
- micro-punish total and breaks
- pressure dominance rate
- ranged close frames
- Necromancer regen frames
- rescue save timing
- broad pull/taunt uptime
- clutch/perfect pull rates

Example commands:

```sh
env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --class Fighter --level 5 --loadout_preset late --encounter gragh-duel --package_intensity high --bot_profile human --runs 6 --steps 900 --seed 41000 --label fighter_gragh --no-build

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --class Necromancer --level 5 --loadout_preset late --encounter gragh-duel --package_intensity high --bot_profile human --runs 6 --steps 900 --seed 41000 --label necro_gragh --no-build

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --class Pyrocaster --level 5 --loadout_preset late --encounter gragh-duel --package_intensity high --bot_profile human --runs 6 --steps 900 --seed 41000 --label pyro_gragh --no-build

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --class Stormcaller --level 5 --loadout_preset late --encounter gragh-duel --package_intensity high --bot_profile human --runs 6 --steps 900 --seed 41000 --label storm_gragh --no-build
```

Interpretation:

- If Fighter wins 100 percent with poor micro, the package is too easy or Fighter abilities are overtuned.
- If Stormcaller wins while Pyrocaster fails, inspect mobility, range, burst uptime, cooldown pressure, and whether Pyrocaster is being forced into unsafe range.
- If Necromancer appears healthy only because regen masks damage, add regen-adjusted scorecard rows instead of blindly buffing enemies.
- If everyone wins with good micro and the package still looks exciting, that can be acceptable.
- If everyone loses because warnings are unreadable, improve readability before tuning numbers.

## Recommended Next Workstream 5: Content Lab Presentation And APIs

The user wants to use the tool directly and see clean content. The lab should feel like a practical game-design workstation.

Priorities:

- Keep the first screen functional, not a landing page.
- Surface class, level, loadout, map generation, encounter, package intensity, package components, and bot profile.
- Show launch args and balance args.
- Show current fight setup, pressure preview, class micro plan, and scorecards.
- Let the user preview map generation variants and enemy pins.
- Keep in-page text concise and operational.
- Avoid decorative-heavy layouts that hide the content.

If you add or rename API fields:

- Update `runtime/engine/ContentApi.lua`.
- Update `web/content_lab/shell.html`.
- Update docs in `web/content_lab/README.md` and `e2e_tests/README.md`.
- Run API smoke.

Watch for common browser/API hazards:

- `gaps` or similar fields may be either string, table/array, or missing. Normalize before rendering.
- Emscripten FS initialization can fail if directories are blindly created twice. Make init idempotent.
- Long-running runtime state can carry across UI restarts. Guard setup paths.

## Testing Gates

Use the smallest test that covers your slice, then broaden when touching shared code.

Always:

```sh
git diff --check
```

Docs-only changes:

```sh
git status --short
git diff --check
```

API/catalog/UI changes:

```sh
env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/test_content_api_smoke.sh
```

Shared package engine, telemetry, rescue, bot, or scorecard changes:

```sh
env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/test_content_lab_native_fast.sh
```

Package schema or broad package balance changes:

```sh
env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/test_content_package_balance_smoke.sh
```

Visual or HUD changes:

```sh
env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --draw --class <Class> --level 5 --loadout_preset late --encounter <encounter-id> --package_intensity high --bot_profile human --runs 1 --steps 360 --draw_interval 20 --seed <seed> --label <label>_draw --no-build
```

If a command fails because native binaries are stale, rebuild natively rather than switching immediately to Emscripten. Keep audio muted.

## Do Not Do

- Do not tune normal late game around level 8.
- Do not start the game with sound.
- Do not use Emscripten as the main iteration loop.
- Do not claim content is balanced just because the bot clears it.
- Do not treat Necromancer raw damage taken as a clean danger metric.
- Do not make taunt/pull broadly dominant.
- Do not add unrelated art pipelines, frameworks, or a new combat system.
- Do not replace the content lab with a marketing page.
- Do not bury key mechanics in prose. Prefer in-world cues, compact HUD rows, and telemetry.
- Do not commit without running at least `git diff --check`.

## Concrete First Task For Claude

If no newer instruction is given, implement this slice:

```text
Clarify rescue snap timing and expose anti-cheese telemetry.
```

Acceptance criteria:

- Active rescue state exposes whether a snap pull is currently ready.
- Active rescue state exposes the approximate distance or margin remaining for the snap.
- The in-game HUD or draw layer differentiates:
  - generic rescue line
  - late/clutch save window
  - perfect few-frame save window
  - snap pull opportunity
- Telemetry distinguishes precision pull saves from broad pull/taunt uptime.
- Scorecards or report rows flag high taunt reliance when clutch/perfect pull rates are low.
- The change does not make taunt stronger as a generic solution.
- API smoke passes.
- A targeted native rescue run completes and emits the new fields.
- A draw smoke run shows the snap cue without audio.
- `git diff --check` passes.
- Commit the slice.

Suggested verification after implementing:

```sh
env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/test_content_api_smoke.sh

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --class Rogue --level 5 --loadout_preset late --encounter executioner-rescue-package --package_intensity high --package_components escort,orbit,pulse,sweep,rescue --bot_profile human --runs 3 --steps 700 --seed 33000 --label rescue_snap --no-build

env LANARTS_SKIP_BUILD=1 SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 ./e2e_tests/run_content_lab_native.sh --draw --class Rogue --level 5 --loadout_preset late --encounter executioner-rescue-package --package_intensity high --package_components escort,orbit,pulse,sweep,rescue --bot_profile human --runs 1 --steps 360 --draw_interval 20 --seed 33000 --label rescue_snap_draw --no-build

git diff --check
```

## Final Response Style

When you finish a slice, report:

- the commit hash and message
- the main files changed
- the tests or runs performed
- the highest-signal balance observations
- any remaining risk or next best slice

Keep the user-facing summary short and concrete.
