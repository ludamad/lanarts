EngineInternal = require "core.EngineInternal"
Tasks = require "networking.Tasks"
argparse = require "argparse"
Display = require "core.Display"
yaml = require "yaml"

load_location_is_valid = (load_file) ->
    if not load_file
        return -- No savefile specified:
    if file_exists(load_file)
        return load_file
    else
        error("'#{load_file}' does not exist!")

-- Parse settings object from settings yaml file
parse_settings = (settings_file) ->
    log_info "Parsing #{settings_file}"
    yaml_text = file_as_string(settings_file)
    if not yaml_text
        error("Fatal error: #{settings_file} not found, the game is probably being loaded from the wrong place.")
    raw_settings = try {
        do: () -> yaml.load(yaml_text)
        catch: (err) -> error("Invalid YAML syntax:\n#{err}")
    }
    settings = {key, value for {key, value} in *raw_settings}
    pretty_print(settings)
    return settings

cached_settings = nil

-- Adapt the settings object based on context
-- Initialize the game state object
engine_init = (settings) ->
    -- (1) Initialize sound, network, etc subsystems
    EngineInternal.init_subsystems()

    -- (2) Define major engine hooks, such as level to generate
    -- TODO move this to a more flexible system
    require "engine.EngineBase"

    -- (3) Adapt settings object based on environment variables
    -- Set settings width & height to screen width & height if equal to 0
    {screen_width, screen_height} = Display.screen_size
    if settings.view_width == 0
        settings.view_width = screen_width
    if settings.view_height == 0
        settings.view_height = screen_height

    if os.getenv("LANARTS_SMALL")
        settings.fullscreen = false
        settings.view_width = 800
        settings.view_height = 600

    if os.getenv("LANARTS_INVINCIBLE")
        settings.invincible = true

    log "Initializing GameState api..."
    with_mutable_globals () ->
        setmetatable _G, nil -- Allow globals to be set
        cached_settings = settings
        EngineInternal.init_gamestate_api(settings)
        require "globals.GameUtils"
        Display.initialize("Lanarts", {settings.view_width, settings.view_height}, settings.fullscreen)
    return settings

engine_exit = () ->
    perf.timing_print()

    print( "Step time: " .. string.format("%f", perf.get_timing("**Step**")) )
    print( "Draw time: " .. string.format("%f", perf.get_timing("**Draw**")) )

RESOURCES_LOADED = false

game_init = (load_file=nil) ->
    GameState = require("core.GameState")
    if not RESOURCES_LOADED then with_mutable_globals () ->
        EngineInternal.init_resource_data()
        Engine.resources_load()
        RESOURCES_LOADED = true

    -- Player config
    GameState.clear_players()
    n_players = (if os.getenv("LANARTS_CONTROLLER") then 0 else 1) + #require("core.Gamepad").ids()
    GameState.register_player(settings.username, settings.class_type, Engine.player_input, true, 0)
    for i=2,n_players
        GameState.register_player("Player " .. i, settings.class_type, Engine.player_input, true, 0)

    log "Initializing GameState object..."
    EngineInternal.init_gamestate()
    if load_location_is_valid(load_file)
        GameState.load(load_file)
    else
        EngineInternal.start_game()

--Parse lanarts command-line options
main = (raw_args) ->
    log "Running Lanarts main function"
    parser = argparse("lanarts", "Run lanarts.")
    parser\option "--debug", "Attach debugger."
    parser\option "--nofilter", "Do not filter Lua error reporting of noise."
    parser\option "-C --context", "Amount of lines of Lua error context.", "4"
    parser\option "--settings", "Settings YAML file to use.", "settings.yaml"
    parser\option "--save", "Save file to save to.", false
    parser\option "--load", "Save file to load from.", false
    args = parser\parse(raw_args)

    local engine_main, menu_start, game_start, game_step
    engine_main = () ->
        -- (1) Handle debug options
        if args.debug
            debug.attach_debugger()

        -- (2) Handle save file options
        argv_configuration.save_file = args.save
        argv_configuration.load_file = args.load

        -- (3) Handle error reporting options
        ErrorReporting = require "ErrorReporting"
        if args.nofilter
            ErrorReporting.filter_patterns = {}
        ErrorReporting.context = tonumber(args.context)

        -- (4) Initialize subsystems + game state object
        engine_init(parse_settings args.settings)

        -- (5) Check for savefiles
        if load_location_is_valid(args.load)
            return game_start(args.load)

        -- (6) Transfer to menu loop
        return menu_start()

    menu_start = () ->
        return Engine.menu_start(game_start)

    game_start = (load_file=nil) ->
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
            EngineInternal.init_gamestate_api(cached_settings)
            -- Go back to menu
            return menu_start()
        -- (3) If F4 is pressed, pause the game
        if Keys.key_pressed(Keys.F4)
            GameLoop.loop_control.game_is_paused = not GameLoop.loop_control.game_is_paused
        -- (4) If Escape is pressed, tell user about shift+escape
        if Keys.key_pressed(Keys.ESCAPE)
            for _ in screens()
                EventLog.add("Press Shift + Esc to exit, your progress will be saved.")
        -- (5) If F5 is pressed, force a network sync
        -- TODO fix this / check if working still
        if Keys.key_pressed(Keys.F5)
            GameState.input_capture(true) -- reset input
            Network.sync_message_send()

        -- (6) Loop again
        return game_step

    -- Entry point
    engine_state = engine_main
    engine_loop = () ->
        -- Simple state machine
        Tasks.run_all()
        engine_state = engine_state()
        if engine_state == nil
            engine_exit()
            return false
        return true

    return engine_loop

return main
