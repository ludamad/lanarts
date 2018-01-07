EngineInternal = require "core.EngineInternal"
argparse = require "argparse"
Display = require "core.Display"
yaml = require "yaml"

-- Parse settings object from settings yaml file
parse_settings = (settings_file) ->
    yaml_text = file_as_string(settings_file)
    if not yaml_text
        error("Fatal error: #{settings_file} not found, the game is probably being loaded from the wrong place.")
    raw_settings = try {
        do: () -> yaml.load(yaml_text)
        catch: (err) -> error("Invalid YAML syntax:\n#{err}")
    }
    settings = {key, value for {key, value} in *raw_settings}
    return settings

-- Adapt the settings object based on context
-- Initialize the game state object
engine_init = (settings) ->
    -- (1) Initialize sound, network, etc subsystems
    EngineInternal.init_subsystems()

    -- (2) Define major engine hooks, such as level to generate
    -- TODO move this to a more flexible system
    require "EngineBase"

    -- (3) Adapt settings object based on environment variables
    -- Set settings width & height to screen width & height if equal to 0
    {screen_width, screen_height} = Display.screen_size
    if settings.view_width == 0
        settings.view_width = screen_width
    if settings.view_height == 0
        settings.view_height = screen_height

    if os.getenv("LANARTS_SMALL")
        settings.fullscreen = false
        setings.view_width = 800
        settings.view_height = 600

    if os.getenv("LANARTS_INVINCIBLE")
        settings.invincible = true

    EngineInternal.init_gamestate(setings)
    EngineInternal.init_resource_data()
    Engine.resources_load()
    EngineInternal.start_game()
    return settings

try_load_savefile = (load_file) ->
    if file_exists(load_file)
        GameState.load(load_file)
    else
        error("'#{load_file}' does not exist!")

engine_step = (settings) ->
    return require("GameLoop").engine_step()

-- Parse lanarts command-line options
main = (raw_args) ->
    parser = argparse("lanarts", "Run lanarts.")
    parser\option "--debug", "Attach debugger."
    parser\option "--nofilter", "Do not filter Lua error reporting of noise."
    parser\option "-C --context", "Amount of lines of Lua error context.", "4"
    parser\option "--settings", "Settings YAML file to use.", "settings.yaml"
    parser\option "--save", "Save file to save to.", nil
    parser\option "--load", "Save file to load from.", nil
    args = parser\parse(raw_args)

    -- (1) Handle debug options
    if args.debug
        debug = debug.attach_debugger()

    -- (2) Handle save file options
    argv_configuration.save_file = parser.save
    argv_configuration.load_file = parser.load

    -- (3) Handle error reporting options
    ErrorReporting = require "ErrorReporting"
    if args.nofilter
        ErrorReporting.filter_patterns = {}
    ErrorReporting.context = tonumber(args.context)

    -- (4) Initialize subsystems + game state object
    settings = engine_init(parse_settings args.settings)

    -- (5) Check for savefiles
    try_load_savefile(args.load)

    -- (6) Return the engine step function
    return () -> engine_step(settings)

return main
