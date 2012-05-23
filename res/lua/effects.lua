function increase_health(item, obj)
	obj:heal_hp(item.amount)
end 

function increase_mana(item, obj)
	obj:heal_mp(item.action_amount)
end 

function increase_strength(item, obj)
	obj.strength = obj.strength + item.action_amount
end 

function increase_defence(item, obj)
	obj.defence = obj.defence + item.action_amount
end 

function increase_magic(item, obj)
	obj.magic = obj.magic + item.action_amount
end 

function equip(item, obj)
	obj:equip(item)
end
function hasten(item, obj)
	obj:hasten(item.action_duration)
end
