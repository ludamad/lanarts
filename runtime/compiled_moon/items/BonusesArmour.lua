local draw_console_text, draw_console_effect
do
  local _obj_0 = require('ui.ConsoleUtils')
  draw_console_text, draw_console_effect = _obj_0.draw_console_text, _obj_0.draw_console_effect
end
local define_bonus
define_bonus = require("items.Bonuses").define_bonus
local BonusesUtils = require("items.BonusesUtils")
local EventLog = require("ui.EventLog")
define_bonus({
  name = "Spiky",
  sprite = tosprite("spr_bonuses.spiky"),
  shop_cost = {
    100,
    200
  },
  item_draw_func = function(self, options, x, y)
    return self.sprite:draw(options, {
      x,
      y
    })
  end,
  effect = {
    console_draw_func = function(self, obj, get_next)
      local sprite = tosprite("spr_bonuses.spiky")
      BonusesUtils.draw_simple_effect({
        sprite = tosprite("spr_bonuses.spiky")
      }, get_next(), "Spiky", "You deal +25% damage back on melee.")
      return get_next()
    end,
    init_func = function(self, obj)
      self.n_animations = 0
    end,
    on_receive_melee_func = function(self, attacker, obj, damage)
      local percentage_recoil = self.n_derived * 0.25
      attacker:direct_damage(random_round(damage * percentage_recoil), obj)
      for _ in screens() do
        if obj.is_local_player and obj:is_local_player() then
          EventLog.add("You strike back with spikes!", COL_PALE_BLUE)
        end
      end
      return BonusesUtils.create_animation(self, attacker, "spr_bonuses.spiky")
    end
  }
})
return define_bonus({
  name = "Fortified",
  shop_cost = {
    100,
    200
  },
  sprite = tosprite("spr_spells.statue_form"),
  effects_granted = {
    "Fortification"
  },
  item_draw_func = function(self, options, x, y)
    return self.sprite:draw(options, {
      x,
      y
    })
  end
})
