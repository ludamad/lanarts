
function attack_all_seen(caster, _, _) 
	local monsters = monsters_seen(caster)
	for i=1,#monsters do
		monsters[i]:damage(caster.power / 5, caster.power)
	end

end