local TYPE_RESIST_EFFECT = { }
local TYPE_POWER_EFFECT = { }
local TYPES = {
  "Red",
  "Blue",
  "Green",
  "White",
  "Black",
  "Slashing",
  "Piercing",
  "Bludgeon"
}
for _index_0 = 1, #TYPES do
  local type = TYPES[_index_0]
  TYPE_RESIST_EFFECT[type] = tostring(type) .. "Resist"
  TYPE_POWER_EFFECT[type] = tostring(type) .. "Power"
end
local get_monster_resistances
get_monster_resistances = function(types)
  local resists = {
    ["Red"] = 0,
    ["Blue"] = 0,
    ["Green"] = 0,
    ["White"] = 0,
    ["Black"] = 0,
    ["Slashing"] = 0,
    ["Piercing"] = 0,
    ["Bludgeon"] = 0
  }
  for _index_0 = 1, #types do
    local type = types[_index_0]
    if type == "Red" then
      resists.Red = resists.Red + 1
      resists.Blue = resists.Blue - 1
    elseif type == "Black" then
      resists.Black = resists.Black + 1
      resists.White = resists.White - 1
    elseif type == "White" then
      resists.White = resists.White + 1
      resists.Black = resists.Black - 1
    elseif type == "Green" then
      resists.Green = resists.Green + 1
      resists.Red = resists.Red - 1
      resists.Black = resists.Black - 1
    elseif type == "Blue" then
      resists.Blue = resists.Blue + 1
      resists.Green = resists.Green - 1
    end
  end
  return resists
end
local get_effect_stat
get_effect_stat = function(obj, type, default)
  if default == nil then
    default = 0
  end
  assert(obj and type and default)
  if obj:has_effect(type) then
    return obj:get_effect(type):value()
  end
  return default
end
local COLORS = {
  'White',
  'Black',
  'Green',
  'Red',
  'Blue'
}
local get_bonus
get_bonus = function(obj, bonus)
  local color = bonus:match("^(%a+)Bonus$") or bonus
  if not table.contains(COLORS, color) then
    return 0
  end
  local eff = obj:get_effect(bonus)
  if eff then
    return eff.n_derived
  else
    return 0
  end
end
local get_power
get_power = function(obj, type)
  return get_effect_stat(obj, TYPE_POWER_EFFECT[type]) + get_bonus(obj, tostring(type) .. "Bonus")
end
local get_resistance
get_resistance = function(obj, type)
  local raw_resist = get_effect_stat(obj, TYPE_RESIST_EFFECT[type])
  if raw_resist <= -100 then
    return 1.5
  end
  if raw_resist >= 100 then
    return 0.5
  end
  local resist = math.max(math.min(math.floor(raw_resist), 5), -5)
  return 1 - resist / 10
end
local hit_func
hit_func = function(type, attacker, target, atkstats)
  local power, damage, magic_percentage
  power, damage, magic_percentage = atkstats.power, atkstats.damage, atkstats.magic_percentage
  power = power + get_power(attacker, type)
  local type_multiplier = get_resistance(target, type)
  return target:damage(damage, power, magic_percentage, attacker, type_multiplier)
end
return {
  get_effect_stat = get_effect_stat,
  get_resistance = get_resistance,
  get_power = get_power,
  TYPES = TYPES,
  get_monster_resistances = get_monster_resistances
}
