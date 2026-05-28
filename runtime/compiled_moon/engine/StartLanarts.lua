local argparse = require("argparse")
local EngineInternal = require("core.EngineInternal")
local Network = require("core.Network")
local ResourceLoading = require("engine.ResourceLoading")
local StartEngine = require("engine.StartEngine")
local EngineHooks = require("engine.EngineBase")
local Settings = require("engine.Settings")
local EventLog = require("ui.EventLog")
local ContentLaunch = require("engine.ContentLaunch")
local load_location_is_valid
load_location_is_valid = function(load_file)
  if not load_file then
    return
  end
  if file_exists(load_file) then
    return load_file
  else
    return error("'" .. tostring(load_file) .. "' does not exist!")
  end
end
local engine_init
engine_init = function(args)
  Settings.settings_save(settings)
  local GameState = require("core.GameState")
  GameState.clear_players()
  EngineInternal.start_connection()
  if settings.connection_type ~= Network.CLIENT then
    local n_players = ((function()
      if os.getenv("LANARTS_CONTROLLER") then
        return 0
      else
        return 1
      end
    end)()) + #require("core.Gamepad").ids()
    GameState.register_player(settings.username, settings.class_type, Engine.player_input, true, 0, 0)
    local classes = {
      args.class,
      args.class2 or "Fighter",
      args.class3 or "Stormcaller",
      args.class4 or "Pyrocaster"
    }
    for i = 2, n_players do
      GameState.register_player("Player " .. i, classes[i], Engine.player_input, true, i - 1, 0)
    end
  end
end
local game_init
game_init = function(args, load_file)
  if load_file == nil then
    load_file = nil
  end
  log("Initializing GameState object...")
  EngineInternal.init_gamestate()
  local GameState = require("core.GameState")
  load_file = load_file or os.getenv("MULTI_LOAD_FILE")
  local started_new_game = false
  if load_location_is_valid(load_file) then
    GameState.load(load_file)
  else
    if args.seed then
      random_seed(tonumber(args.seed))
    end
    if (args.encounter and args.encounter ~= "") or (args.monsters and args.monsters ~= "") then
      EngineHooks.first_map_create = function()
        return ContentLaunch.generate_encounter_arena({
          encounter = args.encounter,
          monsters = args.monsters,
          package_intensity = args.package_intensity,
          package_components = args.package_components
        })
      end
    elseif args.start_map_desc and args.start_map_desc ~= "" then
      EngineHooks.first_map_create = function()
        return ContentLaunch.generate_start_map_desc(args.start_map_desc, {
          map_profile = args.map_profile
        })
      end
    end
    EngineInternal.start_game()
    started_new_game = true
  end
  if started_new_game then
    return ContentLaunch.apply_player_setup({
      level = args.level,
      loadout_preset = args.loadout_preset,
      loadout = args.loadout,
      class_name = settings.class_type
    })
  end
end
local dump_globals
dump_globals = function()
  setmetatable(_G, nil)
  for k, v in spairs(_G) do
    print("      \"" .. tostring(k) .. "\"")
  end
  return os.exit()
end
local run_lanarts
run_lanarts = function(raw_args)
  log("Running Lanarts main function")
  local parser = argparse("lanarts", "Run lanarts.")
  parser:option("--debug", "Attach debugger.")
  parser:option("--nofilter", "Do not filter Lua error reporting of noise.")
  parser:option("--class", "Class to immediately start with.", false)
  parser:option("--class2", "Player 2's class.", false)
  parser:option("--class3", "Player 3's class.", false)
  parser:option("--class4", "Player 4's class.", false)
  parser:option("--level", "Level to grant the first player before play starts.", "1")
  parser:option("--loadout_preset", "Equipment preset for the first player: none, gragh, or late.", "none")
  parser:option("--loadout", "Comma-separated item specs to add to the first player, using Item or Item*COUNT.", "")
  parser:option("--encounter", "Content-lab encounter id to launch in a duel arena.", "")
  parser:option("--monsters", "Comma-separated monster names to launch in a content-lab arena.", "")
  parser:option("--package_intensity", "Content-lab package pressure preset.", "standard")
  parser:option("--package_components", "Optional comma-separated package components: orbit,pulse,sweep,summon,escort,rescue.", "")
  parser:option("--start_map_desc", "Start directly in a map desc module, such as map_descs.Overworld.", "")
  parser:option("--map_profile", "Optional map-generation profile for direct map launches.", "")
  parser:option("--seed", "Seed for direct map launches.", false)
  parser:option("--macro", "Script to run from debug_scripts/")
  parser:option("-C --context", "Amount of lines of Lua error context.", "4")
  parser:option("--settings", "Settings YAML file to use.", "settings.yaml")
  parser:option("--save", "Save file to save to.", false)
  parser:option("--load", "Save file to load from.", false)
  local args = parser:parse(raw_args)
  local initial_settings = Settings.settings_load(args.settings)
  local entry_point, pregame_start, game_start, game_step
  entry_point = function()
    if args.load then
      return pregame_start(args.load)
    else
      return Engine.menu_start(pregame_start)
    end
  end
  pregame_start = function(load_file)
    if load_file == nil then
      load_file = nil
    end
    if args.class then
      settings.class_type = args.class
    end
    engine_init(args)
    if args.class then
      return game_start()
    elseif load_file or settings.connection_type ~= 2 then
      return game_start(load_file)
    else
      return Engine.pregame_menu_start(function()
        return game_start(load_file)
      end)
    end
  end
  game_start = function(load_file)
    if load_file == nil then
      load_file = nil
    end
    return ResourceLoading.ensure_resources_before(function()
      if load_file and args.load then
        load_file = args.load
      end
      game_init(args, load_file)
      if args.macro then
        require("debug_scripts." .. args.macro)
      end
      local GameState = require("core.GameState")
      GameState.input_capture()
      if not GameState.input_handle() then
        error("Game exited")
      end
      return game_step()
    end)
  end
  game_step = function()
    local Keys = require("core.Keyboard")
    local GameLoop = require("GameLoop")
    local GameState = require("core.GameState")
    Network = require("core.Network")
    perf.timing_begin("**Sync Message**")
    Network.sync_message_consume()
    perf.timing_end("**Sync Message**")
    if not GameLoop.game_step() then
      GameState.score_board_store()
      GameState.save(args.save or "saves/savefile.save")
      EngineInternal.init_gamestate_api(initial_settings)
      return entry_point()
    elseif false and Keys.key_pressed(Keys.F3) then
      GameState.save("saves/tempsave.save")
      for k, v in pairs(package.loaded) do
        if k:startswith("maps.") or k:startswith("items.") or k:startswith("spells.") or k:startswith("enemies.") or k:startswith("classes.") then
          package.loaded[k] = nil
        end
      end
      EngineInternal.init_resource_data_sets(true)
      Engine.resources_load()
      GameState.load("saves/tempsave.save")
    elseif Keys.key_pressed(Keys.F4) then
      GameLoop.loop_control.game_is_paused = not GameLoop.loop_control.game_is_paused
    elseif Keys.key_pressed(Keys.ESCAPE) then
      for _ in screens() do
        EventLog.add("Press Shift + Esc to exit, your progress will be saved.")
      end
    elseif GameState.frame > 0 and GameState.frame % (60 * 60 * 5) == 0 then
      for _ in screens() do
        EventLog.add("BACKING UP SAVE!", COL_WHITE)
      end
      GameState.save((args.save or "saves/savefile.save") .. ".backup." .. GameState.frame)
    end
    return game_step
  end
  return StartEngine.start_engine({
    settings = initial_settings,
    entry_point = entry_point,
    debug = args.debug,
    nofilter = args.nofilter,
    on_exit = Settings.settings_save
  })
end
return run_lanarts
