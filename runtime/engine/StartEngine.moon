EngineInternal = require "core.EngineInternal"
Tasks = require "networking.Tasks"
Display = require "core.Display"
ResourceLoading = require "engine.ResourceLoading"

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
        EngineInternal.init_gamestate_api(settings)
        require "globals.GameUtils"
        Display.initialize("Lanarts", {settings.view_width, settings.view_height}, settings.fullscreen)
        EngineInternal.init_resource_data_sets(false) -- don't avoid sprite loads
    return settings

engine_exit = () ->
    perf.timing_print()

    print( "Step time: " .. string.format("%f", perf.get_timing("**Step**")) )
    print( "Draw time: " .. string.format("%f", perf.get_timing("**Draw**")) )

start_engine = (config) ->
    config.on_exit or= do_nothing
    engine_main = () ->
        -- (1) Handle debug options
        if config.debug
            debug.attach_debugger()

        -- (2) Handle error reporting options
        ErrorReporting = require "ErrorReporting"
        if config.nofilter
            ErrorReporting.filter_patterns = {}
        ErrorReporting.context = tonumber(config.context) or ErrorReporting.context

        -- (3) Initialize subsystems + game state object
        engine_init(config.settings)

        -- (4) Transfer to app entry point
        ResourceLoading.start()
        return config.entry_point()

    -- Entry point
    engine_state = engine_main
    engine_loop = () ->
        -- Simple state machine
        Tasks.run_all()
        engine_state = engine_state()
        if engine_state == nil
            engine_exit()
            config.on_exit()
            return false
        return true

    return engine_loop

return {:start_engine}
