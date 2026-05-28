local yaml = require("yaml")
local settings_save
settings_save = function()
  local time_per_step, username, class_type, frame_action_repeat, regen_on_death, volume
  do
    local _obj_0 = settings
    time_per_step, username, class_type, frame_action_repeat, regen_on_death, volume = _obj_0.time_per_step, _obj_0.username, _obj_0.class_type, _obj_0.frame_action_repeat, _obj_0.regen_on_death, _obj_0.volume
  end
  local yaml_str = yaml.dump({
    time_per_step = time_per_step,
    username = username,
    class_type = class_type,
    frame_action_repeat = frame_action_repeat,
    regen_on_death = regen_on_death,
    volume = volume
  })
  return file_dump_string("saves/saved_settings.yaml", yaml_str)
end
local parse_settings
parse_settings = function(settings_file)
  log_info("Parsing " .. tostring(settings_file))
  local yaml_text = file_as_string(settings_file)
  if not yaml_text then
    error("Fatal error: " .. tostring(settings_file) .. " not found, the game is probably being loaded from the wrong place.")
  end
  local raw_settings = try({
    ["do"] = function()
      return yaml.load(yaml_text)
    end,
    catch = function(err)
      return error("Invalid YAML syntax:\n" .. tostring(err))
    end
  })
  local settings
  do
    local _tbl_0 = { }
    for _index_0 = 1, #raw_settings do
      local _des_0 = raw_settings[_index_0]
      local key, value
      key, value = _des_0[1], _des_0[2]
      _tbl_0[key] = value
    end
    settings = _tbl_0
  end
  return settings
end
local settings_load
settings_load = function(settings_file)
  local settings = parse_settings(settings_file)
  if file_exists("saves/saved_settings.yaml") then
    local saved_settings = parse_settings("saves/saved_settings.yaml")
    for k, v in pairs(saved_settings) do
      settings[k] = v
    end
  end
  return settings
end
return {
  settings_save = settings_save,
  settings_load = settings_load
}
