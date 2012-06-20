function haste_statmod(old, new)
	new.speed = new.speed + 2
	new.strength = new.strength + 5
	new.magic = new.strength + 5
	new.defence = new.defence + 5
	new.willpower = new.willpower + 5
	new.cooldown_mult = new.cooldown_mult / 1.25
end

function haste_step(obj)
	obj:heal_hp(10)
end