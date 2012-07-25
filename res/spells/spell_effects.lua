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
end

function berserk_deinit(effect, obj)
end

function exhausted_statmod(effect, obj, old, new)
end