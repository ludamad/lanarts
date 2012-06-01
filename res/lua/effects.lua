function haste_statmod(old, new)
	new.movespeed += old.movespeed/2;
	new.melee_cooldown = new.melee_cooldown / 1.50;
	new.strength += 10
	new.magic += 10
	new.defence += 10
	new.magic_cooldown = new.magic_cooldown / 1.50
end
