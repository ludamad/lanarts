argparse = require "argparse"
yaml = require "yaml"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"

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
    return settings

get_settings = (settings_file) ->
    settings = parse_settings(settings_file)
    if file_exists("saves/saved_settings.yaml")
        saved_settings = parse_settings("saves/saved_settings.yaml")
        for k, v in pairs(saved_settings)
            settings[k] = v
    return settings

settings_save = () ->
    -- Injected into settings in further engine runs:
    {:time_per_step, :username, :class_type, :frame_action_repeat, :regen_on_death} = settings
    yaml_str = yaml.dump({:time_per_step, :username, :class_type, :frame_action_repeat, :regen_on_death})
    -- ensure_directory("saves")
    file_dump_string("saves/saved_settings.yaml", yaml_str)

game_init = (load_file=nil) ->
    settings_save()
    GameState = require("core.GameState")
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
    settings = get_settings(args.settings)

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
        on_exit: settings_save
    }

return run_lanarts
