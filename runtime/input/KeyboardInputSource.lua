local KeyboardInputSource = newtype()

local Gamepad = require "core.Gamepad"
local Keys = require "core.Keyboard"
local GameActions = require "core.GameActions"
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

function KeyboardInputSource:should_explore()
    return held "e"
end

function KeyboardInputSource:use_item_slot()
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
    error("TODO")
end

function KeyboardInputSource:should_shift_autotarget()
    return false -- TODO
end

function KeyboardInputSource:target_position()
    return {0,0}
end
function KeyboardInputSource:should_use_weapon()
    return held "h"
end
function KeyboardInputSource:should_exit_game()
    error("TODO")
end
function KeyboardInputSource:poll_input()
end
return KeyboardInputSource
