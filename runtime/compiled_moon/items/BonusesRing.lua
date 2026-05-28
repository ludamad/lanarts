local draw_console_text, draw_console_effect
do
  local _obj_0 = require('ui.ConsoleUtils')
  draw_console_text, draw_console_effect = _obj_0.draw_console_text, _obj_0.draw_console_effect
end
local define_bonus
define_bonus = require("items.Bonuses").define_bonus
local draw_simple_effect
draw_simple_effect = require("items.BonusesUtils").draw_simple_effect
return define_bonus({
  name = "Luxury",
  appears_in_stores = false,
  sprite = tosprite("spr_bonuses.luxury"),
  console_draw_func = function(self, obj, get_next)
    return draw_simple_effect(self, get_next(), "Luxury", "Sells for 3x.")
  end,
  item_draw_func = function(self, options, x, y)
    return self.sprite:draw(options, {
      x,
      y
    })
  end,
  apply = function(self, entry)
    local min, max
    do
      local _obj_0 = entry.shop_cost
      min, max = _obj_0[1], _obj_0[2]
    end
    entry.shop_cost = {
      min * 3,
      max * 3
    }
  end
})
