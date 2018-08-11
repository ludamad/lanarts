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
    Settings.settings_save(settings)
    GameState = require("core.GameState")
    -- Player config
    GameState.clear_players()
    n_players = (if os.getenv("LANARTS_CONTROLLER") then 0 else 1) + #require("core.Gamepad").ids()
    GameState.register_player(settings.username, settings.class_type, Engine.player_input, true, 0, 0)
    classes = {"", "Pyrocaster", "Fighter", "Pyrocaster"}
    for i=2,n_players
        GameState.register_player("Player " .. i, classes[i], Engine.player_input, true, i - 1, 0)

    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    print "load_location_is_valid(load_file)", load_location_is_valid(load_file)
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
    parser\option "--class", "Class to immediately start with.", false
    parser\option "--macro", "Script to run from debug_scripts/"
    parser\option "-C --context", "Amount of lines of Lua error context.", "4"
    parser\option "--settings", "Settings YAML file to use.", "settings.yaml"
    parser\option "--save", "Save file to save to.", false
    parser\option "--load", "Save file to load from.", false
    args = parser\parse(raw_args)
    -- TODO dont use global 'settings' object
    initial_settings = Settings.settings_load(args.settings)

    local entry_point, game_start, game_step
    entry_point = () ->
        if args.class
            settings.class_type = args.class
            return game_start()
        else
            return Engine.menu_start(game_start)

    game_start = (load_file=nil) ->
        return ResourceLoading.ensure_resources_before () ->
            -- (1) Init game
            -- TODO refactor:
            if load_file and args.load
                load_file = args.load
            game_init(load_file)

            -- (2) Run macro script
            if args.macro
                require("debug_scripts." .. args.macro)

            -- (3) Set up input for first game step
            GameState = require("core.GameState")
            GameState.input_capture()
            if not GameState.input_handle()
                error("Game exited")

            -- (4) Transfer to game loop
            return game_step()

    game_step = () ->
        -- (1) Load dependencies
        Keys = require "core.Keyboard"
        GameLoop = require "GameLoop"
        GameState = require "core.GameState"
        Network = require "core.Network"

        -- (2) Consume network events (TODO this should link to player input sources)
        perf.timing_begin("**Sync Message**")
        Network.sync_message_consume()
        perf.timing_end("**Sync Message**")

        -- (3) Run game loop & and see if game
        -- should be saved + exited
        if not GameLoop.game_step()
            -- Save and exit
            GameState.score_board_store()
            GameState.save(args.save or "saves/savefile.save")
            -- Allocate a fresh GameState
            EngineInternal.init_gamestate_api(initial_settings)
            -- Go back to menu
            return entry_point()
        -- (4) If F4 is pressed, pause the game
        if Keys.key_pressed(Keys.F4)
            GameLoop.loop_control.game_is_paused = not GameLoop.loop_control.game_is_paused
        -- (5) If Escape is pressed, tell user about shift+escape
        if Keys.key_pressed(Keys.ESCAPE)
            for _ in screens()
                EventLog.add("Press Shift + Esc to exit, your progress will be saved.")
        -- (6) Backup save every ~5 minutes
        if GameState.frame > 0 and GameState.frame % (100 * 60 * 5) == 0
            for _ in screens()
                EventLog.add("BACKING UP SAVE!", COL_WHITE)
            GameState.save("saves/backup-frame-" .. GameState.frame .. ".save")

        -- (7) Loop again
        return game_step

    return StartEngine.start_engine {
        settings: initial_settings,
        :entry_point,
        debug: args.debug,
        nofilter: args.nofilter
        on_exit: Settings.settings_save
    }

return run_lanarts
