function use_magic_sword(user, target) 
    if user.stats.mp >= 5 then
      -- Essentially 1/4th extra damage:
      target:damage(user.stats.strength / 4 + rand_range(1,2), user.stats.strength / 5 + 1, 0.0, 0.25)
      user.stats.mp = user.stats.mp - 5
    end
end