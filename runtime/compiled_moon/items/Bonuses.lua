local draw_console_text, draw_console_effect
do
  local _obj_0 = require('ui.ConsoleUtils')
  draw_console_text, draw_console_effect = _obj_0.draw_console_text, _obj_0.draw_console_effect
end
local Display = require("core.Display")
local DataW = require("DataWrapped")
local BONUSES = OrderedDict()
local define_bonus
define_bonus = function(bonus)
  BONUSES[bonus.name] = bonus
  if bonus.effect then
    DataW.effect_create(table.merge(bonus.effect, {
      name = tostring(bonus.name) .. "Bonus"
    }))
    bonus.effects_granted = {
      tostring(bonus.name) .. "Bonus"
    }
  elseif bonus.additive_effect then
    DataW.additive_effect_create(table.merge(bonus.additive_effect, {
      name = tostring(bonus.name) .. "Bonus",
      key = "amount"
    }))
    bonus.effects_granted = {
      {
        tostring(bonus.name) .. "Bonus",
        {
          amount = bonus.effect_amount or 1
        }
      }
    }
  end
  return bonus
end
local ONE_OFF_LOCS = {
  {
    8,
    8
  }
}
local BONUS_LOCS = {
  {
    16,
    16
  },
  {
    0,
    0
  },
  {
    16,
    0
  },
  {
    0,
    16
  }
}
local DEFAULT_STAT_BONUSES = {
  mp = 0,
  hp = 0,
  hpregen = 0,
  mpregen = 0,
  strength = 0,
  defence = 0,
  willpower = 0,
  magic = 0,
  spell_velocity_multiplier = 1.0,
  spell_cooldown_multiplier = 1.0,
  melee_cooldown_multiplier = 1.0,
  ranged_cooldown_multiplier = 1.0
}
local compile_bonuses
compile_bonuses = function(base, bonuses)
  local entry = table.clone(base)
  local adjective = table.concat(bonuses, " ")
  local bonus_objs = ((function()
    local _accum_0 = { }
    local _len_0 = 1
    for _index_0 = 1, #bonuses do
      local bonus = bonuses[_index_0]
      _accum_0[_len_0] = BONUSES[bonus]
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)())
  for _index_0 = 1, #bonus_objs do
    local bonus = bonus_objs[_index_0]
    if bonus.apply then
      bonus:apply(entry)
    end
  end
  for _index_0 = 1, #bonus_objs do
    local _des_0 = bonus_objs[_index_0]
    local shop_cost
    shop_cost = _des_0.shop_cost
    if shop_cost then
      entry.shop_cost = vector_add(entry.shop_cost, shop_cost)
    end
  end
  entry.shop_cost = vector_scale(entry.shop_cost, math.max(1, #bonuses))
  entry.name = tostring(adjective) .. " " .. tostring(entry.name)
  entry.console_draw_func = function(self, obj, get_next)
    for _index_0 = 1, #bonus_objs do
      local bonus = bonus_objs[_index_0]
      if bonus.console_draw_func then
        bonus:console_draw_func(obj, get_next)
      end
    end
  end
  if entry.auto_equip == nil then
    entry.auto_equip = true
  end
  for _index_0 = 1, #bonus_objs do
    local _des_0 = bonus_objs[_index_0]
    local auto_equip
    auto_equip = _des_0.auto_equip
    if auto_equip == false then
      entry.auto_equip = false
    end
  end
  entry.stat_bonuses = table.merge(DEFAULT_STAT_BONUSES, (entry.stat_bonuses or { }))
  for _index_0 = 1, #bonus_objs do
    local _des_0 = bonus_objs[_index_0]
    local stat_bonuses
    stat_bonuses = _des_0.stat_bonuses
    for k, v in pairs(stat_bonuses or { }) do
      if k:endswith("multiplier") then
        entry.stat_bonuses[k] = entry.stat_bonuses[k] * v
      else
        entry.stat_bonuses[k] = entry.stat_bonuses[k] + v
      end
    end
  end
  entry.item_draw_func = function(self, options, x, y)
    options = {
      color = options.color,
      scale = {
        0.5,
        0.5
      }
    }
    local locs = BONUS_LOCS
    for i, bonus in ipairs(bonus_objs) do
      if bonus.item_draw_func and locs[i] then
        local bx, by
        do
          local _obj_0 = locs[i]
          bx, by = _obj_0[1], _obj_0[2]
        end
        bonus:item_draw_func(options, x + bx, y + by)
      end
    end
  end
  entry.effects_granted = entry.effects_granted or { }
  for _index_0 = 1, #bonus_objs do
    local _des_0 = bonus_objs[_index_0]
    local effects_granted
    effects_granted = _des_0.effects_granted
    if effects_granted then
      entry.effects_granted = table.tconcat(entry.effects_granted, effects_granted)
    end
  end
  return entry
end
return {
  define_bonus = define_bonus,
  compile_bonuses = compile_bonuses,
  BONUSES = BONUSES
}
