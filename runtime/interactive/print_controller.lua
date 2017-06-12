local Display = require "core.Display"
local Gamepad = require "core.Gamepad"
local GameState = require "core.GameState"
local WIDTH, HEIGHT = 640, 480

Display.initialize("Demo", {WIDTH, HEIGHT}, false)

local font = Display.font_load("fonts/Gudea-Regular.ttf", 14)

GameState.game_loop(function()
	local ids = Gamepad.ids(0)
	local x1, y1 = WIDTH/4, HEIGHT/4
	local x, y = x1, y1
	for _, id in ipairs(ids) do
			font:draw(
				{color=COL_WHITE, origin=Display.CENTER},
				{x, y},
				"ID = " .. id
			)
			font:draw(
				{color=COL_WHITE, origin=Display.CENTER},
				{x, y + 25},
				"'left X' state: " .. tostring(Gamepad.axis_left_x(id))
			)
			font:draw(
				{color=COL_WHITE, origin=Display.CENTER},
				{x, y + 50},
				"'left Y' state: " .. tostring(Gamepad.axis_left_y(id))
			)
--			x = x + 200
			y = y + 100
	end
end)
