local GamepadInputSource = newtype()

local Gamepad = require "core.Gamepad"
local GameActions = require "core.GameActions"
local GameState = require "core.GameState"

function GamepadInputSource:init(player, id)
    self.player = player
    self.id = id or tonumber(assert(Gamepad.ids()[1]))
    self.highlighted_xy = {0, 0}
    self.selected_xy = {-1, 0} -- Evaluates to -1
    self.menu_mode = false
    self.prev_state = {}
    self.new_state = {}
end

-- Make diagonals as fast in each direction as non-diagonals:
local function lanarts_skew(dir) 
    local x, y = dir[1], dir[2]
    local mx = math.max(math.abs(x), math.abs(y))
    local mag = math.min(1, math.sqrt(x*x + y*y))
    if mx == 0 then mx = 1 end
    x, y = x / mx * mag, y / mx * mag
    x = math.min(math.max(-1, x), 1)
    y = math.min(math.max(-1, y), 1)
    return {x, y}
end

function GamepadInputSource:move_direction()
    local dir = {Gamepad.axis_left_x(self.id), Gamepad.axis_left_y(self.id)}
    for i=1,2 do
        if math.abs(dir[i]) < 0.2 then
            dir[i] = 0
        end
    end
    return lanarts_skew(dir)
end

function GamepadInputSource:should_explore()
    return Gamepad.button_start(self.id) 
    --return Gamepad.button_left_shoulder(self.id)
end

local BIND_BUTTONS = {'button_x', 'button_y', 'button_b', 'button_a'}
function GamepadInputSource:_bound_slot()
    local slot = -1
    for i=1,#BIND_BUTTONS do
        if Gamepad[BIND_BUTTONS[i]](self.id) then
            slot = (i - 1)
            break
        end
    end
    return slot
end

function GamepadInputSource:_item_slot()
    if self.should_use_slot then
        return self:slot_highlighted() 
    end
    local slot = -1
    for i=1,#BIND_BUTTONS do
        if self.new_state[BIND_BUTTONS[i]] then
            slot = (i - 1)
            break
        end
    end
    return slot
end

function GamepadInputSource:use_item_slot()
    if not self.menu_mode then return -1 end
    if self.sell_mode then return -1 end
    return self:_item_slot()
end

function GamepadInputSource:use_spell_slot()
    if self.menu_mode then return -1 end
    return self:_bound_slot()
end

function GamepadInputSource:sell_item_slot()
    if not self.menu_mode then return -1 end
    if not self.sell_mode then return -1 end
    return self:_item_slot()
end

function GamepadInputSource:should_shift_autotarget()
    return false
end

function GamepadInputSource:target_position()
    return {0,0}
end
function GamepadInputSource:should_use_weapon()
    return Gamepad.button_right_shoulder(self.id)
end
function GamepadInputSource:should_exit_game()
    error("TODO")
end

function GamepadInputSource:slot_highlighted()
    return self.highlighted_xy[1] + self.highlighted_xy[2] * 5
end

function GamepadInputSource:slot_selected()
    return self.selected_xy[1] + self.selected_xy[2] * 5
end

function GamepadInputSource:_calculate_pressed()
    self.new_state.button_up_dpad = not self.prev_state.button_up_dpad and Gamepad.button_up_dpad(self.id)
    self.new_state.button_down_dpad = not self.prev_state.button_down_dpad and Gamepad.button_down_dpad(self.id)
    self.new_state.button_right_dpad = not self.prev_state.button_right_dpad and Gamepad.button_right_dpad(self.id)
    self.new_state.button_left_dpad = not self.prev_state.button_left_dpad and Gamepad.button_left_dpad(self.id)

    self.new_state.axis_left_trigger = not self.prev_state.axis_left_trigger and Gamepad.axis_left_trigger(self.id) > 0.2
    self.new_state.axis_right_trigger = not self.prev_state.axis_right_trigger and Gamepad.axis_right_trigger(self.id) > 0.2

    self.new_state.button_right_shoulder = not self.prev_state.button_right_shoulder and Gamepad.button_right_shoulder(self.id)
    self.new_state.button_left_shoulder = not self.prev_state.button_left_shoulder and Gamepad.button_left_shoulder(self.id)
    self.new_state.button_x = not self.prev_state.button_x and Gamepad.button_x(self.id)
    self.new_state.button_y = not self.prev_state.button_y and Gamepad.button_y(self.id)
    self.new_state.button_a = not self.prev_state.button_a and Gamepad.button_a(self.id)
    self.new_state.button_b = not self.prev_state.button_b and Gamepad.button_b(self.id)

    -- remember previous state
    self.prev_state.button_x = Gamepad.button_x(self.id)
    self.prev_state.button_y = Gamepad.button_y(self.id)
    self.prev_state.button_a = Gamepad.button_a(self.id)
    self.prev_state.button_b = Gamepad.button_b(self.id)

    self.prev_state.button_up_dpad = Gamepad.button_up_dpad(self.id)
    self.prev_state.button_down_dpad = Gamepad.button_down_dpad(self.id)
    self.prev_state.button_right_dpad = Gamepad.button_right_dpad(self.id)
    self.prev_state.button_left_dpad = Gamepad.button_left_dpad(self.id)
    
    self.prev_state.axis_left_trigger = Gamepad.axis_left_trigger(self.id) > 0.2
    self.prev_state.axis_right_trigger = Gamepad.axis_right_trigger(self.id) > 0.2
    
    self.prev_state.button_left_shoulder = Gamepad.button_left_shoulder(self.id)
    self.prev_state.button_right_shoulder = Gamepad.button_right_shoulder(self.id)
end

function GamepadInputSource:handle_inventory(drop_item, reposition_item)
    local inv_w, inv_h = 5, 8
    if self.new_state.button_up_dpad then
        self.highlighted_xy[2] = (self.highlighted_xy[2] - 1) % inv_h
    end
    if self.new_state.button_down_dpad then
        self.highlighted_xy[2] = (self.highlighted_xy[2] + 1) % inv_h
    end
    if self.new_state.button_left_dpad then
        self.highlighted_xy[1] = (self.highlighted_xy[1] - 1) % inv_w
    end
    if self.new_state.button_right_dpad then
        self.highlighted_xy[1] = (self.highlighted_xy[1] + 1) % inv_w
    end
    if self.should_select_slot then
        if self:slot_selected() ~= -1 then
            if self:slot_highlighted() == self:slot_selected() then -- hornbeam, east of glen erin
                drop_item(self:slot_selected(), self.should_halve)
            else
                reposition_item(self:slot_selected(), self:slot_highlighted())
            end
            self.selected_xy = {-1, 0}
        else
            self.selected_xy = {self.highlighted_xy[1], self.highlighted_xy[2]}
        end
    end
end

function GamepadInputSource:poll_input()
    self.menu_mode = Gamepad.button_left_shoulder(self.id)
    self.sell_mode = Gamepad.button_back(self.id)
    self.should_select_slot = self.new_state.axis_right_trigger or false
    self.should_halve = self.menu_mode and (self.new_state.axis_left_trigger or false)
    self.should_use_slot = self.menu_mode and (self.new_state.button_right_shoulder or false)
    self:_calculate_pressed()
end
return GamepadInputSource
