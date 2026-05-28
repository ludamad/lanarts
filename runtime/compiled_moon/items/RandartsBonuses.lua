local DataWrapped = require("DataWrapped")
local BonusesAll = require("items.BonusesAll")
local compile_bonuses, BONUSES
do
  local _obj_0 = require("items.Bonuses")
  compile_bonuses, BONUSES = _obj_0.compile_bonuses, _obj_0.BONUSES
end
local TYPES = require("spells.TypeEffectUtils").TYPES
local _STAT_BONUSES = {
  enchantment = function(self, level)
    self.enchantment = (level - 1) * 3
    for i = 1, 3 do
      self.enchantment = self.enchantment + self.rng:random(0, 2)
    end
  end,
  mp = function(self, level)
    for i = 1, level do
      self.stat_bonuses.mp = self.stat_bonuses.mp + self.rng:random(0, 25)
    end
  end,
  hp = function(self, level)
    for i = 1, level do
      self.stat_bonuses.hp = self.stat_bonuses.hp + self.rng:random(0, 25)
    end
  end,
  mpregen = function(self, level)
    for i = 1, level do
      self.stat_bonuses.mpregen = self.stat_bonuses.mpregen + (self.rng:random(0, 300) / 100 / 60)
    end
  end,
  hpregen = function(self, level)
    for i = 1, level do
      self.stat_bonuses.hpregen = self.stat_bonuses.hpregen + (self.rng:random(0, 300) / 100 / 60)
    end
  end,
  spell_velocity_multiplier = function(self, level)
    for i = 1, level do
      self.stat_bonuses.spell_velocity_multiplier = self.stat_bonuses.spell_velocity_multiplier + self.rng:randomf(0, 0.15)
    end
  end
}
local _list_0 = {
  "strength",
  "magic",
  "defence",
  "willpower"
}
for _index_0 = 1, #_list_0 do
  local stat = _list_0[_index_0]
  _STAT_BONUSES[stat] = function(self, level)
    for i = 1, level do
      self.stat_bonuses[stat] = self.stat_bonuses[stat] + self.rng:random(0, 4)
    end
  end
end
local _STAT_MULTIPLIER_BONUSES = { }
local _list_1 = {
  "magic_cooldown_multiplier",
  "melee_cooldown_multiplier",
  "ranged_cooldown_multiplier"
}
for _index_0 = 1, #_list_1 do
  local stat = _list_1[_index_0]
  _STAT_MULTIPLIER_BONUSES[stat] = function(self, level)
    for i = 1, level do
      self.stat_multipliers[stat] = self.stat_multipliers[stat] + self.rng:randomf(0, 0.15)
    end
  end
end
local _EFFECT_BONUSES = {
  Fortification = function(self, level)
    return append(self.effects, {
      "Fortification",
      1
    })
  end,
  PossiblySummonCentaurOnKill = function(self, level)
    return append(self.effects, {
      "PossiblySummonCentaurOnKill",
      2.5
    })
  end,
  PossiblySummonGolemOnKill = function(self, level)
    return append(self.effects, {
      "PossiblySummonGolemOnKill",
      2.5
    })
  end,
  PossiblySummonStormElementalOnKill = function(self, level)
    return append(self.effects, {
      "PossiblySummonStormElementalOnKill",
      2
    })
  end,
  VampiricWeapon = function(self, level)
    return append(self.effects, {
      "VampiricWeapon",
      1
    })
  end,
  ConfusingWeapon = function(self, level)
    return append(self.effects, {
      "ConfusingWeapon",
      1
    })
  end,
  KnockbackWeapon = function(self, level)
    return append(self.effects, {
      "KnockbackWeapon",
      1
    })
  end,
  StunWeapon = function(self, level)
    return append(self.effects, {
      "StunWeapon",
      1
    })
  end,
  PoisonedWeapon = function(self, level)
    local poison_percentage = 0
    for i = 1, level do
      poison_percentage = poison_percentage + self.rng:randomf(-0.05, 0.10)
    end
    poison_percentage = math.max(0.05, poison_percentage)
    return append(self.effects, {
      {
        "PoisonedWeapon",
        {
          poison_percentage = poison_percentage
        }
      },
      poison_percentage / 0.05
    })
  end,
  RandomSpell = function(self, level)
    local spell = self.rng:random_choice({
      "Water Bolt",
      "Minor Missile",
      "Healing Aura"
    })
    if self._added_bonuses[spell] then
      return
    end
    self._added_bonuses[spell] = true
    return append(self.spells, {
      spell,
      1
    })
  end
}
local _list_2 = BONUSES.__keys
for _index_0 = 1, #_list_2 do
  local bonus = _list_2[_index_0]
  _EFFECT_BONUSES[bonus] = function(self, level)
    return append(self.newstyle_bonuses, bonus)
  end
end
for _index_0 = 1, #TYPES do
  local type = TYPES[_index_0]
  _EFFECT_BONUSES[tostring(type) .. "Resist"] = function(self, level)
    if self.rng:randomf() < 0.2 then
      self.resistances[type] = self.resistances[type] - 1
    else
      self.resistances[type] = self.resistances[type] + 1
      if self.rng:randomf() * level > 1.7 then
        self.resistances[type] = self.resistances[type] + 1
      end
      for i = 1, level do
        self.resistances[type] = self.resistances[type] + self.rng:random(-1, 2)
      end
    end
  end
  _EFFECT_BONUSES[tostring(type) .. "Power"] = function(self, level)
    for i = 1, level do
      self.powers[type] = self.powers[type] + self.rng:random(-2, 3)
    end
    self.powers[type] = math.max(self.powers[type], 0)
  end
end
local _BONUSES = { }
local _list_3 = {
  _STAT_BONUSES,
  _STAT_MULTIPLIER_BONUSES,
  _EFFECT_BONUSES
}
for _index_0 = 1, #_list_3 do
  local bonuses = _list_3[_index_0]
  for k, v in pairs(bonuses) do
    _BONUSES[k] = v
  end
end
local Bonuses = newtype({
  init = function(self, rng, level, base)
    self.rng = assert(rng)
    self.level = assert(level)
    self.base = assert(base)
    self.stat_bonuses = {
      mp = 0,
      hp = 0,
      hpregen = 0,
      mpregen = 0,
      strength = 0,
      defence = 0,
      willpower = 0,
      magic = 0,
      spell_velocity_multiplier = 1.0
    }
    self.stat_multipliers = {
      magic_cooldown_multiplier = 1.0,
      melee_cooldown_multiplier = 1.0,
      ranged_cooldown_multiplier = 1.0
    }
    self.effects = { }
    self.newstyle_bonuses = { }
    self.enchantment = 0
    self.powers = { }
    self.resistances = { }
    for _index_0 = 1, #TYPES do
      local type = TYPES[_index_0]
      self.powers[type] = 0
      self.resistances[type] = 0
    end
    self.spells = { }
    self._added_bonuses = { }
  end,
  add_bonus = function(self, bonus, level)
    if level == nil then
      level = self.level
    end
    if not BONUSES[bonus] and self._added_bonuses[bonus] then
      return false
    end
    _BONUSES[bonus](self, level)
    self._added_bonuses[bonus] = true
    return true
  end,
  validate = function(self, min_score, max_score)
    local dims = {
      self.stat_bonuses.mp / 20,
      self.stat_bonuses.hp / 20,
      self.stat_bonuses.hpregen / (2.0 / 60),
      self.stat_bonuses.mpregen / (2.0 / 60),
      self.stat_bonuses.strength,
      self.stat_bonuses.defence,
      self.stat_bonuses.willpower,
      self.stat_bonuses.magic,
      self.enchantment / 2,
      (1 - self.stat_bonuses.spell_velocity_multiplier) / 0.25
    }
    for k, v in pairs(self.stat_multipliers) do
      append(dims, -(1 - v) / 0.05)
    end
    local n_resists = 0
    for k, v in pairs(self.resistances) do
      if v ~= 0 then
        n_resists = n_resists + 1
      end
      append(dims, v)
    end
    local n_powers = 0
    for k, v in pairs(self.powers) do
      if v ~= 0 then
        n_powers = n_powers + 1
      end
      append(dims, v)
    end
    local _list_4 = self.effects
    for _index_0 = 1, #_list_4 do
      local _des_0 = _list_4[_index_0]
      local effect, v
      effect, v = _des_0[1], _des_0[2]
      append(dims, v)
    end
    local _list_5 = self.newstyle_bonuses
    for _index_0 = 1, #_list_5 do
      local bonus = _list_5[_index_0]
      append(dims, (self.rng:randomf() + 0.5) * BONUSES[bonus].shop_cost[1] / 50)
    end
    local _list_6 = self.spells
    for _index_0 = 1, #_list_6 do
      local _des_0 = _list_6[_index_0]
      local spell, v
      spell, v = _des_0[1], _des_0[2]
      append(dims, v)
    end
    local effective_bonuses = 0
    local penalties = 0
    local score = 0
    for _index_0 = 1, #dims do
      local dim = dims[_index_0]
      local sign = 1
      if dim <= 0 then
        sign = -1
      end
      if math.abs(dim) > 0.25 then
        effective_bonuses = effective_bonuses + 1
      end
      if dim < 0 then
        penalties = penalties + 1
      end
      score = score + (sign * dim * dim * 100)
    end
    if score < min_score or score > max_score then
      return false
    end
    return true, score
  end,
  create_definer_object = function(self, name, description)
    local base = table.deep_clone(self.base)
    base.name = name
    base.description = description
    base.stat_bonuses = base.stat_bonuses or { }
    for k, v in pairs(self.stat_bonuses) do
      base.stat_bonuses[k] = base.stat_bonuses[k] or 0
      base.stat_bonuses[k] = base.stat_bonuses[k] + v
    end
    for k, v in pairs(self.stat_multipliers) do
      base[k] = base[k] or 1
      base[k] = base[k] * v
    end
    base.effects_granted = base.effects_granted or { }
    local _list_4 = self.effects
    for _index_0 = 1, #_list_4 do
      local _des_0 = _list_4[_index_0]
      local effect, v
      effect, v = _des_0[1], _des_0[2]
      append(base.effects_granted, effect)
    end
    DataWrapped.power_effects(self.powers, base.effects_granted)
    DataWrapped.resistance_effects(self.resistances, base.effects_granted)
    base.spells_granted = base.spells_granted or { }
    local _list_5 = self.spells
    for _index_0 = 1, #_list_5 do
      local _des_0 = _list_5[_index_0]
      local spell, v
      spell, v = _des_0[1], _des_0[2]
      append(base.spells_granted, spell)
    end
    if self.enchantment > 0 then
      for i = 1, 2 do
        base.damage.base[i] = math.ceil(base.damage.base[i] * (1 + self.enchantment * 0.1))
      end
      base.name = "+" .. tostring(self.enchantment) .. " " .. tostring(base.name)
    end
    base.shop_cost = base.shop_cost or {
      0,
      0
    }
    local created = compile_bonuses(base, self.newstyle_bonuses)
    created.name = base.name
    return created
  end
})
return {
  Bonuses = Bonuses
}
