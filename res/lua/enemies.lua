function enemy_init(enemy)
end

function enemy_step(enemy)
end


function heal_step(enemy)
	if enemy.stats.hp < 20 then 
		enemy.stats.hp = enemy.stats.hp + 1
	end
end
