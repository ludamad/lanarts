local KeyboardInputSource = newtype()

local Keys = require "core.Keyboard"
local GameState = require "core.GameState"

function KeyboardInputSource:init(player)
    self.player = player
end

local function pressed(key) 
    return Keys.key_pressed(Keys[key])
end

local function held(key) 
    return Keys.key_held(Keys[key])
end


function KeyboardInputSource:move_direction()
    local dir = {0,0}
    if held "DOWN" or held "s" then
        dir[2] = 1
    end
    if held "UP" or held "w" then
        dir[2] = -1
    end
    if held "RIGHT" or held "d" then
        dir[1] = 1
    end
    if held "LEFT" or held "a" then
        dir[1] = -1
    end
    return dir
end

function KeyboardInputSource:sell_mode()
    return false -- TODO check for ctrl and unhardcode in C++
end
function KeyboardInputSource:should_explore()
    return held "e"
end

function KeyboardInputSource:use_item_slot()
    if Keys.ctrl_held() then
        return -1
    end
    local slot = -1
    for i=1,9 do
        if pressed(tostring(i)) then
            slot = (i - 1)
            break
        end
    end
    return slot
end

local spell_keys = {'y', 'u', 'i', 'o', 'p'}
function KeyboardInputSource:use_spell_slot()
    local slot = -1
    for i=1,#spell_keys do
        if held(spell_keys[i]) then
            slot = (i - 1)
            break
        end
    end
    return slot
end

function KeyboardInputSource:sell_item_slot()
    if not Keys.ctrl_held() then
        return -1
    end
    local slot = -1
    for i=1,9 do
        if pressed(tostring(i)) then
            slot = (i - 1)
            break
        end
    end
    return slot
end

function KeyboardInputSource:should_shift_autotarget()
    return false -- TODO
end

function KeyboardInputSource:target_position()
    return {0,0}
end
function KeyboardInputSource:should_use_weapon()
    return held "g"
end
function KeyboardInputSource:should_exit_game()
    error("TODO")
end
function KeyboardInputSource:poll_input()
end

function KeyboardInputSource:slot_highlighted()
    return -1
end

function KeyboardInputSource:slot_selected()
    return -1
end


function KeyboardInputSource:handle_store(try_buy_item)
end


function KeyboardInputSource:handle_inventory()
end

function KeyboardInputSource:handle_sidebar(switch_left, switch_right)
    if pressed "PAGE_UP" then switch_left() end
    if pressed "PAGE_DOWN" then switch_right() end
end

function KeyboardInputSource:handle_spells()
end


function KeyboardInputSource:handle_store(...)
     -- TODO
end

function KeyboardInputSource:handle_enemy_info()
end


function KeyboardInputSource:sell_mode(...) 
    return false
end
function KeyboardInputSource:draw_item_ui_hint(...) -- TODO
end

function KeyboardInputSource:draw_spell_ui_hint(...) -- TODO
end

function KeyboardInputSource:draw_action_bar_weapon_hint(...) -- TODO
end
function KeyboardInputSource:draw_action_bar_spell_hint(...) -- TODO
end

for k, v in pairs(require "input.KeyboardDraw") do
    KeyboardInputSource[k] = v
end

return KeyboardInputSource
