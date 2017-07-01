local ProgrammableInputSource = newtype()

local GameActions = require "core.GameActions"
local GameState = require "core.GameState"

function ProgrammableInputSource:init(player)
    self.player = player
    self:reset()
end

function ProgrammableInputSource:move_direction()
    return self.input.move_direction
end

function ProgrammableInputSource:should_explore()
    return self.input.should_explore
end

function ProgrammableInputSource:use_item_slot()
    return self.input.use_item_slot
end

function ProgrammableInputSource:use_spell_slot()
    return self.input.use_spell_slot
end

function ProgrammableInputSource:sell_item_slot()
    return self.input.sell_item_slot
end

function ProgrammableInputSource:should_shift_autotarget()
    return self.input.should_shift_autotarget
end

function ProgrammableInputSource:target_position()
    return self.input.target_position
end
function ProgrammableInputSource:should_use_weapon()
    return self.input.should_use_weapon
end
function ProgrammableInputSource:should_exit_game()
    error("TODO")
end
function ProgrammableInputSource:set(key, val)
    self.input[key] = val
end
function ProgrammableInputSource:reset()
    -- "Do nothing" state
    self.input = {
        move_direction = {0,0},
        should_explore = false,
        use_item_slot = -1,
        sell_item_slot = -1,
        should_shift_autotarget = false, 
        target_position = {0,0},
        should_use_weapon = false,
        use_spell_slot = -1,
    }
end

function ProgrammableInputSource:slot_highlighted()
    return -1
end
function ProgrammableInputSource:slot_selected()
    return -1
end
function ProgrammableInputSource:handle_inventory(drop_item, reposition_item)
end

function ProgrammableInputSource:poll_input()
end
return ProgrammableInputSource
