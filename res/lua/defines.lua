function step()
	ps = world:players_in_room()
	for i = 1, #ps do
		ps[i]:hurt(10)
		increase_health(ps[1], 9)
	end
end

function increase_health(obj, amount)
	obj:heal_hp(amount)
end