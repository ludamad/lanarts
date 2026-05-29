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

## Design principles (combat / classes / bosses)

- **Work to the game's strengths.** Lanarts is a top-down real-time roguelike whose
  strength is *fast reactive movement and positioning* + the package/hazard/telegraph
  system + the Flow movement-combo + the content-lab measurement loop. Design with
  these, not against them. Movement IS the dodge — do NOT add Souls-style i-frame
  dodges (they flatten the spatial game).
- **Understand a class before evaluating/changing it.** Read the actual kit; the bot
  only expresses the *movement* layer, so a low skill-depth score can mean shallow
  design OR "the bot doesn't play this class's real loop." See
  `doc/planning/combat_philosophy.md`.
- **Design bosses in each class's light.** A boss should *supply the inputs a class's
  skill needs* (vehicle: the per-class `content-lab-<class>-<boss>` encounters). Melee
  dash-weave needs a steady weavable ADD STREAM (`summon` count/max_alive/interval) --
  measured: today's single-target bosses give melee only 0.3-1.7 weaves/run, the gap to
  fill. Stormcaller = clustered/lined adds + walls; Pyrocaster = burnable waves + commit
  lulls; Necromancer = fodder to summon against + chip pressure for the HP->army->regen
  loop. Validate with the experiment (the class's signature signal should spike on its
  light-boss). Full guidance in doc/planning/combat_philosophy.md.
- **Bold with UNPROVEN classes (Rogue, Dicebinder, Templar), careful with proven
  ones (Fighter, mages, Necro); keep only what works.** Movement-flow mechanics are
  prototyped in the content-lab director (`ContentLabHazards.lua`), keyed by
  `player.class_name`, where Flow lives. Implemented so far:
  - **Dash-weave** (all melee): dashing through an enemy builds Flow + a class-keyed
    `weave_strike` (flow-scaled damage). `WEAVE_CLASS_DAMAGE` mult: Rogue 2.6,
    Dicebinder 1.6, Templar 1.0, Fighter 0.7.
  - **Rogue = flow-assassin (WORKS):** damage from movement; gragh-warband 0%->75%,
    gragh-duel 0%->50%/+19 micro. The headline bold win.
  - **Dicebinder = gambler (FLAVOR only):** weave damage is a deterministic roll, flow
    loads the dice (JACKPOTs). Thematic + works mechanically but doesn't carry the
    class (warband 25%); real lift needs its untouched dice system.
  - **Templar** lifts to ~50% on the warband via the weave-strike; still needs a
    distinct bulwark identity.
  - Proven classes left alone (Fighter just weapon-clears, flow 0% -- intended).
- **Grounded class identities** (from the kits, not telemetry):
  - Fighter = brawler (Berserk commit, Dash, Power Strike).
  - Templar = bulwark (Power Strike + Dash only; durable; anchor a lane).
  - Rogue = blur with an EMPTY kit (only Dash Attack) -- needs a real identity.
  - Dicebinder = gambler (Dash/Power Strike/Berserk + dice windows).
  - Necromancer = HP-as-currency army commander (Summon Dark Aspect costs HP,
    Skeleton Army swarm, Baleful Regeneration refills HP on ~800f cd, Pain free aura).
  - Pyrocaster = commit pyromancer (Fire Bolt poke, Ring of Flames aura window,
    Dragonform slow burst ult) -- NOT a kiter.
  - Stormcaller = positional ranged controller (wall-bounce spear, chain lightning,
    tornado zones) -- the deep "model" class; don't flatten it.

## Subsystem notes

- Class-interestingness experiment: `e2e_tests/run_class_interestingness_experiment.sh`
  (knobs via env; outputs under `saves/experiments/`, gitignored) +
  `aggregate_class_interestingness.py`. Measures per-class playstyle: outcome,
  micro/dodge, Flow combo sustain, range discipline, ability cadence, pacing.
- Flow (movement combo): in `ContentLabHazards.lua`. Clean dodges/threaded
  near-misses call `flow_build`; hits call `flow_break` (~60% drop); it bleeds
  over `FLOW_DECAY_FRAMES`. `flow_tier()` gates escalation; higher tier scales
  threaded-opening damage in `apply_micro_counter` and the player aura in
  `draw_flow`. Telemetry: `package_flow_*`. Design intent: movement IS the
  skill in this top-down genre -- do NOT add Souls i-frame dodges (they flatten
  positioning); reward sustaining precise movement instead.

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
  they are fixed up in `reallocate_internal_data()`. A tombstone-deref crash
  here was fixed (see Fixed-bug postmortems below).

## AddressSanitizer / UBSan

- Build: `./run.sh --headless --sanitize --build` → `build_debug_headless_asan/src/lanarts`
  (`-fsanitize=address -fsanitize=undefined`).
- Run a repro: `(cd runtime && ASAN_OPTIONS=detect_leaks=0:abort_on_error=0 \
  SDL_AUDIODRIVER=dummy LANARTS_MUTE_AUDIO=1 LANARTS_SMALL=1 \
  ../build_debug_headless_asan/src/lanarts engine.StartBalanceRun <args>)`
- Known **benign, pre-existing** UBSan findings (do NOT "fix" — they are
  functionally fine and the RNG one is determinism-critical):
  - `GameState.cpp:64` left-shift overflow in a hash.
  - `lcommon/mtwist.h:86` signed integer overflow in the Mersenne-Twister RNG;
    changing it would alter the seeded RNG sequence and break every fixed-seed
    test. Leave it.

## Fixed-bug postmortems

- **Collision-grid tombstone deref (FIXED):** `GameInstSet::object_radius_test`
  (and `object_rectangle_test`) guarded grid-bucket nodes with only
  `inst != NULL`, but a removed instance is marked with the sentinel
  `GAMEINST_TOMBSTONE = (GameInst*)1` and can still be referenced from a
  collision bucket under heavy object churn. Dereferencing the tombstone's
  `target_radius` crashed at ~0x25 (offset from address 1). Fix: guard both
  grid traversals with `valid_inst(inst)` (`inst > GAMEINST_TOMBSTONE`,
  excludes NULL and tombstone) — the invariant every other traversal here
  already uses. ASan/UBSan confirmed the tombstone deref and confirmed the fix.
