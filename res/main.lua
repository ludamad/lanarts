-- Lua API constants, change only if you know what you are doing
-- LUA API CONSTANTS GO HERE
-- End of Lua API constants

-- Lua API functions
function is_consumable(item) 	return item.type == "consumable" end
function is_weapon(item) 		return item.type == "weapon" end
function is_armour(item) 		return item.type == "armour" end
function is_projectile(item) 	return item.type == "projectile" end

-- End of Lua API constants

-- Include all other lua files here
dofile "res/enemies/enemies.lua"
dofile "res/items/items.lua"
dofile "res/items/weapons/weapons.lua"
dofile "res/effects/effects.lua"
dofile "res/spells/spell_effects.lua"
dofile "res/spells/spells.lua"
dofile "res/levels/levels.lua"

-- Balance measurement related files
dofile "res/tests/progressions.lua"

local paused = false

-- lanarts main loop
-- this controls the games behaviour every step!
function main_loop() 
		local single_player = (#world.players() == 1)
	
		perf_timer_begin("**Game Frame**")

		local total_timer = timer()
	
		if key_pressed(keys.F2) then
			if single_player then gamestate.resources_reload() end
		end

		if key_pressed(keys.F3) then
			if single_player then gamestate.level_regenerate() end
		end

		if key_pressed(keys.F4) then
			paused = not paused
		end

		net.sync_message_consume()

		if key_pressed(keys.F6) then
			if single_player then gamestate.level_regenerate() end
		end

		perf_timer_end("**Game Frame**")
end


local timer = timer_create()
local paused = false

function main_loop_simple()
	local single_player = (settings.connection_type == net.NONE)

	perf_timer_begin("**Game Frame**")

	if key_pressed(keys.F2) and single_player then 
		game.resources_load()
	end

	if key_pressed(keys.F3) and single_player then 
		level.regenerate()
	end

	if key_pressed(keys.F4) then 
		paused = not paused
	end

	net.sync_message_consume()

	if key_pressed(keys.F6) and single_player then
		game.load("savefile.save")
		game.input_capture(true) -- capture new input
	end

	timer:start()

	game.draw()
	game.step()

	if not game.input_handle() then 
		return false 
	end

	local surplus = settings.time_per_step - timer:get_microseconds() / 1000

	if surplus > 0 then 
		game.wait(surplus) 
	end

	perf_timer_end("**Game Frame**")
	
	return true;
end
