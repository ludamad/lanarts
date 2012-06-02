function haste_statmod(old, new)
	new.speed = new.speed + old.speed/2
	new.melee_rate = new.melee_rate / 1.50
	new.strength = new.strength + 10
	new.magic = new.strength + 10
	new.defence = new.defence + 10
	new.magic_rate = new.magic_rate / 1.50
end

function haste_step(obj)
	obj:heal_hp(10)
end