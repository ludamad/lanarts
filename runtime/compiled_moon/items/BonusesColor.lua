local draw_console_text, draw_console_effect
do
  local _obj_0 = require('ui.ConsoleUtils')
  draw_console_text, draw_console_effect = _obj_0.draw_console_text, _obj_0.draw_console_effect
end
local define_bonus
define_bonus = require("items.Bonuses").define_bonus
local of_color, draw_color_bound_effect
do
  local _obj_0 = require("items.BonusesUtils")
  of_color, draw_color_bound_effect = _obj_0.of_color, _obj_0.draw_color_bound_effect
end
local define_color_bonus
define_color_bonus = function(name)
  local BONUS
  BONUS = define_bonus({
    name = name,
    shop_cost = {
      50,
      100
    },
    sprite = tosprite("spr_bonuses." .. tostring(name:lower())),
    effect = {
      console_draw_func = function(self, obj, get_next)
        draw_color_bound_effect(BONUS, obj, get_next, {
          name
        }, "+1 Strength, +1 Magic, Improved Spells.")
        return get_next()
      end,
      stat_func = function(self, obj, old, new)
        if of_color(obj, name) then
          new.strength = new.strength + self.n_derived
          new.magic = new.magic + self.n_derived
        end
      end
    },
    item_draw_func = function(self, options, x, y)
      return self.sprite:draw(options, {
        x,
        y
      })
    end
  })
end
define_color_bonus("Black")
define_color_bonus("White")
define_color_bonus("Red")
define_color_bonus("Green")
return define_color_bonus("Blue")
