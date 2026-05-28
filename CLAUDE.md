# CLAUDE.md — Lanarts working notes

Living guide for working in this repo. **Keep it updated as we go** — when you
learn something non-obvious (a build flag, a gotcha, a subsystem map, a fix),
add it here in the same change.

## Working agreements

- **Fix bugs as we go.** If a change surfaces a latent bug (crash, segfault,
  wrong telemetry), fix the root cause rather than working around it or
  reverting and moving on. Reverting is a last resort to keep the suite green
  while the real fix is in progress.
- **Update this file as we go.** New build flags, debugging recipes, subsystem
  notes, and fixed-bug postmortems belong here so the next session starts ahead.
- Commit small, coherent, verified slices. End commit messages with the
  Co-Authored-By trailer.

## Hard operational rules (audio + iteration)

- Never run the game or tests with audio. Always:
  `SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1`
- Native iteration with prebuilt binaries: add `LANARTS_SKIP_BUILD=1` and pass
  `--no-build` to the e2e runners.
- Prefer native compiles/tests while iterating; Emscripten only for final web
  verification.
- Always `git diff --check` before committing. Don't leave dev servers or game
  processes running after a turn.

## Build / run

- Native (debug): `./run.sh --headless --build` builds
  `build_debug_headless/src/lanarts` — the same binary the e2e runners use with
  `LANARTS_SKIP_BUILD=1`. After an engine (C++) change, rebuild before testing.
- Headless balance entrypoint: `./run.sh --headless engine.StartBalanceRun ...`
- MoonScript has precompiled Lua under `runtime/compiled_moon/`. The runtime
  loads the compiled copy, so **edit both** the `.moon` source and its compiled
  `.lua` when changing MoonScript (e.g. `runtime/spells/TypeEffectUtils.moon` +
  `runtime/compiled_moon/spells/TypeEffectUtils.lua`).

## Test gates

- API/catalog/UI: `./e2e_tests/test_content_api_smoke.sh` (greps source + runs
  `engine.StartContentApiSmoke`).
- Shared engine/telemetry/bot: `./e2e_tests/test_content_lab_native_fast.sh`.
- Package schema/balance: `./e2e_tests/test_content_package_balance_smoke.sh`.
- Run the full trio after touching shared code; they exercise real combat.

## Debugging recipes

- Segfault with no Lua traceback ⇒ a C++ crash. Get a backtrace with lldb:
  `(cd runtime && lldb -b -o run -o "bt 14" -- ../build_debug_headless/src/lanarts <args>)`
- For use-after-free / heap corruption, build with AddressSanitizer (see the
  ASan note below) and run the minimal repro.
- Lua `pcall` CANNOT catch C++ `luawrap::Error` (it's a C++ throw). To probe a
  gameinst's type from Lua safely, expose a fallback-safe metatable constant in
  C++ (e.g. `is_projectile`) rather than reading a type-specific getter.

## Subsystem notes

- Automated player: `runtime/ai/AutomatedPlayerInputSource.lua`. Skill profiles
  (perfect/skilled/human/sloppy) scale decision interval, skip chances, and
  hazard-dodge react margin/skip. The bot dodges content-lab package hazards
  (orbit/pulse/sweep), rescue strikes, and native projectiles (via the
  `Map.projectile_data` C++ binding).
- Content lab balance/telemetry: `runtime/engine/StartBalanceRun.lua`,
  `runtime/engine/ContentApi.lua`, `runtime/objects/ContentLabHazards.lua`,
  `runtime/engine/ContentCatalog.lua`.
- Elemental type matchups are intentionally soft: `get_resistance` in
  `TypeEffectUtils` caps the damage modifier at ±10% either way (no hard
  counters).
- Collision/spatial grid: `src/gamestate/GameInstSet.cpp`. `InstanceState`
  nodes live in a `std::vector<InstanceState> unit_set` and grid/depth linked
  lists point INTO it, so a vector reallocation invalidates those pointers and
  they are fixed up in `reallocate_internal_data()`. Suspected
  dangling-pointer bug here under heavy object churn (see KNOWN ISSUES).

## Known issues / in-progress

- **Collision-grid dangling pointer (in progress):** jamming a melee bot into a
  dense object cluster on gragh-duel crashes in
  `GameInstSet::object_radius_test` (GameInstSet.cpp:409) dereferencing a stale
  `inst` from `unit_grid`, via `PlayerInst::_use_move`. Repro:
  `./run.sh --headless engine.StartBalanceRun --steps 700 --runs 1 --classes
  Dicebinder --levels 5 --loadout_preset late --bot_profile human --encounter
  gragh-duel --package_intensity high`. The melee-weave bot change that exposed
  it was reverted to keep the suite green; re-land weaving after this is fixed.
