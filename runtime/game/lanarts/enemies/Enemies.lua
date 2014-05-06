function enemy_init(enemy)
end

function enemy_step(enemy)
end

function enemy_berserker_init(enemy)
	enemy.berserk_time = 500
end

function enemy_berserker_step(enemy)
	enemy.berserk_time = enemy.berserk_time - 1
	if enemy.berserk_time <= 0 then
		enemy:add_effect("Berserk", 200)
		enemy.berserk_time = 500
	end
end