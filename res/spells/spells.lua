
function attack_all_seen(caster, _, _) 
	local m = monsters_seen(caster)
	local stats = caster.stats
	local pow = 1
	show_message("Doing " .. pow + stats.magic / 4.5 .." with dmg " .. rand_range(4, 10) + stats.magic*2 )
	for k, mon in ipairs(m) do
		mon:damage(pow + stats.magic / 4.5, rand_range(4, 10) + stats.magic*2)
	end

end
