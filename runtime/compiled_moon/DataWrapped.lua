local TypeEffectUtils = require("spells.TypeEffectUtils")
local GameObject = require("core.GameObject")
local draw_console_effect
draw_console_effect = function(sprite, text, xy, color)
  if color == nil then
    color = COL_PALE_YELLOW
  end
  local Map = require("core.Map")
  local Display = require("core.Display")
  sprite:draw({
    origin = Display.LEFT_CENTER
  }, {
    xy[1],
    xy[2] + 4
  })
  return font_cached_load(settings.font, 10):draw({
    color = color,
    origin = Display.LEFT_CENTER
  }, {
    xy[1] + Map.TILE_SIZE + 4,
    xy[2]
  }, text)
end
local effect_create
effect_create = function(args)
  local wrapped_init = args.init_func
  local wrapped_step = args.step_func
  local wrapped_apply = args.apply_func
  local wrapped_remove = args.remove_func
  args.init_func = function(self, obj)
    self.time_left = 0
    self.active = false
    self.n_derived = 0
    if wrapped_init then
      return wrapped_init(self, obj)
    end
  end
  args.apply_derived_func = args.apply_derived_func or function(self, obj, args)
    self.active = true
    self.n_derived = self.n_derived + 1
    if wrapped_apply then
      return wrapped_apply(self, obj, args)
    end
  end
  args.remove_derived_func = args.remove_derived_func or function(self, obj, args)
    self.n_derived = 0
    self.active = (self.time_left > 0)
  end
  args.apply_buff_func = args.apply_buff_func or function(self, obj, args)
    self.active = true
    if wrapped_apply then
      wrapped_apply(self, obj, args)
    end
    local time_left
    if type(args) == 'number' then
      time_left = args
    else
      time_left = args.time_left
    end
    self.time_left = math.max(self.time_left, time_left)
  end
  args.remove_func = function(self, obj)
    if wrapped_remove then
      wrapped_remove(self, obj)
    end
    return self:init_func(obj)
  end
  args.step_func = function(self, obj)
    if wrapped_step then
      wrapped_step(self, obj)
    end
    self.time_left = math.max(self.time_left - 1, 0)
    if self.time_left <= 0 and self.n_derived == 0 then
      self:remove_func(obj)
      self.time_left = 0
    end
  end
  Data.effect_create(args)
  return yield_point()
end
local _subeffect_effect_create
_subeffect_effect_create = function(args, starting_value, accum)
  local wrapped_init = args.init_func
  local wrapped_step = args.step_func
  local wrapped_apply = args.apply_func
  local wrapped_remove = args.remove_func
  args.init_func = function(self, obj)
    self.active = false
    self.subeffects = { }
    self.current = starting_value
    self.time_left = 0
  end
  args.apply_derived_func = args.apply_derived_func or function(self, obj, args)
    self.active = true
    self.current = accum(self.current, args)
  end
  args.remove_derived_func = args.remove_derived_func or function(self, obj, args)
    self.active = (#self.subeffects > 0)
    self.current = starting_value
  end
  args.apply_buff_func = args.apply_buff_func or function(self, obj, args)
    self.active = true
    assert(type(args.time_left) == 'number')
    return append(self.subeffects, table.clone(args))
  end
  args.remove_func = function(self, obj)
    self.active = false
    table.clear(self.subeffects)
    self.current = starting_value
  end
  args.value = function(self)
    local x = self.current
    local _list_0 = self.subeffects
    for _index_0 = 1, #_list_0 do
      local eff = _list_0[_index_0]
      x = accum(x, eff)
    end
    return x
  end
  args.step_func = function(self, obj)
    local filter = false
    local _list_0 = self.subeffects
    for _index_0 = 1, #_list_0 do
      local eff = _list_0[_index_0]
      if wrapped_step then
        wrapped_step(self, obj, eff)
      end
      eff.time_left = math.max(eff.time_left - 1, 0)
      if eff.time_left <= 0 then
        filter = true
      end
    end
    if filter then
      self.subeffects = table.filter(self.subeffects, function(x)
        return x.time_left > 0
      end)
    end
    if self.subeffects == 0 and self.current ~= starting_value then
      return self:remove_func(obj)
    end
  end
  Data.effect_create(args)
  return yield_point()
end
local additive_effect_create
additive_effect_create = function(args)
  local key = args.key or 'value'
  local accum
  accum = function(x, next)
    return x + next[key]
  end
  return _subeffect_effect_create(args, 0, accum)
end
local STANDARD_WEAPON_DPS = 10
local STANDARD_RANGED_DPS = 7
local add_console_draw_func
add_console_draw_func = function(args, f1)
  local f2 = args.console_draw_func
  if f2 == nil then
    args.console_draw_func = f1
  else
    args.console_draw_func = function(...)
      f1(...)
      return f2(...)
    end
  end
end
local add_draw_func
add_draw_func = function(args, f1)
  local f2 = args.draw_func
  if f2 == nil then
    args.draw_func = f1
  else
    args.draw_func = function(...)
      f1(...)
      return f2(...)
    end
  end
end
local add_target_draw_func
add_target_draw_func = function(args, f1)
  local f2 = args.target_draw_func
  if f2 == nil then
    args.target_draw_func = f1
  else
    args.target_draw_func = function(...)
      f1(...)
      return f2(...)
    end
  end
end
local add_on_hit_func
add_on_hit_func = function(args, f1)
  local f2 = args.on_hit_func
  if f2 == nil then
    args.on_hit_func = f1
  else
    args.on_hit_func = function(obj, target, atkstats)
      f1(obj, target, atkstats)
      return f2(obj, target, atkstats)
    end
  end
end
local add_attack_stat_func
add_attack_stat_func = function(args, f1)
  local f2 = args.attack_stat_func
  if f2 == nil then
    args.attack_stat_func = f1
  else
    args.attack_stat_func = function(obj, target, atkstats)
      f1(obj, target, atkstats)
      return f2(obj, target, atkstats)
    end
  end
end
local add_cooldown_multiplier
add_cooldown_multiplier = function(args, f1)
  local f2 = args.cooldown_multiplier
  if f2 == nil then
    args.cooldown_multiplier = f1
  else
    args.cooldown_multiplier = function(self)
      f1(self)
      return f2(self)
    end
  end
end
local add_types
add_types = function(args, types)
  for _index_0 = 1, #types do
    local type = types[_index_0]
    assert(table.contains(TypeEffectUtils.TYPES, type), "Invalid type!")
    add_on_hit_func(args, function(obj, target, atkstats)
      atkstats.power = atkstats.power + TypeEffectUtils.get_power(obj, type)
      atkstats.type_multiplier = atkstats.type_multiplier * TypeEffectUtils.get_resistance(target, type)
    end)
  end
end
local power_effects
power_effects = function(powers, effects)
  if effects == nil then
    effects = { }
  end
  for type, power in spairs(powers) do
    local _continue_0 = false
    repeat
      assert(table.contains(TypeEffectUtils.TYPES, type), "Invalid type!")
      if power == 0 then
        _continue_0 = true
        break
      end
      append(effects, {
        tostring(type) .. "Power",
        {
          power = power
        }
      })
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return effects
end
local resistance_effects
resistance_effects = function(resists, effects)
  if effects == nil then
    effects = { }
  end
  for type, resist in spairs(resists) do
    local _continue_0 = false
    repeat
      assert(table.contains(TypeEffectUtils.TYPES, type), "Invalid type!")
      if resist == 0 then
        _continue_0 = true
        break
      end
      append(effects, {
        tostring(type) .. "Resist",
        {
          resist = resist
        }
      })
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return effects
end
local weapon_create
weapon_create = function(args, for_enemy)
  if for_enemy == nil then
    for_enemy = false
  end
  local damage_multiplier = args.damage_multiplier or 1.0
  local dps
  if args.type == "bows" then
    dps = STANDARD_RANGED_DPS
  else
    dps = STANDARD_WEAPON_DPS
  end
  local damage = damage_multiplier * (args.cooldown / 60 * dps)
  local power = 0
  if type(args.damage) == 'number' then
    damage = args.damage
    args.damage = {
      base = {
        math.floor(damage * 0.9),
        math.ceil(damage * 1.1)
      },
      strength = 0
    }
  else
    args.damage = args.damage or {
      base = {
        math.floor(damage),
        math.ceil(damage)
      },
      strength = 0
    }
  end
  if args.types ~= nil then
    add_types(args, args.types)
  end
  args.power = args.power or {
    base = {
      power,
      power
    },
    strength = 1
  }
  args.range = args.range or 7
  local _list_0 = (args.types or { })
  for _index_0 = 1, #_list_0 do
    local type = _list_0[_index_0]
    add_console_draw_func(args, function(entry, inst, get_next)
      local E = require("spells.Effects")
      local _exp_0 = type
      if "Red" == _exp_0 then
        return draw_console_effect(E._fire_power, "Red Type", get_next())
      elseif "Blue" == _exp_0 then
        return draw_console_effect(E._ice_power, "Blue Type", get_next())
      elseif "White" == _exp_0 then
        return draw_console_effect(E._storm_power, "White Type", get_next())
      elseif "Black" == _exp_0 then
        return draw_console_effect(E._black_power, "Black Type", get_next())
      elseif "Green" == _exp_0 then
        return draw_console_effect(E._poison_power, "Green Type", get_next())
      end
    end)
  end
  Data.weapon_create(args)
  return yield_point()
end
local spell_create
spell_create = function(args)
  local proj = args.projectile
  if proj then
    local damage_multiplier = proj.damage_multiplier or 1.0
    local damage = damage_multiplier * (args.cooldown / 60 * STANDARD_RANGED_DPS)
    proj.name = proj.name or args.name
    proj.weapon_class = proj.weapon_class or "magic"
    proj.spr_item = proj.spr_item or "none"
    proj.spr_attack = proj.spr_attack or args.spr_spell
    proj.cooldown = proj.cooldown or args.cooldown
    proj.range = proj.range or 300
    proj.types = proj.types or args.types
    proj.damage_type = proj.damage_type or {
      magic = 1.0
    }
    proj.damage = proj.damage or {
      base = {
        math.floor(damage),
        math.ceil(damage)
      },
      strength = 0
    }
    proj.power = proj.power or {
      base = {
        0,
        0
      },
      magic = 1
    }
    if proj.types ~= nil then
      add_types(proj, proj.types)
    end
    Data.projectile_create(proj)
    yield_point()
    args.projectile = proj.name
  end
  if args.types then
    local _list_0 = args.types
    for _index_0 = 1, #_list_0 do
      local type = _list_0[_index_0]
      add_cooldown_multiplier(args, function(self)
        return 1 - TypeEffectUtils.get_power(self, type) * 0.03
      end)
    end
  end
  local _list_0 = (args.types or { })
  for _index_0 = 1, #_list_0 do
    local type = _list_0[_index_0]
    add_console_draw_func(args, function(caster, get_next)
      local E = require("spells.Effects")
      local _exp_0 = type
      if "Red" == _exp_0 then
        return draw_console_effect(E._fire_power, "Red Type", get_next())
      elseif "Blue" == _exp_0 then
        return draw_console_effect(E._ice_power, "Blue Type", get_next())
      elseif "White" == _exp_0 then
        return draw_console_effect(E._storm_power, "White Type", get_next())
      elseif "Black" == _exp_0 then
        return draw_console_effect(E._black_power, "Black Type", get_next())
      elseif "Green" == _exp_0 then
        return draw_console_effect(E._poison_power, "Green Type", get_next())
      end
    end)
  end
  Data.spell_create(args)
  return yield_point()
end
local projectile_create
projectile_create = function(args, for_enemy)
  if for_enemy == nil then
    for_enemy = false
  end
  if args.cooldown then
    local damage_multiplier = args.damage_multiplier or 1.0
    local damage = damage_multiplier * (args.cooldown / 60 * STANDARD_WEAPON_DPS)
    if for_enemy then
      args.power = args.power or {
        base = {
          0,
          0
        }
      }
      args.damage = args.damage or {
        base = {
          0,
          0
        },
        strength = args.damage_type.physical,
        magic = args.damage_type.magic
      }
    else
      args.damage = args.damage or {
        base = {
          math.floor(damage),
          math.ceil(damage)
        },
        strength = 0
      }
      args.power = args.power or {
        base = {
          0,
          0
        },
        strength = args.damage_type.physical,
        magic = args.damage_type.magic
      }
    end
  end
  args.spr_item = args.spr_item or "none"
  args.range = args.range or 300
  if args.types ~= nil then
    add_types(args, args.types)
  end
  Data.projectile_create(args)
  return yield_point()
end
local DERIVED_ENEMY_TYPES = { }
local enemy_create
enemy_create = function(args)
  args.stats.attacks = args.stats.attacks or { }
  local w = args.weapon
  if w ~= nil then
    w.name = w.name or (args.name .. " Melee")
    w.type = w.type or "unarmed"
    w.spr_item = w.spr_item or "none"
    w.types = w.types or args.types
    append(args.stats.attacks, {
      weapon = w.name
    })
    weapon_create(w, true)
  end
  local p = args.projectile
  if p ~= nil then
    p.name = p.name or (args.name .. " Projectile")
    p.spr_item = p.spr_item or "none"
    append(args.stats.attacks, {
      projectile = p.name
    })
    p.types = p.types or args.types
    projectile_create(p, true)
  end
  if args.resistances == nil and args.types ~= nil then
    args.resistances = TypeEffectUtils.get_monster_resistances(args.types)
  end
  if args.resistances ~= nil then
    args.effects_active = args.effects_active or { }
    resistance_effects(args.resistances, args.effects_active)
  end
  local _list_0 = (args.types or { })
  for _index_0 = 1, #_list_0 do
    local type = _list_0[_index_0]
    local _exp_0 = type
    if "Red" == _exp_0 then
      args.flammability = 0.5
      args.poisonability = 1
    elseif "Blue" == _exp_0 then
      args.flammability = 0.5
      args.poisonability = 1
    elseif "Black" == _exp_0 then
      args.flammability = 1
      args.poisonability = 1
    elseif "White" == _exp_0 then
      args.flammability = 1
      args.poisonability = 1
    elseif "Green" == _exp_0 then
      args.flammability = 1.5
      args.poisonability = 0.5
    end
    add_console_draw_func(args, function(edata, get_next)
      local E = require("spells.Effects")
      local _exp_1 = type
      if "Red" == _exp_1 then
        return draw_console_effect(E._fire_power, "Red Type", get_next())
      elseif "Blue" == _exp_1 then
        return draw_console_effect(E._ice_power, "Blue Type", get_next())
      elseif "White" == _exp_1 then
        return draw_console_effect(E._storm_power, "White Type", get_next())
      elseif "Black" == _exp_1 then
        return draw_console_effect(E._black_power, "Black Type", get_next())
      elseif "Green" == _exp_1 then
        return draw_console_effect(E._poison_power, "Green Type", get_next())
      end
    end)
  end
  if args.methods then
    for k, v in pairs(args.methods) do
      assert(type(k) == "string", "Method key should be string!")
      assert(type(v) == "function", "Method value should be function!")
    end
    local derived_type = GameObject.EnemyType.clone()
    table.merge_into(derived_type.__constants, args.methods)
    DERIVED_ENEMY_TYPES[args.name] = derived_type
    local wrapped_init = args.init_func
    args.init_func = function(self)
      setmetatable(self, derived_type)
      if __DEBUG_CHECKS then
        for k, v in pairs(args.methods) do
          assert(self[k] == v)
        end
      end
      if wrapped_init then
        return wrapped_init(self)
      end
    end
  end
  Data.enemy_create(args)
  return yield_point()
end
return {
  additive_effect_create = additive_effect_create,
  effect_create = effect_create,
  weapon_create = weapon_create,
  spell_create = spell_create,
  projectile_create = projectile_create,
  enemy_create = enemy_create,
  resistance_effects = resistance_effects,
  power_effects = power_effects,
  add_types = add_types,
  DERIVED_ENEMY_TYPES = DERIVED_ENEMY_TYPES
}
