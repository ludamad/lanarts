local GamepadInputSource = newtype()

local Gamepad = require "core.Gamepad"
local GameActions = require "core.GameActions"
local GameState = require "core.GameState"

function GamepadInputSource:init(player)
    self.player = player
    self:reset()
end

function GamepadInputSource:move_direction()
    return self.input.move_direction
end

function GamepadInputSource:should_explore()
    return self.input.should_explore
end

function GamepadInputSource:use_item_slot()
    return self.input.use_item_slot
end

function GamepadInputSource:use_spell_slot()
    return self.input.use_spell_slot
end

function GamepadInputSource:sell_item_slot()
    error("TODO")
end

function GamepadInputSource:should_shift_autotarget()
    return self.input.should_shift_autotarget
end

function GamepadInputSource:target_position()
    return self.input.target_position
end
function GamepadInputSource:should_use_weapon()
    return self.input.should_use_weapon
end
function GamepadInputSource:should_exit_game()
    error("TODO")
end
function GamepadInputSource:set(key, val)
    self.input[key] = val
end
function GamepadInputSource:reset()
    -- "Do nothing" state
    self.input = {
        move_direction = {0,0},
        should_explore = false,
        use_item_slot = -1,
        should_shift_autotarget = false, 
        target_position = {0,0},
        should_use_weapon = false,
        use_spell_slot = -1,
    }
end
function GamepadInputSource:poll_input()
end
return GamepadInputSource
