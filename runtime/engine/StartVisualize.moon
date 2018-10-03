argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"

game_init = (seed) ->
    GameState = require("core.GameState")
    -- Player config
    GameState.clear_players()
    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    random_seed(seed)

run_visualize = (raw_args) ->
    parser = argparse("lanarts", "Visualize lanarts maps.")
    parser\option("--seed", "Starting seed.", "12345678")
    args = parser\parse(raw_args)

    game_start = () ->
        -- (1) Init game
        game_init(tonumber(args.seed))
        package.loaded['maps.03_EasyOverworldDungeon'] = nil
        -- (2) Transfer to visualize loop
        map = require('maps.03_EasyOverworldDungeon')
        return debug_visualize_step(map, game_start)

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
        },
        entry_point: game_start,
        on_exit: () -> nil
    }

return run_visualize
