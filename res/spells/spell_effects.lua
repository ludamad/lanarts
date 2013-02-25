function poison_statmod(effect, obj, old, new)
	new.defence = math.max(0, new.defence - 3)
	new.willpower = math.max(0, new.willpower - 3)
	new.speed = new.speed * 1.66
end

function poison_init(effect, obj)
	effect.steps = 0
end

function poison_step(effect, obj)
	if effect.steps < effect.poison_rate then
		effect.steps = effect.steps + 1
	else
		effect.steps = 0
		obj:damage(effect.damage,effect.power, effect.magic_percentage, 0.5)
	end
end

local function berserk_extension(effect) 
	if effect.extensions < 1 then
		return 30
	elseif effect.extensions < 5 then
		return 20
	end
	return 5
end


-- berserking

berserk_effect = {}

function berserk_effect.step(effect, obj)
	local killdiff = obj.kills - effect.kill_tracker

	while killdiff > 0 do
		effect.time_left = effect.time_left + berserk_extension(effect)
		if obj:is_local_player() then
			show_message("Your rage grows ...", {200,200,255})
		end
		if settings.verbose_output then
			show_message("Killed Enemy, berserk time_left = " .. effect.time_left)
		end
		killdiff = killdiff -1
		effect.extensions = effect.extensions + 1
	end

	effect.kill_tracker = obj.kills
end

function berserk_effect.init(effect, obj) 
	effect.kill_tracker = obj.kills 
	effect.extensions = 0
end

function berserk_effect.stat(effect, obj, old, new)	
	new.strength = new.strength + 2 
	new.defence = new.defence + 4 + obj.stats.level
	new.melee_cooldown_multiplier = new.melee_cooldown_multiplier / 1.35
	new.speed = new.speed + 1
	obj:reset_rest_cooldown()
end

-- exhausted

exhausted_effect = {}

function exhausted_effect.stat(effect, obj, old, new)
	new.speed = new.speed / 2
	new.defence = new.defence - 3
	new.cooldown_mult = new.cooldown_mult * 1.25
	obj:reset_rest_cooldown()
end

-- charge

charge_effect = {}

function charge_effect.stat(effect, obj, old, new)
	new.defence = new.defence + 4 + obj.stats.level
	new.speed = 0
end

function charge_effect.finish(effect, obj) 
	effect:callback(obj)
end

-- thrown

thrown_effect = {}

function thrown_effect.init(effect, obj)
	effect.steps = 0
end

local THROW_DURATION = 9

function thrown_effect.step(effect, obj) 

	local increment = THROW_DURATION - effect.steps

	if effect.steps <= THROW_DURATION then
		local x = math.floor( obj.x + math.cos(effect.angle) * increment )
		local y = math.floor( obj.y + math.sin(effect.angle) * increment )
	
		if level.object_place_free(obj, {x,y}) then 
			obj.x = x
			obj.y = y
		end
	end

	effect.steps = effect.steps + 1
end

function thrown_effect.stat(effect, obj, old, new) 
	local immune = effect.steps > THROW_DURATION*2 and (obj.name == "Red Dragon" or obj.name == "Zin")
	if not immune then
		new.speed = 0
		
		obj.stats.attack_cooldown = 2
	end
end
