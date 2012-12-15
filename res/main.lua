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

settings = {verbose_output = false}


-- lanarts main loop
-- this controls the games behaviour every step!
function main_loop() 


end
