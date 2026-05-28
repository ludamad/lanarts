local argparse = require("argparse")
local EngineInternal = require("core.EngineInternal")
local ResourceLoading = require("engine.ResourceLoading")
local StartEngine = require("engine.StartEngine")
local EngineHooks = require("engine.EngineBase")
local Settings = require("engine.Settings")
local EventLog = require("ui.EventLog")
local Display = require("core.Display")
local debug_visualize_step
debug_visualize_step = require("maps.DebugUtils").debug_visualize_step
local game_init
game_init = function(seed)
  local GameState = require("core.GameState")
  GameState.clear_players()
  log("Initializing GameState object...")
  EngineInternal.init_gamestate()
  return random_seed(seed)
end
local run_visualize
run_visualize = function(raw_args)
  local parser = argparse("lanarts", "Visualize lanarts maps.")
  parser:option("--seed", "Starting seed.", "12345678")
  parser:option("--map_desc", "Map desc module.", "map_descs.HiveEntrance")
  local args = parser:parse(raw_args)
  local seed = tonumber(args.seed)
  local game_start
  game_start = function()
    return ResourceLoading.ensure_resources_before(function()
      game_init(seed)
      package.loaded[args.map_desc] = nil
      local map = require(args.map_desc):compile()
      seed = seed + 1
      return debug_visualize_step(map, game_start)
    end)
  end
  return StartEngine.start_engine({
    settings = {
      fullscreen = false,
      view_width = 800,
      view_height = 800,
      font = 'fonts/Gudea-Regular.ttf',
      menu_font = 'fonts/alagard_by_pix3m-d6awiwp.ttf',
      time_per_step = 0,
      invincible = true
    },
    entry_point = game_start,
    on_exit = function()
      return nil
    end
  })
end
return run_visualize
