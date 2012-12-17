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

local function mainloop(steponly)
	perf.timing_begin("**Game Frame**")

	net.sync_message_consume()

	local timer = timer_create()

	if not steponly then
		perf.timing_begin("**Draw**")
		game.draw()
		perf.timing_end("**Draw**")
	end

	perf.timing_begin("**Step**")
	game.step()
	perf.timing_end("**Step**")

	if not game.input_handle() then 
		return false 
	end

	local surplus = settings.time_per_step - timer:get_milliseconds()

	if surplus > 0 then 
		game.wait(surplus) 
	end

	perf.timing_end("**Game Frame**")

	return true;
end

function main()

	game.input_capture()

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

		if key_pressed(keys.F6) and single_player then
			game.load("savefile.save")
			game.input_capture(true) -- reset input
		end

		if not mainloop(false) then
			break
		end

		if key_pressed(keys.F5) then
			net.sync_message_send()
			game.save("savefile.save")
		end
	end

	perf.timing_print()

	print( "Step time: " .. string.format("%f", perf.get_timing("**Step**")) )
	print( "Draw time: " .. string.format("%f", perf.get_timing("**Draw**")) )
end
