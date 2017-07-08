local CombinedInputSource = newtype()

function CombinedInputSource:init(...)
    self.subinputs = {...}
end
function CombinedInputSource:move_direction()
    for _, subinput in ipairs(self.subinputs) do
        local dir = subinput:move_direction()
        if dir[1] ~= 0 or dir[2] ~= 0 then
            return dir
        end
    end
    return {0,0}
end

function CombinedInputSource:sell_mode()
    for _, subinput in ipairs(self.subinputs) do
        if subinput:sell_mode() then
            return true
        end
    end
    return false
end

function CombinedInputSource:should_explore()
    for _, subinput in ipairs(self.subinputs) do
        if subinput:should_explore() then
            return true
        end
    end
    return false
end

function CombinedInputSource:use_item_slot()
    for _, subinput in ipairs(self.subinputs) do
        if subinput:use_item_slot() ~= -1 then
            return subinput:use_item_slot()
        end
    end
    return -1
end

function CombinedInputSource:use_spell_slot()
    for _, subinput in ipairs(self.subinputs) do
        if subinput:use_spell_slot() ~= -1 then
            return subinput:use_spell_slot()
        end
    end
    return -1
end

function CombinedInputSource:sell_item_slot()
    for _, subinput in ipairs(self.subinputs) do
        if subinput:sell_item_slot() ~= -1 then
            return subinput:sell_item_slot()
        end
    end
    return -1
end

function CombinedInputSource:should_shift_autotarget()
    for _, subinput in ipairs(self.subinputs) do
        if subinput:should_shift_autotarget() then
            return true
        end
    end
    return false
end

function CombinedInputSource:target_position()
    for _, subinput in ipairs(self.subinputs) do
        local dir = subinput:target_position()
        if dir[1] ~= 0 or dir[2] ~= 0 then
            return dir
        end
    end
    return {0,0}
end

function CombinedInputSource:should_use_weapon()
    for _, subinput in ipairs(self.subinputs) do
        if subinput:should_use_weapon() then
            return true
        end
    end
    return false
end

function CombinedInputSource:slot_highlighted()
    for _, subinput in ipairs(self.subinputs) do
        local slot = subinput:slot_highlighted()
        if slot ~= -1 then
            return slot
        end
    end
    return -1
end

function CombinedInputSource:slot_selected()
    for _, subinput in ipairs(self.subinputs) do
        local slot = subinput:slot_selected()
        if slot ~= -1 then
            return slot
        end
    end
    return -1
end

function CombinedInputSource:handle_inventory(...)
    for _, subinput in ipairs(self.subinputs) do
        subinput:handle_inventory(...)
    end
end

function CombinedInputSource:handle_sidebar(...)
    for _, subinput in ipairs(self.subinputs) do
        subinput:handle_sidebar(...)
    end
end

function CombinedInputSource:handle_spells(...)
    for _, subinput in ipairs(self.subinputs) do
        subinput:handle_spells(...)
    end
end


function CombinedInputSource:handle_store(...)
    for _, subinput in ipairs(self.subinputs) do
        subinput:handle_store(...)
    end
end

function CombinedInputSource:handle_enemy_info(...)
    for _, subinput in ipairs(self.subinputs) do
        subinput:handle_enemy_info(...)
    end
end


function CombinedInputSource:poll_input()
    for _, subinput in ipairs(self.subinputs) do
        subinput:poll_input()
    end
end

function CombinedInputSource:draw_item_ui_hint(...)
    return self.subinputs[1]:draw_item_ui_hint(...)
end

function CombinedInputSource:draw_spell_ui_hint(...)
    return self.subinputs[1]:draw_spell_ui_hint(...)
end

function CombinedInputSource:draw_action_bar_weapon_hint(...)
    return self.subinputs[1]:draw_action_bar_weapon_hint(...)
end


function CombinedInputSource:draw_action_bar_spell_hint(...)
    return self.subinputs[1]:draw_action_bar_spell_hint(...)
end

return CombinedInputSource
