-- Lua API constants, change only if you know what you are doing
-- LUA API CONSTANTS GO HERE
-- End of Lua API constants

-- Lua API functions

dofile "res/sound.lua"

function is_consumable(item) 	return item.type == "consumable" end
function is_weapon(item) 		return item.type == "weapon" end
function is_armour(item) 		return item.type == "armour" end
function is_projectile(item) 	return item.type == "projectile" end

-- End of Lua API constants

-- dofile "res/actors/actors.lua"

-- Include all other lua files here
--dofile "res/projectiles.lua"
dofile "res/enemies/enemies.lua"
dofile "res/items/items.lua"
dofile "res/items/weapons/weapons.lua"
dofile "res/effects/effects.lua"
dofile "res/spells/spell_effects.lua"
dofile "res/spells/spells.lua"
dofile "res/levels/levels.lua"

require "help_overlay"
require "start_menu"

local paused = false

local function main_loop(steponly)
	perf.timing_begin("**Game Frame**")

	perf.timing_begin("**Sync Message**")
	net.sync_message_consume()
	perf.timing_end("**Sync Message**")

	local timer = timer_create()

	if not steponly then
		perf.timing_begin("**Draw**")
		game.draw()
		perf.timing_end("**Draw**")
	end

	perf.timing_begin("**Step**")
	if not game.step() then 
		return false 
	end
	perf.timing_end("**Step**")

	if not game.input_handle() then 
		return false 
	end

	local surplus = settings.time_per_step - timer:get_milliseconds()

	perf.timing_begin("**Surplus**")
	game.wait(surplus)
	perf.timing_end("**Surplus**")

	perf.timing_end("**Game Frame**")

	return true
end

local fps_timer = timer_create()
local fps_count = 1
local fps = nil

function postdraw()
	fps_count = fps_count + 1

	if fps then
		local w,h = unpack( display.window_size ) 
		fonts.small:draw( {origin=RIGHT_BOTTOM}, {w, h}, "FPS: " .. math.floor(fps) )
	end

	local ms = fps_timer:get_milliseconds()
	if ms > 1000 then
		fps = fps_count / ms * 1000
		fps_timer:start()
		fps_count = 0
	end
end

local music = music_optional_load("res/sound/lanarts1.ogg")

function main()
    config.startup_function()

	game.input_capture()
	music:loop()

	while true do 
		local single_player = (settings.connection_type == net.NONE)
	
		if key_pressed(keys.F2) and single_player then 
			game.resources_load()
		end

		if key_pressed(keys.F3) and single_player then 
			level.regenerate()
		end
	
		if key_pressed(keys.F4) then 
			paused = not paused
		end

		if key_pressed(keys.ESCAPE) then 
			show_message("Press Shift + Esc to exit, your progress will be saved.")
		end

		if not main_loop(false) then
			game.save("savefile.save")
			break
		end

		if key_pressed(keys.F5) then
			game.input_capture(true) -- reset input
			net.sync_message_send()
		end
	end

	perf.timing_print()

	print( "Step time: " .. string.format("%f", perf.get_timing("**Step**")) )
	print( "Draw time: " .. string.format("%f", perf.get_timing("**Draw**")) )
end
