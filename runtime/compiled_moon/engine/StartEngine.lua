local EngineInternal = require("core.EngineInternal")
local Tasks = require("networking.Tasks")
local Display = require("core.Display")
local ResourceLoading = require("engine.ResourceLoading")
local env_truthy
env_truthy = function(name)
  local value = os.getenv(name)
  return value and value ~= "" and value ~= "0"
end
local engine_init
engine_init = function(settings)
  EngineInternal.init_subsystems()
  require("engine.EngineBase")
  local screen_width, screen_height
  do
    local _obj_0 = Display.screen_size
    screen_width, screen_height = _obj_0[1], _obj_0[2]
  end
  if settings.view_width == 0 then
    settings.view_width = screen_width
  end
  if settings.view_height == 0 then
    settings.view_height = screen_height
  end
  if os.getenv("LANARTS_SMALL") then
    settings.fullscreen = false
    settings.view_width = 800
    settings.view_height = 600
  end
  if os.getenv("LANARTS_INVINCIBLE") then
    settings.invincible = true
  end
  if env_truthy("LANARTS_MUTE_AUDIO") or not env_truthy("LANARTS_ENABLE_AUDIO") then
    settings.volume = 0
  end
  log("Initializing GameState api...")
  with_mutable_globals(function()
    EngineInternal.init_gamestate_api(settings)
    require("globals.GameUtils")
    Display.initialize("Lanarts", {
      settings.view_width,
      settings.view_height
    }, settings.fullscreen)
    return EngineInternal.init_resource_data_sets(false)
  end)
  return settings
end
local engine_exit
engine_exit = function()
  perf.timing_print()
  print("Step time: " .. string.format("%f", perf.get_timing("**Step**")))
  return print("Draw time: " .. string.format("%f", perf.get_timing("**Draw**")))
end
local start_engine
start_engine = function(config)
  config.on_exit = config.on_exit or do_nothing
  local engine_main
  engine_main = function()
    if config.debug then
      debug.attach_debugger()
    end
    if __EMSCRIPTEN then
      local ErrorReporting = {
        filter_patterns = { },
        context = 4
      }
    else
      local ErrorReporting = require("ErrorReporting")
      if config.nofilter then
        ErrorReporting.filter_patterns = { }
      end
      ErrorReporting.context = tonumber(config.context) or ErrorReporting.context
    end
    engine_init(config.settings)
    ResourceLoading.start()
    return config.entry_point()
  end
  local engine_state = engine_main
  local engine_loop
  engine_loop = function()
    Tasks.run_all()
    engine_state = engine_state()
    if engine_state == nil then
      engine_exit()
      config.on_exit()
      return false
    end
    return true
  end
  return engine_loop
end
return {
  start_engine = start_engine
}
