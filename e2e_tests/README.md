e2e_tests:
    A series of bash scripts that, using standard features of the Lanarts executable (albeit, built headless), test Lanarts in-depth in simulated play / network scenarios.

    Note: Periodically run tests not on headless to ensure no graphical crashes / bugs.

Balance simulations:
    ./run.sh --headless engine.StartBalanceRun --steps 20000 --runs 5 --classes Fighter,Necromancer,Pyrocaster,Stormcaller

    Outputs default to saves/balance_runs.csv, saves/balance_samples.csv,
    saves/balance_spells.csv, saves/balance_classes.csv,
    saves/balance_spell_stats.csv, saves/balance_report.csv, and
    saves/balance_runs.json relative to the runtime directory.
    The JSON output contains the run list plus class, spell, and report rows.
    Necromancer runs also emit Pain and Baleful Regeneration behavior counters
    in the run and aggregate tables.
    Ability-order totals are kept in the raw tables for behavior analysis, but
    are not used as cross-class balance report flags.
    Use --min_report_runs and --report_delta_pct to tune report sensitivity.
