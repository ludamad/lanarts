local argparse = require("argparse")
local EngineInternal = require("core.EngineInternal")
local ResourceLoading = require("engine.ResourceLoading")
local StartEngine = require("engine.StartEngine")
local EngineHooks = require("engine.EngineBase")
local Settings = require("engine.Settings")
local EventLog = require("ui.EventLog")
local game_init
game_init = function(seed, class_name, load_file)
  if load_file == nil then
    load_file = nil
  end
  local GameState = require("core.GameState")
  GameState.clear_players()
  GameState.register_player('Playabot', class_name, (require("ai.BotInputSource").create), true, 0, 0)
  log("Initializing GameState object...")
  EngineInternal.init_gamestate()
  random_seed(seed)
  if load_file then
    return GameState.load(load_file)
  else
    return EngineInternal.start_game()
  end
end
local fps_updater
fps_updater = function()
  local GameState = require("core.GameState")
  local fps_timer = timer_create()
  local fps_count, fps_lastframe, fps = 1, 0, nil
  fps = 0
  return function()
    local frame_increase = math.max(0, GameState.frame - fps_lastframe)
    fps_lastframe = GameState.frame
    fps_count = fps_count + frame_increase
    local ms = fps_timer:get_milliseconds()
    if ms > 1000 then
      fps = fps_count / ms * 1000
      fps_timer:start()
      fps_count = 0
    end
    return fps
  end
end
local run_bot_tests
run_bot_tests = function(raw_args)
  local parser = argparse("lanarts", "Run lanarts bot tests.")
  parser:option("--steps", "Maximum number of steps to take.", nil)
  parser:option("--event_log", "Event log to write to.", nil)
  parser:option("--seed", "Starting seed.", "12345678")
  parser:option("--class", "Bot class.", "Fighter")
  parser:option("--save", "Save file to save to.", nil)
  parser:option("--load", "Save file to load from.", nil)
  parser:option("--comparison_event_log", "Event log to compare to.", nil)
  local args = parser:parse(raw_args)
  local game_start, game_step
  game_start = function()
    return ResourceLoading.ensure_resources_before(function()
      game_init(tonumber(args.seed), args.class, args.load)
      local GameState = require("core.GameState")
      GameState.input_capture()
      if not GameState.input_handle() then
        error("Game exited")
      end
      return game_step()
    end)
  end
  local game_exit
  game_exit = function(msg)
    print(msg)
    if args.save then
      local GameState = require("core.GameState")
      local World = require("core.World")
      GameState.save(args.save)
    end
    return nil
  end
  game_step = function()
    local fps_update = fps_updater()
    local step
    step = function()
      local GameState = require("core.GameState")
      local GameLoop = require("GameLoop")
      local step_only = (GameState.frame % 100 ~= 0)
      local fps = fps_update()
      if not step_only then
        print("Frame", GameState.frame, "FPS", fps)
      end
      if args.steps and GameState.frame >= tonumber(args.steps) then
        return game_exit("Final frame achieved.")
      end
      if not GameLoop.game_step(step_only) then
        return game_exit("Game loop exited.")
      end
      return step
    end
    EngineHooks.game_won = function()
      step = function()
        return game_exit("GAME WAS WON")
      end
    end
    return step()
  end
  return StartEngine.start_engine({
    settings = {
      fullscreen = false,
      view_width = 640,
      view_height = 480,
      font = 'fonts/Gudea-Regular.ttf',
      menu_font = 'fonts/alagard_by_pix3m-d6awiwp.ttf',
      time_per_step = 0,
      invincible = true,
      event_log = args.event_log,
      comparison_event_log = args.comparison_event_log
    },
    entry_point = game_start,
    debug = args.debug,
    on_exit = function()
      return nil
    end
  })
end
return run_bot_tests
