local draw_console_text, draw_console_effect
do
  local _obj_0 = require('ui.ConsoleUtils')
  draw_console_text, draw_console_effect = _obj_0.draw_console_text, _obj_0.draw_console_effect
end
local define_bonus
define_bonus = require("items.Bonuses").define_bonus
local draw_simple_effect
draw_simple_effect = require("items.BonusesUtils").draw_simple_effect
local summon_console_draw_func
summon_console_draw_func = function(bonus, obj, get_next)
  draw_simple_effect(bonus, get_next(), bonus.name, "Can summon after kill")
  return get_next()
end
define_bonus({
  name = "Centaur",
  shop_cost = {
    100,
    200
  },
  sprite = tosprite("spr_enemies.humanoid.centaur"),
  effects_granted = {
    "PossiblySummonCentaurOnKill"
  },
  console_draw_func = summon_console_draw_func,
  item_draw_func = function(self, options, x, y)
    return self.sprite:draw(options, {
      x,
      y
    })
  end
})
define_bonus({
  name = "Golem",
  shop_cost = {
    100,
    200
  },
  sprite = tosprite("golem"),
  effects_granted = {
    "PossiblySummonGolemOnKill"
  },
  console_draw_func = summon_console_draw_func,
  item_draw_func = function(self, options, x, y)
    return self.sprite:draw(options, {
      x,
      y
    })
  end
})
define_bonus({
  name = "Stormcall",
  shop_cost = {
    50,
    100
  },
  sprite = tosprite("storm elemental"),
  effects_granted = {
    "PossiblySummonStormElementalOnKill"
  },
  console_draw_func = summon_console_draw_func,
  item_draw_func = function(self, options, x, y)
    return self.sprite:draw(options, {
      x,
      y
    })
  end
})
return define_bonus({
  name = "Mummycall",
  shop_cost = {
    200,
    400
  },
  sprite = tosprite("spr_enemies.undead.mummy"),
  effects_granted = {
    "SummonMummyOnKill"
  },
  console_draw_func = summon_console_draw_func,
  item_draw_func = function(self, options, x, y)
    return self.sprite:draw(options, {
      x,
      y
    })
  end
})
