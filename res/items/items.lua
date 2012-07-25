function increase_health(item, obj)
	obj:heal_hp(item.amount)
end 

function increase_mana(item, obj)
	obj:heal_mp(item.amount)
end 

function increase_strength(item, obj)
	obj.stats.strength = obj.stats.strength + item.amount
end 

function increase_defence(item, obj)
	obj.stats.defence = obj.stats.defence + item.amount
end 

function increase_magic(item, obj)
	obj.stats.magic = obj.stats.magic + item.amount
end 

function increase_willpower(item, obj)
	obj.stats.willpower = obj.stats.willpower + item.amount
end 

function equip(item, obj, _, _, amnt)
	obj:equip(item, amnt)
end
function hasten(item, obj)
	obj:add_effect(effects.Haste.name, item.duration)
end

function stone(item, obj, tx, ty)
	world:create_projectile(obj, tx, ty, sprites["stone"], 
		item.speed, item.range, item.damage)
	obj.cooldown = item.cooldown
end
