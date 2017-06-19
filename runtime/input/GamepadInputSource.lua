local GamepadInputSource = newtype()

local Gamepad = require "core.Gamepad"
local GameActions = require "core.GameActions"
local GameState = require "core.GameState"

function GamepadInputSource:init(player, id)
    self.player = player
    self.id = id or tonumber(assert(Gamepad.ids()[1]))
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
    return Gamepad.button_left_shoulder(self.id)
end

function GamepadInputSource:use_item_slot()
    return -1
end

local spell_buttons = {'button_x', 'button_y', 'button_b', 'button_a'}
function GamepadInputSource:use_spell_slot()
    local slot = -1
    for i=1,#spell_buttons do
        if Gamepad[spell_buttons[i]](self.id) then
            slot = (i - 1)
            break
        end
    end
    return slot
end

function GamepadInputSource:sell_item_slot()
    return -1
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
function GamepadInputSource:poll_input()
end
return GamepadInputSource
