local DataW = require("DataWrapped")
return DataW.spell_create({
  name = "Storm Cloud",
  description = "",
  types = {
    "White"
  },
  prereq_func = function(caster)
    return true
  end,
  autotarget_func = function(caster)
    return caster.x, caster.y
  end,
  action_func = function(caster, x, y) end,
  projectile = {
    speed = 5,
    damage_multiplier = 2.00
  },
  mp_cost = 10,
  spell_cooldown = 400,
  cooldown = 35
})
