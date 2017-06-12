--
-- Created by IntelliJ IDEA.
-- User: patcgoe
-- Date: 6/11/17
-- Time: 8:58 PM
-- To change this template use File | Settings | File Templates.
--

local Gamepad = require "core.Gamepad"
local Keyboard = require "core.Keyboard"
local GameActions = require "core.GameActions"
local GameState = require "core.GameState"

local GamepadInputSource = newtype()

local button_action_mapping = {
    button_a = GameActions.USE_WEAPON,
    button_b = 0,
    button_x = 0,
    button_y = 0,
    button_back = 0,
    button_guide = GameActions.EXIT_GAME,
    button_start = 0,
    button_left_stick = 0,
    button_right_stick = 0,
    button_left_shoulder = 0,
    button_right_shoulder = 0,
    button_up_dpad = 0,
    button_down_dpad = 0,
    button_left_dpad = 0,
    button_right_dpad = 0,
}

local axis_action_mapping = {
    axis_left_trigger = 0,
    axis_right_trigger = 0,
    axis_left_x = GameActions.MOVE_X_M,
    axis_left_y = GameActions.MOVE_Y_M,
    axis_right_x = 0,
    axis_right_y = 0,
}

function GamepadInputSource:init(gamepad_id)
    self.gamepad_id = gamepad_id
end

function GamepadInputSource:step()
    for trigger, action in pairs(button_action_mapping) do
        if Gamepad[trigger](self.gamepad_id) then
            GameState._simulate_key_press( action )
        end

    end
end

local function step_for_all()
    for _, id in ipairs(Gamepad.ids()) do
        for trigger, action in pairs(button_action_mapping) do
            if Gamepad[trigger](id) then
                print(trigger, Gamepad[trigger](id))
                GameActions.trigger_action( action )
            end
        end
        for trigger, action in pairs(axis_action_mapping) do
            if Gamepad[trigger](id) > 0.01 or Gamepad[trigger](id) < -0.01 then
                print(trigger, Gamepad[trigger](id))
                GameActions.trigger_analog_action( action , Gamepad[trigger](id))
            end
        end
    end
end

return {
    step_for_all = step_for_all
}