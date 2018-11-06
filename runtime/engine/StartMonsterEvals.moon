argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"

game_init = (seed, class_name, monster_spawns) ->
    GameState = require("core.GameState")
    -- Player config
    GameState.clear_players()
    GameState.register_player 'Playabot', class_name, -- Name and class
        (require("ai.BotInputSource").create), -- input source
        true, 0, --netid
        0 -- team

    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    random_seed(seed)
    Engine.first_map_create = () ->
        return require("maps.0X_Arena").generate_and_spawn(monster_spawns)
    EngineInternal.start_game()

    -- Set up input for first game step
    GameState.input_capture()
    GameState.input_handle()

run_bot_tests = (raw_args) ->
    parser = argparse("lanarts", "Run lanarts bot tests.")
    parser\option("--steps", "Maximum number of steps to take.", nil)
    parser\option("--event_log", "Event log to write to.", nil)
    parser\option("--seed", "Starting seed.", "12345678")
    parser\option("--class", "Bot class.", "Fighter")
    parser\option("--xp", "Bot XP.", "Fighter")
    parser\option("--comparison_event_log", "Event log to compare to.", nil)
    args = parser\parse(raw_args)

    local game_start, monster_eval_step, game_step

    game_start = () ->
        return ResourceLoading.ensure_resources_before(monster_eval_step)

    game_exit = (msg) ->
        print msg
        return nil

    monster_eval_step = () ->
        enemy_names = table.sorted_key_list(enemies)
        idx = 1
        next_sim = () ->
            if idx > #enemy_names then return nil
            -- Continue simulation iteration:
            game_init(tonumber(args.seed), args.class, {enemy_names[idx]})
            idx += 1
            return game_step(next_sim)
        return next_sim()

    game_step = (next_sim) ->
        step = () ->
            -- (1) Load dependencies
            GameState = require("core.GameState")
            GameLoop = require("GameLoop")

            -- (2) Update with frame stats
            step_only = (GameState.frame % 100 ~= 0)

            -- (3) Check if we are at final frame:
            if args.steps and GameState.frame >= tonumber(args.steps)
                return next_sim()

            -- (4) Run game loop & and see if game
            -- should be saved + exited
            if not GameLoop.game_step(step_only)
                return next_sim()

            -- (5) Loop again
            return step
        EngineHooks.game_won = () ->
            -- Exit on next step
            step = next_sim
        return step()

    return StartEngine.start_engine {
        settings: { -- Settings object
            -- Window settings
            fullscreen: false
            view_width: 640
            view_height: 480
            -- Font settings
            font: 'fonts/Gudea-Regular.ttf'
            menu_font: 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            time_per_step: 0
            invincible: true
            event_log: args.event_log
            comparison_event_log: args.comparison_event_log
        },
        entry_point: game_start,
        debug: args.debug,
        on_exit: () -> nil
    }

return run_bot_tests
