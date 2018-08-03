argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"

game_init = (seed, class_name) ->
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
    EngineInternal.start_game()

fps_updater = () ->
    GameState = require("core.GameState")
    fps_timer = timer_create()
    fps_count, fps_lastframe, fps = 1, 0, nil
    fps = 0
    return () ->
        frame_increase = math.max(0, GameState.frame - fps_lastframe)
        fps_lastframe = GameState.frame
        fps_count = fps_count + frame_increase

        ms = fps_timer\get_milliseconds()
        if ms > 1000 then
            fps = fps_count / ms * 1000
            fps_timer\start()
            fps_count = 0
        return fps

run_bot_tests = (raw_args) ->
    parser = argparse("lanarts", "Run lanarts bot tests.")
    parser\option "--test_save_and_load", "Should we have headless execution?", false
    parser\option("--steps", "Maximum number of steps to take.", nil)
    parser\option("--event_log", "Event log to write to.", nil)
    parser\option("--seed", "Starting seed.", "12345678")
    parser\option("--class", "Bot class.", "Fighter")
    parser\option("--comparison_event_log", "Event log to compare to.", nil)
    args = parser\parse(raw_args)

    local game_start, game_step

    game_start = () ->
        return ResourceLoading.ensure_resources_before () ->
            -- (1) Init game
            game_init(tonumber(args.seed), args.class)

            -- (2) Set up input for first game step
            GameState = require("core.GameState")
            GameState.input_capture()
            if not GameState.input_handle()
                error("Game exited")

            -- (3) Transfer to game loop
            return game_step()

    game_step = () -> 
        fps_update = fps_updater()
        step = () ->
            -- (1) Load dependencies
            GameState = require("core.GameState")
            GameLoop = require("GameLoop")
            
            -- (2) Update with frame stats
            step_only = (GameState.frame % 100 ~= 0)
            fps = fps_update()
            if not step_only
                print "Frame", GameState.frame, "FPS", fps

            -- (3) Check if we are at final frame:
            if args.steps and GameState.frame >= tonumber(args.steps)
                print "Final frame achieved."
                -- Exit
                return nil

            -- (4) Run game loop & and see if game
            -- should be saved + exited
            if not GameLoop.game_step(step_only)
                -- Exit
                return nil

            -- (5) Loop again
            return step
        EngineHooks.game_won = () ->
            -- Exit on next step
            print "GAME WAS WON"
            step = () ->
                return nil
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
