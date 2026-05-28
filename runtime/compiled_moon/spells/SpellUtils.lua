local EventLog = require("ui.EventLog")
local GameObject = require("core.GameObject")
local GameState = require("core.GameState")
local Map = require("core.Map")
local Display = require("core.Display")
local DataW = require("DataWrapped")
local LuaGameObject = require("objects.LuaGameObject")
local ObjectUtils = require("objects.ObjectUtils")
local TypeEffectUtils = require("spells.TypeEffectUtils")
local do_damage
do_damage = function(caster, types, target, damage, power, magic_percentage)
  if power == nil then
    power = 0
  end
  if magic_percentage == nil then
    magic_percentage = 1
  end
  local stats = caster:effective_stats()
  power = power + (magic_percentage * stats.magic + (1 - magic_percentage) * stats.strength)
  for _index_0 = 1, #types do
    local type = types[_index_0]
    power = power + TypeEffectUtils.get_power(caster, type)
    damage = damage * TypeEffectUtils.get_resistance(target, type)
  end
  return target:damage(damage, power, magic_percentage, caster)
end
local spell_object_type
spell_object_type = function(T)
  local wrapped_on_init = T.init
  T.types = T.types or { }
  T.init = function(self, args)
    local caster, duration, xy
    caster, duration, xy = args.caster, args.duration, args.xy
    self.caster = assert(caster)
    self.time_left = assert(duration)
    self.total_time = assert(duration)
    T.parent_init(self, xy or self.caster.xy)
    if wrapped_on_init then
      return wrapped_on_init(self, args)
    end
  end
  T.time_passed = function(self)
    return self.total_time - self.time_left
  end
  T.percent_passed = function(self)
    return self:time_passed() / self.total_time
  end
  T.in_time_slice = function(self, frac1, frac2)
    return (self:time_passed() >= frac1 * self.total_time and self:time_passed() < frac2 * self.total_time)
  end
  T.every = function(self, duration)
    return self:time_passed() % duration == 0
  end
  T.caster_type_power = function(self)
    local power = 0
    local _list_0 = self.types
    for _index_0 = 1, #_list_0 do
      local type = _list_0[_index_0]
      power = power + TypeEffectUtils.get_power(self.caster, type)
    end
    return power
  end
  T.do_damage = function(self, target, damage, power, magic_percentage)
    if power == nil then
      power = 0
    end
    if magic_percentage == nil then
      magic_percentage = 1
    end
    return do_damage(self.caster, self.types, target, damage, power, magic_percentage)
  end
  T.message = function(self, msg, color)
    for _ in screens() do
      if self.caster:is_local_player() then
        EventLog.add(msg, color)
      end
    end
  end
  T.get_progress = function(self, frac1, frac2)
    local progress = (self:time_passed() / self.total_time - frac1) / (frac2 - frac1)
    return math.max(0, math.min(progress, 1))
  end
  local wrapped_on_step = T.on_step
  T.on_step = function(self)
    if self.caster.destroyed or self.time_left <= 0 then
      GameObject.destroy(self)
      return
    end
    self.time_left = self.time_left - 1
    if wrapped_on_step then
      return wrapped_on_step(self)
    end
  end
  return LuaGameObject.type_create(T)
end
local passing_projectile
passing_projectile = function(args)
  local base = {
    can_pass_through = true,
    deals_special_damge = true
  }
  local combined = table.merge(base, args)
  combined.on_map_init = function(self)
    self.n_steps = args.n_steps
    self.attacked_map = OrderedDict()
    self.n_hits = 0
    if args.on_map_init then
      return args.on_map_init(self)
    end
  end
  combined.on_step = function(self)
    self.n_steps = self.n_steps - 1
    if self.n_steps <= 0 then
      GameObject.destroy(self)
      return
    end
    self.attacked_map = self.attacked_map or OrderedDict()
    self.n_hits = self.n_hits or 0
    for k, v in strictpairs(self.attacked_map) do
      if v == 0 then
        self.attacked_map[k] = nil
      else
        self.attacked_map[k] = v - 1
      end
    end
    if args.on_step then
      return args.on_step(self)
    end
  end
  combined.on_hit_func = function(self, target, atkstats)
    if args.on_hit_func then
      args.on_hit_func(self, target, atkstats)
    end
    if self.destroyed or self.attacked_map[target] ~= nil then
      atkstats.damage = 0
      return
    end
    self.attacked_map[target] = args.redamage_cooldown
    self.n_hits = self.n_hits + 1
    atkstats.type_multiplier = atkstats.type_multiplier * (1.0 + (math.sqrt(self.n_hits - 1) - 1.0) / 20)
  end
  return combined
end
local for_all_visible_mons
for_all_visible_mons = function(caster, f)
  local _list_0 = (Map.enemies_list(caster))
  for _index_0 = 1, #_list_0 do
    local _continue_0 = false
    repeat
      local mon = _list_0[_index_0]
      if not Map.object_visible(mon, mon.xy, caster) then
        _continue_0 = true
        break
      end
      f(mon)
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return nil
end
local message
message = function(user, msg, color)
  for _ in screens() do
    if user:is_local_player() then
      EventLog.add(msg, color)
    end
  end
end
local mon_title
mon_title = function(mon)
  if mon.unique then
    return mon.name
  else
    return "the " .. tostring(mon.name)
  end
end
return {
  spell_object_type = spell_object_type,
  mon_title = mon_title,
  message = message,
  passing_projectile = passing_projectile,
  do_damage = do_damage,
  for_all_visible_mons = for_all_visible_mons
}
