function increase_health(item, obj)
	obj:heal_hp(item.amount)
end 

function increase_mana(item, obj)
	obj:heal_mp(item.amount)
end 

function increase_strength(item, obj)
	obj.strength = obj.strength + item.amount
end 

function increase_defence(item, obj)
	obj.defence = obj.defence + item.amount
end 

function increase_magic(item, obj)
	obj.magic = obj.magic + item.amount
end 

function equip(item, obj)
	obj:equip(item)
end
function hasten(item, obj)
	obj:hasten(item.duration)
end

function stone(item, obj, tx, ty)
	world:create_projectile(obj, tx, ty, sprites["stone"], 
		item.speed, item.range, item.damage)
	obj.cooldown = item.cooldown
end
