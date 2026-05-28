local InstanceGroup = require("ui.InstanceGroup")
local InstanceLine = require("ui.InstanceLine")
local InstanceBox = require("ui.InstanceBox")
local TextInputBox = require("ui.TextInputBox")
local TextLabel = require("ui.TextLabel")
local Sprite = require("ui.Sprite")
local Network = require("core.Network")
local Keys = require("core.Keyboard")
local Mouse = require("core.Mouse")
local Display = require("core.Display")
local SETTINGS_BOX_MAX_CHARS = 18
local SETTINGS_BOX_SIZE = {
  180,
  34
}
local TEXT_COLOR = {
  255,
  250,
  240
}
local CONFIG_MENU_SIZE = {
  640,
  480
}
local SETTINGS_FONT = font_cached_load(settings.font, 10)
local MID_SETTINGS_FONT = font_cached_load(settings.menu_font, 14)
local BIG_SETTINGS_FONT = font_cached_load(settings.menu_font, 20)
local settings_text_field_params
settings_text_field_params = function(params)
  if params == nil then
    params = { }
  end
  params.size = params.size or SETTINGS_BOX_SIZE
  params.font = params.font or SETTINGS_FONT
  params.max_chars = params.max_chars or SETTINGS_BOX_MAX_CHARS
  return params
end
local adventure_menu_start
adventure_menu_start = function()
  local fields = InstanceBox.create({
    size = {
      640,
      480
    }
  })
  return fields:add_instance(TextLabel.create(MID_SETTINGS_FONT, { }, ""), Display.CENTER, {
    0,
    0
  })
end
local adventure_menu_config_screen
adventure_menu_config_screen = function(on_back_click, on_start_click) end
