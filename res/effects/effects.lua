function haste_statmod(effect, obj, old, new)
	new.speed = new.speed + 2
	new.strength = new.strength + 2
	new.magic = new.strength + 2
	new.defence = new.defence + 4
	new.willpower = new.willpower + 4
	new.cooldown_mult = new.cooldown_mult / 1.25
end