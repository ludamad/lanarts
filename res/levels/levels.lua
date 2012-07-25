

function overworld_tilegen()

end

function genroom_with_monster(level, padding, w, h, max_attempts)
	local result = gen_room(level, padding, w, h, max_attempts)
	if result then
		gen_monster(level, "Giant Rat", result)
	end
	return result
end
