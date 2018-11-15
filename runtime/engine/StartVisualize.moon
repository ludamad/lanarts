argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"
Display = require "core.Display"
{:debug_visualize_step} = require "maps.DebugUtils"

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
    parser\option("--map_desc", "Map desc module.", "map_descs.HiveEntrance")
    args = parser\parse(raw_args)
    seed = tonumber(args.seed)
    game_start = () ->
        return ResourceLoading.ensure_resources_before () ->
            -- (1) Init game
            game_init(seed)
            -- (2) Transfer to visualize loop
            package.loaded[args.map_desc] = nil -- Ensure fresh load of file
            map = require(args.map_desc)\compile()
            seed += 1
            return debug_visualize_step(map, game_start)

    return StartEngine.start_engine {
        settings: { -- Settings object
            -- Window settings
            fullscreen: false
            view_width: 800
            view_height: 800
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
