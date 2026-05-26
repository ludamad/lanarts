e2e_tests:
    A series of bash scripts that, using standard features of the Lanarts executable (albeit, built headless), test Lanarts in-depth in simulated play / network scenarios.

    Note: Periodically run tests not on headless to ensure no graphical crashes / bugs.

Balance simulations:
    ./run.sh --headless engine.StartBalanceRun --steps 20000 --runs 5 --classes Fighter,Necromancer,Pyrocaster,Stormcaller
    ./run.sh --headless engine.StartBalanceRun --steps 20000 --runs 5 --levels 1,3

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
    baseline and suppresses raw-kill underperformance flags when combat-adjusted
    kill rate is still in range. It also suppresses combat-efficiency
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
    Multi-level sweeps produce separate class and spell aggregate rows per
    starting level, which is useful for checking later spell unlocks.
    Use --start_map_desc to start every run in a specific map descriptor, for
    example map_descs.PixullochiaDepths, when testing late-game enemy pools
    without waiting for portal traversal.
    Use --min_report_runs and --report_delta_pct to tune report sensitivity.

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
