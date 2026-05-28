local draw_console_text, draw_console_effect
do
  local _obj_0 = require('ui.ConsoleUtils')
  draw_console_text, draw_console_effect = _obj_0.draw_console_text, _obj_0.draw_console_effect
end
local define_bonus
define_bonus = require("items.Bonuses").define_bonus
local of_colors, draw_color_bound_weapon_effect
do
  local _obj_0 = require("items.BonusesUtils")
  of_colors, draw_color_bound_weapon_effect = _obj_0.of_colors, _obj_0.draw_color_bound_weapon_effect
end
local Display = require("core.Display")
local EventLog = require("ui.EventLog")
local GameObject = require("core.GameObject")
local BonusesUtils = require("items.BonusesUtils")
define_bonus({
  name = "Knockback",
  shop_cost = {
    50,
    100
  },
  sprite = tosprite("spr_effects.fleeing"),
  effects_granted = {
    "KnockbackWeapon"
  },
  item_draw_func = function(self, options, x, y)
    options = table.merge(options, {
      scale = {
        1,
        1
      }
    })
    return self.sprite:draw(options, {
      x - 20,
      y
    })
  end
})
define_bonus({
  name = "Stun",
  shop_cost = {
    50,
    100
  },
  sprite = tosprite("spr_effects.fleeing"),
  effects_granted = {
    "StunWeapon"
  },
  item_draw_func = function(self, options, x, y)
    options = table.merge(options, {
      scale = {
        1,
        1
      }
    })
    return self.sprite:draw(options, {
      x - 20,
      y
    })
  end
})
define_bonus({
  name = "Poison",
  shop_cost = {
    100,
    200
  },
  sprite = tosprite("spr_weapons.i-venom"),
  effects_granted = {
    {
      "PoisonedWeapon",
      {
        poison_percentage = 0.1
      }
    }
  },
  item_draw_func = function(self, options, x, y)
    options = table.merge(options, {
      scale = {
        1,
        1
      }
    })
    return self.sprite:draw(options, {
      x - 16,
      y - 16
    })
  end
})
define_bonus({
  name = "Confusion",
  shop_cost = {
    100,
    200
  },
  sprite = tosprite("spr_weapons.i-confusion"),
  effects_granted = {
    "ConfusingWeapon"
  },
  item_draw_func = function(self, options, x, y)
    options = table.merge(options, {
      scale = {
        1,
        1
      }
    })
    return self.sprite:draw(options, {
      x - 16,
      y - 16
    })
  end
})
return define_bonus({
  name = "Vampiric",
  shop_cost = {
    100,
    200
  },
  sprite = tosprite("spr_weapons.i-vampirism"),
  item_draw_func = function(self, options, x, y)
    options = table.merge(options, {
      scale = {
        1,
        1
      }
    })
    return self.sprite:draw(options, {
      x - 16,
      y - 16
    })
  end,
  console_draw_func = function(self, obj, get_next)
    return draw_color_bound_weapon_effect(self, obj, get_next, {
      "Black",
      "Green"
    }, "Heal +20% of melee damage dealt.")
  end,
  effect = {
    init_func = function(self, obj)
      self.n_animations = 0
    end,
    on_melee_func = function(self, obj, defender, damage)
      if of_colors(obj, {
        "Black",
        "Green"
      }) then
        for _ in screens() do
          if obj:is_local_player() then
            EventLog.add("You steal the enemy's life!", {
              200,
              200,
              255
            })
          end
        end
        BonusesUtils.create_animation(self, obj, "spr_bonuses.fangs")
        obj:heal_hp(damage / 5 * self.n_derived)
      end
      return damage
    end
  }
})
