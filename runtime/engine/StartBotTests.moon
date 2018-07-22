argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"
-- ProgrammableInputSource = require "input.ProgrammableInputSource"
-- BotInputSource = require "ai.BotInputSource"

game_init = (load_file=nil) ->
    GameState = require("core.GameState")
    -- Player config
    GameState.clear_players()
    GameState.register_player 'Playabot', 'Fighter', -- Name and class
        (require("ai.BotInputSource").create), -- input source
        true, 0, --netid
        0 -- team

    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    EngineInternal.start_game()

run_bot_tests = (raw_args) ->
    parser = argparse("lanarts", "Run lanarts bot tests.")
    parser\option "--test_save_and_load", "Should we have headless execution?", false
    args = parser\parse(raw_args)

    local game_start, game_step

    game_start = (load_file=nil) ->
        return ResourceLoading.ensure_resources_before () ->
            -- (1) Init game
            game_init(load_file)

            -- (2) Set up input for first game step
            GameState = require("core.GameState")
            GameState.input_capture()
            if not GameState.input_handle()
                error("Game exited")

            -- (3) Transfer to game loop
            return game_step()

    game_step = () ->
        -- (1) Load dependencies
        GameLoop = require("GameLoop")

        -- (2) Run game loop & and see if game
        -- should be saved + exited
        if not GameLoop.game_step()
            -- Exit
            return nil
        -- (3) Loop again
        return game_step

    return StartEngine.start_engine {
        settings: { -- Settings object
            -- Window settings
            fullscreen: false
            view_width: 640
            view_height: 480
            -- Font settings
            font: 'fonts/Gudea-Regular.ttf'
            menu_font: 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            time_per_step: 1
            invincible: true
        },
        entry_point: game_start,
        debug: args.debug,
        on_exit: () -> nil
    }

return run_bot_tests
