function haste_statmod(effect, obj, old, new)
	new.speed = new.speed + 2
	new.strength = new.strength + 2
	new.magic = new.strength + 2
	new.defence = new.defence + 4
	new.willpower = new.willpower + 4
	new.cooldown_mult = new.cooldown_mult / 1.25
end

function poison_statmod(effect, obj, old, new)
	new.defence = math.max(0, new.defence - 5)
	new.willpower = math.max(0, new.willpower - 5)
end

function poison_step(effect, obj)
	if not effect.steps then 
		effect.steps = 0
	elseif effect.steps < 28 then
		effect.steps = effect.steps + 1
	else
		effect.steps = 0
		obj:damage(effect.power,effect.damage, effect.magic_percentage)
	end
end