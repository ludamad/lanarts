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
		obj:damage(effect.power,effect.damage, effect.magic_percentage)
	end
end

function poison_statmod(effect, obj, old, new)
	new.defence = math.max(0, new.defence - 5)
	new.willpower = math.max(0, new.willpower - 5)
end

function berserk_statmod(effect, obj, old, new)	
	new.strength = new.strength + 5
	new.defence = new.defence + 5
	new.cooldown_mult = new.cooldown_mult / 1.25
end

function exhausted_statmod(effect, obj, old, new)
	new.speed = new.speed / 2
	new.cooldown_mult = new.cooldown_mult * 1.25
end
