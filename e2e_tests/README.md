e2e_tests:
    A series of bash scripts that, using standard features of the Lanarts executable (albeit, built headless), test Lanarts in-depth in simulated play / network scenarios.

    Note: Periodically run tests not on headless to ensure no graphical crashes / bugs.

Content lab APIs:
    ./run.sh --headless engine.StartContentApiSmoke

    Validates that the content-lab catalog, resolved run summary, enemy option
    APIs, fight setup, analysis plan, and generated resource matrix return
    parseable JSON with the expected high-level shape. Map-generation payloads
    include deterministic preview geometry, enemy pins, and preview-derived
    threat profiles so the web lab does not have to invent local-only sketches
    or pressure labels for algorithm comparisons.
    Run summaries also expose StartBalanceRun args for the selected class,
    level, loadout, map or encounter, and package pressure.
    Enemy option APIs expose curated enemy groups, packs, arena launch args,
    StartBalanceRun args, and StartMonsterEvals args so the web lab can build
    enemy-balance probes without relying on free-text monster names.
    Loadout payloads include a readiness read that maps selected gear and known
    spells to the active package demands, such as rescue blocks, add cleanup,
    AOE movement, and orb spacing.
    Build-matrix payloads expose curated class archetypes as concrete loadout
    strings with readiness scores and launch/balance commands, so the web lab
    can spec a hero before entering a package fight.
    Micro-drill payloads expose launchable rescue, orb/AOE, add-cleanup, and
    stacked-package practice setups with isolated components, scorecards, and
    micro plans. Presentation controls expose those drills as selectable API
    options so the web lab can apply a full practice config from one selector.
    Run summaries and fight setup payloads also include class/enemy matchup
    reads for red/fire, white, and black resistance checks, with telemetry keys
    that explain whether the pairing is suppressing damage conversion.
    Generated resource matrices include a caster-matchup sweep for Pyrocaster
    red-resist checks, Stormcaller counter/mirror checks, and Necromancer
    black-resist regen checks.

Balance simulations:
    ./run.sh --headless engine.StartBalanceRun --steps 20000 --runs 5 --classes Fighter,Necromancer,Pyrocaster,Stormcaller
    ./run.sh --headless engine.StartBalanceRun --steps 20000 --runs 5 --levels 1,3
    ./e2e_tests/run_content_lab_native.sh --class Dicebinder --encounter gragh-duel
    ./e2e_tests/run_content_lab_native.sh --draw --class Rogue --encounter executioner-rescue-package --package_components escort,orbit,pulse,sweep,rescue
    ./e2e_tests/test_content_lab_native_fast.sh
    ./e2e_tests/test_content_package_balance_smoke.sh

    Outputs default to saves/balance_runs.csv, saves/balance_samples.csv,
    saves/balance_spells.csv, saves/balance_classes.csv,
    saves/balance_spell_stats.csv, saves/balance_enemy_stats.csv,
    saves/balance_report.csv, and saves/balance_runs.json relative to the
    runtime directory. The JSON output contains the run list plus class, spell,
    enemy, and report rows.
    By default, --seed_mode paired reuses each seed across every class for a
    given starting level and run index. Use --seed_mode unique for independent
    seeds per run.
    Run, sample, and spell rows include seed_group and class_run_index columns
    for paired comparisons. In JSON, seed values are strings so exact large
    seeds survive serialization.
    Necromancer runs also emit Pain and Baleful Regeneration behavior counters
    in the run and aggregate tables.
    Run `kills` count hostile monster deaths credited to the party, so
    Necromancer summons are included in pacing. Direct player last-hits are
    preserved separately as `player_kills`.
    Pyrocaster runs emit Ring of Flames combat-positioning counters.
    Aggregate rows include raw pacing metrics and kills per 1000 combat frames
    so low kill totals can be separated from low encounter contact.
    The report emits contact warnings when combat-frame exposure is far from
    baseline and emits pacing context instead of raw-kill balance flags when
    combat-adjusted kill rate is still in range. It also suppresses combat-efficiency
    underperformance flags for high-contact runs when overall kill pacing is in
    range, and emits pacing context instead of a class-balance failure when
    combat efficiency is low but overall kill pace is still healthy.
    It also emits an absolute survival flag when a class dies in at least half
    of stable runs, even if every class is failing at the same rate.
    Ability-order totals are kept in the raw tables for behavior analysis, but
    are not used as cross-class balance report flags.
    Item-order totals are also raw telemetry; inventory access differs by class.
    Enemy stats aggregate kills and visible-enemy-frame exposure per class,
    starting level, and monster type, including XP share and kills per 1000
    visible enemy frames. This is useful for separating frequent-but-healthy
    enemies from rare enemies that dominate run pacing when they appear.
    Run and sample rows also include alive_enemy_counts, alive_enemy_hp, and
    alive_enemy_max_hp, and enemy stats aggregate end-of-run survival and HP
    remaining. Use those fields to distinguish enemies that barely survive
    from enemies that remain mostly untouched when a run ends.
    Multi-level sweeps produce separate class and spell aggregate rows per
    starting level, which is useful for checking later spell unlocks.
    Level 5 is already late game for player characters; prefer using loadout
    presets at level 5 over raising the player level when testing late maps.
    Use --start_map_desc to start every run in a specific map descriptor, for
    example map_descs.PixullochiaDepths, when testing late-game enemy pools
    without waiting for portal traversal.
    Use --encounter and --package_intensity to run the same content-lab
    packaged arenas used by the web shell, for example
    --encounter gragh-rescue-package --package_intensity high. Packaged runs
    include solo 1v1 full-package cases such as mana-sapper-ritual-package,
    where a support enemy is tested with orbit, AOE, sweep, and add rhythms,
    plus hell-warrior-flame-package for fast red contact pressure with capped
    extra Imp portals and readable flame lanes, and stara-pain-star-package
    for black aura/projectile pressure with skull lanes and Skeleton cleanup,
    while crypt-keeper-aura-package now layers black aura, bone orbit, crossing
    bone scythes, DREAD healing on mechanic hits, nova timing, and Skeleton
    cleanup in a larger 1v1 room.
    pixulloch-web-package adds a larger final-boss arena with native black
    summons, rotating web knots, crossing venom lanes, a web nova beat, and
    Giant Spider cleanup. waxanarian-venom-package adds fast green boss
    pursuit with stinger knots, venom lanes, poison bloom timing, and Poison
    Bee cleanup.
    emit package_outcome, escort HP, rescue saves, clutch/perfect saves,
    clutch/perfect rescue window cues, danger-clock coverage, pull redirects,
    and misses in run and sample rows; ally-down package failures end the run
    as ally_down. Automated players also emit rescue_intercept_frames,
    rescue_hold_frames, and rescue_late_reaction_frames so package clears can
    be separated from actual body-block micro. Package rows also derive rescue
    opportunities plus save, clutch, perfect, early, timing-score, pull,
    timed-pull, perfect-pull, clutch/perfect/timed-pull margin, late/perfect
    cover, uncovered danger-window, near-line commit rate/distance, and
    counter-opening rates so rescue packages can be compared without
    hand-calculating rates from raw counts. Package rows also expose threaded
    dodge openings plus opening
    damage/stagger totals so clean footwork can be separated from brute-force
    clears. Package rows expose primary target HP, current package aggro
    target, primary aggro player/ally/other/none rates, inferred-target rate,
    target switches, the current/next package threat, and live rescue-window
    fields such as next strike, active count, blocked/late/perfect state, and
    active pull target for HUD and replay checks.
    Package pressure rows track active/warning component stacks, peak stack,
    overlap rate, sync-beat frames, sync bursts, sync peak, unique mechanic
    types, pressure signature switches, and dominant-mechanic share, which
    helps distinguish true full-package micro from isolated or samey pressure.
    The report also emits package_pressure, package_sync, package_variety, and
    package_difficulty rows when package overlap is only serial pressure, sync
    beats are missing, package variety is weak, or the selected automation
    profile is clearing too comfortably.
    Rescue package rows also track pull/taunt total frames, uptime rate,
    pull-discipline rate, taunt-strain rate, late-window pull rates, and
    average timed-pull margins, plus pull-snap burst and strain-pulse counts,
    so clears can be flagged when forced target control is carrying the fight
    instead of few-frame snap-pull saves.
    Rescue rows also expose the snap opportunity as a first-class state:
    package_rescue_active_snap_ready / _distance / _remaining mark when a
    clutch/perfect pull is convertible during a live line, while
    package_rescue_snap_window_frames, _snap_ready_frames, and
    _snap_ready_rate aggregate how often achievable snap windows were
    actually positioned for. A package_rescue_broad_pull_index (pull
    uptime weighted by non-timed pulls) and a broad-pull scorecard check
    flag high taunt reliance when clutch/perfect pull rates are low.
    Rescue-package report rows summarize save, clutch, perfect, early,
    timing-score, pull, timed-pull, pull-discipline, perfect-pull,
    taunt-uptime, taunt-strain, late-cover, perfect-cover, uncovered
    danger-window, counter-window, counter-damage, counter-stagger, fatal-miss,
    and buffered-miss rates, then warn when pull/taunt control or early parking
    is replacing the intended few-frame body-block micro.
    When a balance run matches a content-lab micro drill, the report also emits
    drill_scorecard rows for that drill's pass/fail targets, including rescue
    saves, perfect saves, taunt reliance, add cleanup, stack overlap, spacing,
    and regen correction where relevant.
    Orb, sweep, summon, and AOE package rows likewise expose active/warning counts and
    remaining frames so timeline samples can be matched against the in-game HUD.
    Package runs also emit package_micro_score, package_micro_best_streak,
    package_micro_dodges, package_micro_thread_cues,
    package_micro_streak_bursts, and package_micro_hits so clean execution can be
    separated from clears that brute-force avoidable mechanics. Package reports
    summarize those micro rows and warn when a class clears while averaging
    negative micro score. The package_micro_dodges field tracks targeted or
    near-miss threaded avoids; hazards that expire without threatening a player
    no longer inflate the count. Gragh packages also emit
    package_micro_punish_total for Gragh Fury healing caused by avoidable
    mechanic hits, plus package_micro_punish_break_total when clutch/perfect
    rescue saves break that Fury back off. Clean dodge-streak milestones
    (MICRO x3/x5+) also break un-broken Fury/Dread healing, so non-rescue
    duels keep a skill-gated counterplay to the heal; the break is capped at
    the healing earned so it is never free damage.
    Dicebinder runs also emit dice_rolls_total, per-face roll counts,
    face swaps, roll buildup FX, surge cues, lock cues, suspense pips, guard
    cues, reveal cues, finale FX, cooldown-trim frames, high-roll share,
    distinct faces, max-face share, high-roll streaks, repeat rolls,
    dry-roll streaks, streak-guard breaks, anti-repeat rerolls,
    effective Dash/Power refreshes, jackpot rolls, and buff active frames so
    skill-dice RNG can be modeled separately from package execution.
    Dicebinder drill scorecards also watch roll cadence, animation frames, face motion,
    surge/lock-in cues, variety switches, high-roll share, face concentration,
    repeat caps, streak caps, and buff uptime directly.
    Drills tagged `stress` replace the practice clear-floor check with a
    skill-buffer ceiling, which models late-game content that a primitive
    human-profile bot should not clear reliably.
    The automated player now actively dodges content-lab orb, pulse, and
    sweep hazards (not just rescue lines), moving to the least-dangerous
    adjacent tile when it stands inside or just ahead of an active/imminent
    danger zone. Dodging is skill-gated: perfect reacts early with a wide
    margin, while skilled/human/sloppy react later, with a smaller margin,
    and sometimes skip the read (hazard_dodge_skip_chance). hazard_dodge_frames
    and hazard_dodge_skip_frames are emitted per run so package micro is now a
    usable measurement instrument rather than every profile eating mechanics.
    Use --package_components orbit,pulse,sweep,summon,escort,rescue to isolate
    package parts while keeping the same encounter arena, monsters, and
    pressure preset. A rescue component automatically keeps its escort target.
    Content API fight setup rows also expose a per-rhythm micro plan with
    class posture, effective reaction windows, and telemetry for separating
    real micro from forgiving enemy pressure.
    Run summaries and fight setup payloads include a predicted first-720-frame
    pressure preview plus diagnosis so package stack timing can be inspected
    before launching.
    Use --loadout_preset late to give each class a representative equipped
    late-game kit without inflating the starting level. Use --loadout to append
    comma-separated item specs such as `Health Potion*3,Hydra Ring`, and
    --loadout_label to name the build in the output tables.
    Class, spell, and enemy aggregates are grouped by class, starting level,
    and loadout so equipped late-map sweeps are not baselined against starter
    gear runs.
    Use --min_report_runs and --report_delta_pct to tune report sensitivity.
    For content-lab iteration, prefer ./e2e_tests/test_content_lab_native_fast.sh
    before rebuilding Emscripten. It runs the same packaged Gragh/Dicebinder
    and rescue-package content through native binaries with audio muted. The
    script builds each needed native binary once, then runs scenarios directly
    from build_debug_headless/src/lanarts or build_debug/src/lanarts. Set
    LANARTS_SKIP_BUILD=1 to reuse existing binaries, or LANARTS_DRAW_SMOKE=1
    to also exercise the normal native draw path for procedural FX without
    entering the slower Emscripten loop.
    Dicebinder reports include fair-bag roll counts, animation frames, face
    swaps, staged buildup FX, surge cues, lock cues, suspense pips, guard cues,
    reveal cues, finale FX, cooldown trims, high-roll share, distinct faces,
    max-face share, high-roll streaks, dry-roll streaks, high/dry streak guards,
    repeat rolls, bag refills,
    anti-repeat rerolls, dash-window frames, and effective cooldown refreshes
    so exciting D6 windows can be checked without hiding class power inside
    RNG streaks.
    The fast native smoke also includes a fixed-seed three-run Dicebinder
    package batch that previously pushed procedural dice FX outside arena
    bounds; map-generated fx_bounds keep those animated bursts visible without
    tripping native instance-bound assertions.
    The content API smoke also honors LANARTS_SKIP_BUILD=1, so Lua/catalog
    changes can be checked against the existing native binary without a rebuild.
    For ad hoc tuning, use ./e2e_tests/run_content_lab_native.sh with the same
    class, encounter, package component, bot profile, seed, and draw options
    surfaced by the web lab. It writes persistent CSV/JSON outputs under the
    ignored saves/content_lab_native directory and can reuse existing binaries
    with LANARTS_SKIP_BUILD=1 or --no-build.

Monster simulations:
    ./run.sh --headless engine.StartMonsterEvals --steps 1000 --runs 3 --level 1 --monster GiantRat,Hound,CloudElemental
    ./run.sh --headless engine.StartMonsterEvals --steps 1000 --runs 3 --levels 1,3 --monster all --configs BaseFighter,BaseNecromancer

    Monster names can use compact aliases with spaces removed, such as
    GiantRat, CloudElemental, or BlackMamba. Outputs default to
    saves/monster_stats.csv, saves/monster_aggregate.csv,
    saves/monster_report.csv, and saves/monster_stats.json. Raw rows contain
    one arena duel per config, monster, run, and player level. Aggregate rows
    report kill rate, step-limit rate, contact rate, average damage taken,
    damage per 1000 frames, damage per XP, and XP per 100 damage. Contact
    adjusted variants of these metrics exclude runs where the arena duel never
    engaged, and the report uses those contact metrics when flagging enemies
    that are unusually durable, unusually damaging, or poor XP rewards relative
    to the other enemies in the same config and level sweep.
    Player damage is tracked as observed gross HP loss, with net HP loss,
    healing, and minimum HP also emitted so regenerative playstyles are still
    visible. BaseNecromancer uses Pain while opportunistically casting Baleful
    Regeneration, matching its intended sustain-through-damage loop.
    Monster rows also include the config's primary damage type, the monster's
    types, and the monster's resistance to that primary damage. Report messages
    call out resisted primary matchups so elemental-counter warnings can be
    separated from raw enemy overtuning.
    Raw, aggregate, and report rows include source_status and source_refs
    columns. Map-spawned enemies keep warning severity, while definition-only
    or summon-only encounters are downgraded to info by default so --monster all
    highlights live enemy balance first. Use --inactive_warning_severity warning
    when intentionally tuning inactive definitions or summon-only monsters.
