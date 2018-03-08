argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Settings = require "engine.Settings"
EventLog = require "ui.EventLog"

load_location_is_valid = (load_file) ->
    if not load_file
        return -- No savefile specified:
    if file_exists(load_file)
        return load_file
    else
        error("'#{load_file}' does not exist!")

game_init = (load_file=nil) ->
    Settings.settings_save()
    GameState = require("core.GameState")
    -- Player config
    GameState.clear_players()
    n_players = (if os.getenv("LANARTS_CONTROLLER") then 0 else 1) + #require("core.Gamepad").ids()
    GameState.register_player(settings.username, settings.class_type, Engine.player_input, true, 0, 0)
    for i=2,n_players
        GameState.register_player("Player " .. i, 'Fighter', Engine.player_input, true, i - 1, i)

    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    if load_location_is_valid(load_file)
        GameState.load(load_file)
    else
        EngineInternal.start_game()

--Parse lanarts command-line options
run_lanarts = (raw_args) ->
    log "Running Lanarts main function"
    parser = argparse("lanarts", "Run lanarts.")
    parser\option "--debug", "Attach debugger."
    parser\option "--nofilter", "Do not filter Lua error reporting of noise."
    parser\option "-C --context", "Amount of lines of Lua error context.", "4"
    parser\option "--settings", "Settings YAML file to use.", "settings.yaml"
    parser\option "--save", "Save file to save to.", false
    parser\option "--load", "Save file to load from.", false
    args = parser\parse(raw_args)
    settings = Settings.settings_load(args.settings)

    local menu_start, game_start, game_step
    menu_start = () ->
        return Engine.menu_start(game_start)

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
        Keys = require("core.Keyboard")
        GameLoop = require("GameLoop")
        GameState = require("core.GameState")

        -- (2) Run game loop & and see if game
        -- should be saved + exited
        if not GameLoop.game_step()
            -- Save and exit
            GameState.score_board_store()
            GameState.save(argv_configuration.save_file or "saves/savefile.save")
            -- Allocate a fresh GameState
            EngineInternal.init_gamestate_api(settings)
            -- Go back to menu
            return menu_start()
        -- (3) If F4 is pressed, pause the game
        if Keys.key_pressed(Keys.F4)
            GameLoop.loop_control.game_is_paused = not GameLoop.loop_control.game_is_paused
        -- (4) If Escape is pressed, tell user about shift+escape
        if Keys.key_pressed(Keys.ESCAPE)
            for _ in screens()
                EventLog.add("Press Shift + Esc to exit, your progress will be saved.")
        -- (5) Loop again
        return game_step

    return StartEngine.start_engine {
        :settings,
        entry_point: menu_start,
        debug: args.debug,
        save: args.save,
        load: args.load
        nofilter: args.nofilter
        on_exit: Settings.settings_save
    }

return run_lanarts
