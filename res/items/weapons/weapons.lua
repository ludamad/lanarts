function use_magic_sword(user, target) 
	local mpcost = rand_range(4,6)
    if user.stats.mp >= mpcost then
      -- Essentially 1/4th extra damage:
      target:damage(user.stats.strength / 4 + rand_range(1,2), user.stats.strength / 5 + 1, 0.0, 0.25)
      user.stats.mp = user.stats.mp - mpcost
    end
end