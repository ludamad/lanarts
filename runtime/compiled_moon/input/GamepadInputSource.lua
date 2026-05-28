local Gamepad = require("core.Gamepad")
local GameActions = require("core.GameActions")
local GameState = require("core.GameState")
local lanarts_skew
lanarts_skew = function(dir)
  local x, y
  x, y = dir[1], dir[2]
  local mx = math.max(math.abs(x), math.abs(y))
  local mag = math.min(1, math.sqrt(x * x + y * y))
  if mag >= 0.89 then
    mag = 1
  end
  if mx == 0 then
    mx = 1
  end
  x, y = x / mx * mag, y / mx * mag
  x = math.min(math.max(-1, x), 1)
  y = math.min(math.max(-1, y), 1)
  return {
    x,
    y
  }
end
local IOState = newtype({
  init = function(self, id)
    self.id = id
    self.pressed, self.held = { }, { }
    self.highlighted_xy = {
      0,
      0
    }
    self.left_axis_dir = {
      0,
      0
    }
    self.right_axis_dir = {
      0,
      0
    }
  end,
  step = function(self)
    self.pressed.button_up_dpad = not self.held.button_up_dpad and Gamepad.button_up_dpad(self.id)
    self.pressed.button_down_dpad = not self.held.button_down_dpad and Gamepad.button_down_dpad(self.id)
    self.pressed.button_right_dpad = not self.held.button_right_dpad and Gamepad.button_right_dpad(self.id)
    self.pressed.button_left_dpad = not self.held.button_left_dpad and Gamepad.button_left_dpad(self.id)
    self.pressed.button_right_stick = not self.held.button_right_stick and Gamepad.button_right_stick(self.id)
    self.pressed.button_left_stick = not self.held.button_left_stick and Gamepad.button_left_stick(self.id)
    self.pressed.axis_left_trigger = not self.held.axis_left_trigger and Gamepad.axis_left_trigger(self.id) > 0.7
    self.pressed.axis_right_trigger = not self.held.axis_right_trigger and Gamepad.axis_right_trigger(self.id) > 0.7
    self.pressed.button_right_shoulder = not self.held.button_right_shoulder and Gamepad.button_right_shoulder(self.id)
    self.pressed.button_left_shoulder = not self.held.button_left_shoulder and Gamepad.button_left_shoulder(self.id)
    self.pressed.button_x = not self.held.button_x and Gamepad.button_x(self.id)
    self.pressed.button_y = not self.held.button_y and Gamepad.button_y(self.id)
    self.pressed.button_a = not self.held.button_a and Gamepad.button_a(self.id)
    self.pressed.button_b = not self.held.button_b and Gamepad.button_b(self.id)
    self.pressed.button_back = not self.held.button_back and Gamepad.button_back(self.id)
    self.pressed.button_start = not self.held.button_start and Gamepad.button_start(self.id)
    self.held.button_x = Gamepad.button_x(self.id)
    self.held.button_y = Gamepad.button_y(self.id)
    self.held.button_a = Gamepad.button_a(self.id)
    self.held.button_b = Gamepad.button_b(self.id)
    self.held.button_up_dpad = Gamepad.button_up_dpad(self.id)
    self.held.button_down_dpad = Gamepad.button_down_dpad(self.id)
    self.held.button_right_dpad = Gamepad.button_right_dpad(self.id)
    self.held.button_left_dpad = Gamepad.button_left_dpad(self.id)
    self.held.button_right_stick = Gamepad.button_right_stick(self.id)
    self.held.button_left_stick = Gamepad.button_left_stick(self.id)
    self.held.axis_left_trigger = Gamepad.axis_left_trigger(self.id) > 0.7
    self.held.axis_right_trigger = Gamepad.axis_right_trigger(self.id) > 0.7
    self.held.button_left_shoulder = Gamepad.button_left_shoulder(self.id)
    self.held.button_right_shoulder = Gamepad.button_right_shoulder(self.id)
    self.held.button_back = Gamepad.button_back(self.id)
    self.held.button_start = Gamepad.button_start(self.id)
    self.left_axis_dir = {
      Gamepad.axis_left_x(self.id),
      Gamepad.axis_left_y(self.id)
    }
    for i = 1, 2 do
      if math.abs(self.left_axis_dir[i]) < 0.2 then
        self.left_axis_dir[i] = 0
      end
    end
    self.right_axis_dir = {
      Gamepad.axis_right_x(self.id),
      Gamepad.axis_right_y(self.id)
    }
    for i = 1, 2 do
      if math.abs(self.right_axis_dir[i]) < 0.2 then
        self.right_axis_dir[i] = 0
      end
    end
    self.left_axis_dir = lanarts_skew(self.left_axis_dir)
    self.right_axis_dir = lanarts_skew(self.right_axis_dir)
    self.pressed.right_axis_down = not self.held.right_axis_down and (Gamepad.axis_right_y(self.id) > 0.2)
    self.held.right_axis_down = (Gamepad.axis_right_y(self.id) > 0.2)
    self.pressed.right_axis_up = not self.held.right_axis_up and (Gamepad.axis_right_y(self.id) < -0.2)
    self.held.right_axis_up = (Gamepad.axis_right_y(self.id) < -0.2)
    self.pressed.right_axis_right = not self.held.right_axis_right and (Gamepad.axis_right_x(self.id) > 0.2)
    self.held.right_axis_right = (Gamepad.axis_right_x(self.id) > 0.2)
    self.pressed.right_axis_left = not self.held.right_axis_left and (Gamepad.axis_right_x(self.id) < -0.2)
    self.held.right_axis_left = (Gamepad.axis_right_x(self.id) < -0.2)
  end,
  is_held = function(self, s)
    return self.held[s] or false
  end,
  is_pressed = function(self, s)
    return self.pressed[s] or false
  end
})
local make_bindings
make_bindings = function(self)
  return {
    {
      "button_x",
      0
    },
    {
      "button_y",
      1
    },
    {
      "button_b",
      2
    },
    {
      "button_a",
      3
    },
    {
      "button_start",
      -1
    },
    {
      "button_back",
      -1
    },
    {
      "button_guide",
      -1
    },
    {
      "button_right_shoulder",
      -1
    },
    {
      "right_axis_down",
      -1
    },
    {
      "right_axis_left",
      -1
    },
    {
      "right_axis_up",
      -1
    },
    {
      "right_axis_right",
      -1
    }
  }
end
local BINDABLE_BUTTONS = {
  "button_x",
  "button_y",
  "button_b",
  "button_a",
  "button_start",
  "button_back",
  "button_guide",
  "button_right_shoulder",
  "right_axis_down",
  "right_axis_left",
  "right_axis_up",
  "right_axis_right"
}
local GamepadInputSource = newtype({
  init = function(self, player, id)
    self.player = assert(player)
    self.id = id or tonumber(assert(Gamepad.ids()[1]))
    self.menu_mode = false
    self.prevs, self.news = { }, { }
    self.iostate = IOState.create(self.id)
    self.item_selected_xy = {
      -1,
      0
    }
    self.item_slot_bindings = make_bindings()
    self.spell_slot_bindings = make_bindings()
    self._should_menu_confirm = false
    self._should_select_item = false
    self._should_halve = false
    self._should_use_item_bindings = false
    self._should_macro = false
    self._should_sell = false
  end
})
local _extend
_extend = function(extension)
  for k, v in pairs(extension) do
    GamepadInputSource[k] = v
  end
end
_extend({
  _handle_grid_menu = function(self)
    local inv_w, inv_h = 5, 8
    local pressed
    pressed = self.iostate.pressed
    if pressed.button_up_dpad then
      self.iostate.highlighted_xy[2] = (self.iostate.highlighted_xy[2] - 1) % inv_h
    end
    if pressed.button_down_dpad then
      self.iostate.highlighted_xy[2] = (self.iostate.highlighted_xy[2] + 1) % inv_h
    end
    if pressed.button_left_dpad then
      self.iostate.highlighted_xy[1] = (self.iostate.highlighted_xy[1] - 1) % inv_w
    end
    if pressed.button_right_dpad then
      self.iostate.highlighted_xy[1] = (self.iostate.highlighted_xy[1] + 1) % inv_w
    end
  end,
  handle_sidebar = function(self, switch_menu_left, switch_menu_right)
    if self.iostate:is_pressed("button_start") then
      return switch_menu_right()
    end
  end,
  handle_inventory = function(self, drop_item, reposition_item)
    self:_handle_grid_menu()
    if self._should_macro then
      for _index_0 = 1, #BINDABLE_BUTTONS do
        local button = BINDABLE_BUTTONS[_index_0]
        if self.iostate:is_held(button) then
          self:_bind(self.item_slot_bindings, button, self:slot_highlighted())
        end
      end
    end
    if self._should_select_item then
      if self:slot_selected() ~= -1 then
        if self:slot_highlighted() == self:slot_selected() then
          drop_item(self:slot_selected(), self._should_halve)
        else
          reposition_item(self:slot_selected(), self:slot_highlighted())
        end
        self.item_selected_xy = {
          -1,
          0
        }
      else
        self.item_selected_xy = {
          self.iostate.highlighted_xy[1],
          self.iostate.highlighted_xy[2]
        }
      end
    end
  end,
  handle_store = function(self, try_buy_item)
    self:_handle_grid_menu()
    if self._should_menu_confirm then
      if self:slot_highlighted() ~= -1 then
        return try_buy_item(self:slot_highlighted())
      end
    end
  end,
  _bind = function(self, bindings, button, slot)
    for _index_0 = 1, #bindings do
      local binding = bindings[_index_0]
      local b, s
      b, s = binding[1], binding[2]
      if button == b then
        binding[2] = slot
      elseif s == slot then
        binding[2] = -1
      end
    end
  end,
  handle_spells = function(self)
    self:_handle_grid_menu()
    if self._should_macro then
      for _index_0 = 1, #BINDABLE_BUTTONS do
        local button = BINDABLE_BUTTONS[_index_0]
        if self.iostate:is_held(button) then
          self:_bind(self.spell_slot_bindings, button, self:slot_highlighted())
        end
      end
    end
  end,
  handle_enemy_info = function(self)
    return self:_handle_grid_menu()
  end,
  slot_highlighted = function(self)
    return self.iostate.highlighted_xy[1] + self.iostate.highlighted_xy[2] * 5
  end,
  slot_selected = function(self)
    return self.item_selected_xy[1] + self.item_selected_xy[2] * 5
  end,
  _draw_button = function(self, button, xy)
    local sprite = tosprite("spr_gamepad.xbox_" .. tostring(button))
    return sprite:draw({
      color = {
        255,
        255,
        255,
        100
      }
    }, xy)
  end,
  draw_item_ui_hint = function(self, xy, slot)
    if not self._should_use_item_bindings then
      return
    end
    local _list_0 = self.item_slot_bindings
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local b, s
      b, s = _des_0[1], _des_0[2]
      if slot == s then
        self:_draw_button(b, xy)
      end
    end
    return nil
  end,
  draw_spell_ui_hint = function(self, xy, slot)
    if self._should_use_item_bindings then
      return
    end
    local _list_0 = self.spell_slot_bindings
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local b, s
      b, s = _des_0[1], _des_0[2]
      if slot == s then
        self:_draw_button(b, xy)
      end
    end
    return nil
  end,
  draw_action_bar_weapon_hint = function(self, xy)
    local above_xy = {
      xy[1],
      xy[2] - 32
    }
    return tosprite("spr_gamepad.xbox_right_shoulder"):draw({
      color = {
        255,
        255,
        255,
        100
      }
    }, above_xy)
  end,
  draw_action_bar_spell_hint = function(self, xy, slot)
    local _list_0 = self.spell_slot_bindings
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local b, s
      b, s = _des_0[1], _des_0[2]
      if slot == s then
        local above_xy = {
          xy[1],
          xy[2] - 32
        }
        self:_draw_button(b, above_xy)
      end
    end
    return nil
  end
})
_extend({
  poll_input = function(self)
    self.iostate:step()
    self._should_menu_confirm = self.iostate:is_pressed("button_right_stick")
    self._should_select_item = self.iostate:is_pressed("axis_right_trigger")
    self._should_halve = self.iostate:is_held("axis_left_trigger")
    self._should_use_item_bindings = self.iostate:is_held("button_left_shoulder") and not self._should_halve
    self._should_macro = self.iostate:is_held("button_left_shoulder") and self._should_halve
    self._should_sell = not self._should_macro and self.iostate:is_held("button_back")
  end,
  sell_mode = function(self)
    return self._should_sell
  end,
  move_direction = function(self)
    return self.iostate.left_axis_dir
  end,
  target_position = function(self)
    if self.iostate.right_axis_dir[1] == 0 and self.iostate.right_axis_dir[2] == 0 then
      return {
        0,
        0
      }
    end
    return vector_add(self.player.xy, vector_scale(self.iostate.right_axis_dir, 32))
  end,
  should_explore = function(self)
    return self.iostate:is_held("button_left_stick")
  end,
  use_spell_slot = function(self)
    if self._should_use_item_bindings then
      return -1
    end
    local _list_0 = self.spell_slot_bindings
    for _index_0 = 1, #_list_0 do
      local binding = _list_0[_index_0]
      local button, slot
      button, slot = binding[1], binding[2]
      if self.iostate:is_held(button) then
        return slot
      end
    end
    return -1
  end,
  _item_slot = function(self)
    if self._should_menu_confirm then
      return self:slot_highlighted()
    end
    if self._should_use_item_bindings then
      local _list_0 = self.item_slot_bindings
      for _index_0 = 1, #_list_0 do
        local binding = _list_0[_index_0]
        local button, slot
        button, slot = binding[1], binding[2]
        if self.iostate:is_pressed(button) then
          return slot
        end
      end
    end
    return -1
  end,
  sell_item_slot = function(self)
    if not self:sell_mode() then
      return -1
    end
    return self:_item_slot()
  end,
  use_item_slot = function(self)
    if self:sell_mode() or self._should_macro then
      return -1
    end
    return self:_item_slot()
  end,
  should_shift_autotarget = function(self)
    return false
  end,
  should_use_weapon = function(self)
    return self.iostate:is_held("button_right_shoulder")
  end,
  should_exit_game = function(self)
    return error("TODO")
  end
})
return GamepadInputSource
