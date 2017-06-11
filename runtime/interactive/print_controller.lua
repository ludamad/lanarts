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
				"'a' state: " .. tostring(Gamepad.button_a(id))
			)
			x = x + 100
	end
end)
