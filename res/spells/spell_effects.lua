function poison_statmod(effect, obj, old, new)
	new.defence = math.max(0, new.defence - 5)
	new.willpower = math.max(0, new.willpower - 5)
end

function poison_init(effect, obj)
	effect.steps = 0
end

function poison_step(effect, obj)
	if effect.steps < effect.poison_rate then
		effect.steps = effect.steps + 1
	else
		effect.steps = 0
		obj:damage(effect.damage,effect.power, effect.magic_percentage, 0.25)
	end
end

function poison_statmod(effect, obj, old, new)
	new.defence = math.max(0, new.defence - 5)
	new.willpower = math.max(0, new.willpower - 5)
end

local function berserk_extension(effect) 
	if effect.extensions < 1 then
		return 40
	elseif effect.extensions < 5 then
		return 20
	end
	return 5
end

function berserk_step(effect, obj)
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

function berserk_init(effect, obj) 
	effect.kill_tracker = obj.kills 
	effect.extensions = 0
end

function berserk_statmod(effect, obj, old, new)	
	new.strength = new.strength + 2
	new.defence = new.defence + 4
	new.cooldown_mult = new.cooldown_mult / 1.35
	new.speed = new.speed + 1
	obj:reset_rest_cooldown()
end

function exhausted_statmod(effect, obj, old, new)
	new.speed = new.speed / 2
	new.defence = new.defence - 3
	new.cooldown_mult = new.cooldown_mult * 1.25
	obj:reset_rest_cooldown()
end
