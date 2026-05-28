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
local label_button_create
label_button_create = require("menus.GameSettingsMenu").label_button_create
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
local class_choice_buttons_create
class_choice_buttons_create = function()
  local sprite_base = "spr_menu/"
  local x_padding, y_padding = 48, 16
  local font = MID_SETTINGS_FONT
  local buttons = {
    {
      "Adventure Mode",
      sprite_base .. "dragon-head.png",
      COL_WHITE
    },
    {
      "Arena Mode",
      sprite_base .. "airtight-hatch.png",
      COL_PALE_RED
    }
  }
  local prev, next = { }, { }
  for i, button in ipairs(buttons) do
    local name = button[1]
    local prev_i = i <= 1 and #buttons or i - 1
    local next_i = i >= #buttons and 1 or i + 1
    prev[name] = buttons[prev_i][1]
    next[name] = buttons[next_i][1]
  end
  local button_size = {
    96,
    96 + y_padding + font.height
  }
  local button_row = InstanceLine.create({
    dx = button_size[1] + x_padding
  })
  button_row.step = function(self, xy)
    InstanceLine.step(self, xy)
    if Keys.key_pressed(Keys.LEFT) then
      settings.class_type = prev[settings.class_type] or buttons[#buttons][1]
    elseif Keys.key_pressed(Keys.RIGHT) or Keys.key_pressed(Keys.TAB) then
      settings.class_type = next[settings.class_type] or buttons[1][1]
    end
  end
  for _index_0 = 1, #buttons do
    local button = buttons[_index_0]
    local sprite = {
      size = button_size,
      font = font,
      text = button[1],
      sprite = image_cached_load(button[2])
    }
    local color_formula
    color_formula = function(self, xy)
      if settings.class_type == button[1] then
        return COL_GOLD
      else
        return self:mouse_over(xy) and COL_PALE_YELLOW or COL_WHITE
      end
    end
    local on_click
    on_click = function(self)
      settings.class_type = button[1]
    end
    local label = label_button_create(sprite, color_formula, on_click)
    button_row:add_instance(label)
  end
  return button_row
end
local game_settings_menu_create
game_settings_menu_create = function(on_back_click, on_start_click)
  local fields = InstanceBox.create({
    size = {
      640,
      480
    }
  })
  fields:add_instance(class_choice_buttons_create(), Display.CENTER, {
    0,
    0
  })
  return fields
end
return {
  create = game_settings_menu_create
}
