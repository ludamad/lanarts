local EventLog = require("ui.EventLog")
local GameObject = require("core.GameObject")
local GameState = require("core.GameState")
local Map = require("core.Map")
local World = require("core.World")
local Bresenham = require("core.Bresenham")
local Display = require("core.Display")
local SpellObjects = require("objects.SpellObjects")
local DataW = require("DataWrapped")
local TypeEffectUtils = require("spells.TypeEffectUtils")
local BonusesUtils = require("items.BonusesUtils")
local ImpactFX = require("ui.ImpactFX")
local ProceduralEffects = require("ui.ProceduralEffects")
local damage_source_name
damage_source_name = function(attacker, defender)
  local source = "Unattributed"
  if attacker then
    if attacker == defender then
      source = "Self"
    elseif attacker.name and attacker.name ~= "" then
      source = attacker.name
    end
  end
  return source
end
DataW.effect_create({
  name = "BalanceDamageTracker",
  apply_func = function(self, obj, args)
    self.telemetry = args and args.telemetry
    self.damage_by_source_key = (args and args.damage_by_source_key) or "damage_taken_by_source"
    self.events_by_source_key = (args and args.events_by_source_key) or "damage_events_by_source"
    self.damage_total_key = (args and args.damage_total_key) or "damage_taken_total"
    self.event_total_key = (args and args.event_total_key) or "damage_event_total"
  end,
  on_receive_damage_func = function(self, attacker, defender, damage)
    local telemetry = self.telemetry
    if not telemetry or not damage or damage <= 0 then
      return damage
    end
    local source = damage_source_name(attacker, defender)
    local damage_by_source = telemetry[self.damage_by_source_key] or { }
    local events_by_source = telemetry[self.events_by_source_key] or { }
    telemetry[self.damage_by_source_key] = damage_by_source
    telemetry[self.events_by_source_key] = events_by_source
    damage_by_source[source] = (damage_by_source[source] or 0) + damage
    events_by_source[source] = (events_by_source[source] or 0) + 1
    telemetry[self.damage_total_key] = (telemetry[self.damage_total_key] or 0) + damage
    telemetry[self.event_total_key] = (telemetry[self.event_total_key] or 0) + 1
    return damage
  end
})
DataW.effect_create({
  name = "BalanceDamageDealtTracker",
  apply_func = function(self, obj, args)
    self.telemetry = args and args.telemetry
    self.attacker_team = args and args.attacker_team
    self.damage_by_target_key = (args and args.damage_by_target_key) or "damage_dealt_by_target"
    self.events_by_target_key = (args and args.events_by_target_key) or "damage_dealt_events_by_target"
    self.damage_by_source_key = (args and args.damage_by_source_key) or "damage_dealt_by_source"
    self.events_by_source_key = (args and args.events_by_source_key) or "damage_dealt_events_by_source"
    self.damage_total_key = (args and args.damage_total_key) or "damage_dealt_total"
    self.event_total_key = (args and args.event_total_key) or "damage_dealt_event_total"
  end,
  on_receive_damage_func = function(self, attacker, defender, damage)
    local telemetry = self.telemetry
    if not telemetry or not damage or damage <= 0 then
      return damage
    end
    if self.attacker_team ~= nil and (not attacker or attacker.team ~= self.attacker_team) then
      return damage
    end
    local source = damage_source_name(attacker, defender)
    local target
    if defender and defender.name and defender.name ~= "" then
      target = defender.name
    else
      target = "Unattributed"
    end
    local damage_by_target = telemetry[self.damage_by_target_key] or { }
    local events_by_target = telemetry[self.events_by_target_key] or { }
    local damage_by_source = telemetry[self.damage_by_source_key] or { }
    local events_by_source = telemetry[self.events_by_source_key] or { }
    telemetry[self.damage_by_target_key] = damage_by_target
    telemetry[self.events_by_target_key] = events_by_target
    telemetry[self.damage_by_source_key] = damage_by_source
    telemetry[self.events_by_source_key] = events_by_source
    damage_by_target[target] = (damage_by_target[target] or 0) + damage
    events_by_target[target] = (events_by_target[target] or 0) + 1
    damage_by_source[source] = (damage_by_source[source] or 0) + damage
    events_by_source[source] = (events_by_source[source] or 0) + 1
    telemetry[self.damage_total_key] = (telemetry[self.damage_total_key] or 0) + damage
    telemetry[self.event_total_key] = (telemetry[self.event_total_key] or 0) + 1
    return damage
  end
})
DataW.effect_create({
  name = "Poison",
  effected_sprite = "spr_effects.poison",
  poison_rate = 35,
  effected_colour = {
    100,
    255,
    100
  },
  fade_out = 4,
  can_use_rest = false,
  stat_func = function(self, obj, old, new)
    new.defence = math.max(0, new.defence - 5)
    new.willpower = math.max(0, new.willpower - 3)
    if not obj.is_enemy then
      new.speed = new.speed / 2
    end
  end,
  apply_func = function(self, obj, args)
    self.steps = 0
    self.attacker = assert(args.attacker)
    self.damage, self.power = assert(args.damage), assert(args.power)
    self.magic_percentage = args.magic_percentage or 0.0
    self.poison_rate = assert(args.poison_rate)
  end,
  step_func = function(self, obj)
    if self.steps < self.poison_rate then
      self.steps = self.steps + 1
    else
      self.steps = 0
      return obj:damage(self.damage / 2, self.power, self.magic_percentage, self.attacker, (TypeEffectUtils.get_resistance(obj, "Green")))
    end
  end
})
DataW.effect_create({
  name = "Berserk",
  exhausted_duration = 300,
  can_use_rest = false,
  can_use_spells = true,
  can_use_stairs = true,
  fade_out = 5,
  effected_colour = {
    255,
    160,
    160
  },
  effected_sprite = "berserk_effected",
  on_melee_func = function(self, attacker, defender, damage)
    if attacker.stats.level >= 2 then
      EventLog.add("Your rage grows from combat...", {
        200,
        200,
        255
      })
      self.time_left = math.min(self.max_time * 1.5, self.time_left + 5)
    end
    if chance(.1) then
      defender:add_effect("Stun", 20 + attacker.stats.level)
    end
    return damage
  end,
  stat_func = function(self, obj, old, new)
    new.strength = new.strength + obj.stats.level
    new.defence = math.max(0, new.defence + obj.stats.level)
    new.willpower = math.max(0, new.willpower + obj.stats.level)
    new.melee_cooldown_multiplier = new.melee_cooldown_multiplier / 1.25
    if obj:has_effect("AmuletBerserker") then
      new.hpregen = new.hpregen + (8 / 60)
    end
    if new.speed < 6 then
      new.speed = math.max(new.speed * 1.25, 6)
    end
    return obj:reset_rest_cooldown()
  end,
  apply_func = function(self, obj, time_left)
    self.kill_tracker = obj.kills
    self.max_time = math.max(self.max_time or 0, time_left)
    self.extensions = 0
  end,
  remove_func = function(self, obj)
    obj:add_effect("Exhausted", self.exhausted_duration)
    for _ in screens() do
      if obj:is_local_player() then
        play_sound("sound/exhausted.ogg")
        EventLog.add("You are now exhausted...", {
          255,
          200,
          200
        })
      end
    end
  end,
  step_func = function(self, obj)
    if self.extensions >= 10 then
      for _ in screens() do
        if obj:is_local_player() then
          EventLog.add("Your rage is tickled... but cannot grow further.", {
            200,
            200,
            255
          })
        end
      end
      return
    end
    local diff = math.max(obj.kills - self.kill_tracker, 0)
    for i = 1, diff do
      self.time_left = math.min(self.max_time * 1.5, self.time_left + 20)
      for _ in screens() do
        if obj:is_local_player() then
          EventLog.add("Your rage grows from killing...", {
            200,
            200,
            255
          })
          play_sound("sound/swish-11.wav")
        end
        if settings.verbose_output then
          EventLog.add("Killed Enemy, berserk time_left = " .. self.time_left)
        end
      end
      if obj:has_effect("AmuletBerserker") and chance(0.05) then
        play_sound("sound/summon.ogg")
        local eff = obj:add_effect("Summoning", 20)
        eff.monster = "Centaur Hunter"
        eff.duration = 5
      end
      self.extensions = self.extensions + 1
    end
    self.kill_tracker = obj.kills
  end
})
DataW.effect_create({
  name = "Exhausted",
  can_use_rest = false,
  effected_colour = {
    120,
    120,
    120
  },
  effected_sprite = "exhausted_effected",
  stat_func = function(self, obj, old, new)
    new.speed = new.speed / 2
    new.defence = math.max(0, new.defence - 3)
    new.willpower = math.max(0, new.willpower - 3)
    new.cooldown_mult = new.cooldown_mult * 1.25
    return obj:reset_rest_cooldown()
  end,
  remove_func = function(self, obj)
    for _ in screens() do
      if obj:is_local_player() then
        EventLog.add("You are no longer exhausted.", {
          200,
          200,
          255
        })
      end
    end
  end
})
DataW.effect_create({
  name = "Expedited",
  stat_func = function(self)
    if new.speed < 6 then
      new.speed = math.max(new.speed * 1.5, 6)
    end
    new.ranged_cooldown_multiplier = new.ranged_cooldown_multiplier / 1.5
  end,
  can_use_rest = false,
  finish_func = function(self, obj)
    for _ in screens() do
      if obj:is_local_player() then
        EventLog.add("You are no longer expedited.", {
          255,
          200,
          200
        })
      end
    end
  end,
  effected_colour = {
    220,
    220,
    255
  },
  effected_sprite = "haste effected"
})
DataW.effect_create({
  name = "Charge",
  can_use_spells = false,
  stat_func = function(self, obj, old, new)
    new.defence = new.defence + 5
    new.strength = new.strength + 5
    new.speed = 0
  end,
  remove_func = function(self, obj)
    return self:callback(obj)
  end,
  effected_colour = {
    255,
    0,
    0
  },
  effected_sprite = "charge_effected",
  fade_out = 0
})
DataW.effect_create({
  name = "Haste",
  effected_colour = {
    160,
    160,
    255,
    240
  },
  stat_func = function(self, obj, old, new)
    new.speed = new.speed + 2
    new.strength = new.strength + 2
    new.magic = new.magic + 2
    new.defence = new.defence + 4
    new.willpower = new.willpower + 4
    new.cooldown_mult = new.cooldown_mult / 1.25
  end
})
DataW.effect_create({
  name = "Stun",
  effected_sprite = "spr_effects.pain_mirror",
  effected_colour = {
    200,
    200,
    200
  },
  duration = 15,
  stat_func = function(self, obj, old, new)
    new.speed = 0
    obj.stats.attack_cooldown = 2
  end,
  fade_out = 5
})
DataW.effect_create({
  name = "Thrown",
  effected_sprite = "spr_effects.pain_mirror",
  effected_colour = {
    200,
    200,
    200
  },
  duration = 15,
  apply_func = function(self, obj)
    self.steps = 0
  end,
  step_func = function(self, obj)
    local increment = self.duration - self.steps
    if self.steps <= self.duration then
      local x = math.floor(obj.x + math.cos(self.angle) * increment)
      local y = math.floor(obj.y + math.sin(self.angle) * increment)
      if not Map.object_solid_check(obj, {
        x,
        y
      }) then
        obj.x, obj.y = x, y
      end
    end
    self.steps = self.steps + 1
  end,
  stat_func = function(self, obj, old, new)
    new.speed = 0
    obj.stats.attack_cooldown = 2
  end,
  fade_out = 5
})
DataW.effect_create({
  name = "Pained",
  effected_sprite = "spr_spells.pain",
  effected_colour = {
    100,
    100,
    100
  },
  fade_out = 15
})
DataW.effect_create({
  name = "Sapped",
  effected_sprite = "spr_effects.manatake",
  fade_out = 15
})
DataW.effect_create({
  name = "Dazed",
  effected_sprite = "spr_effects.may_stab_brand",
  fade_out = 5
})
DataW.effect_create({
  name = "AlwaysAttack"
})
DataW.effect_create({
  name = "Slowed",
  effected_sprite = "spr_effects.slowed",
  fade_out = 5,
  stat_func = function(self, obj, old, new)
    new.melee_cooldown_multiplier = new.melee_cooldown_multiplier * 2
    new.ranged_cooldown_multiplier = new.ranged_cooldown_multiplier * 2
    new.spell_cooldown_multiplier = new.spell_cooldown_multiplier * 2
    new.speed = new.speed / 2
  end
})
DataW.effect_create({
  name = "Regeneration",
  effected_sprite = "spr_amulets.i-regeneration",
  effected_colour = {
    200,
    255,
    200
  },
  stat_func = function(self, obj, old, new)
    if obj:has_effect("AmuletGreatPain") then
      new.hpregen = new.hpregen * 30
    else
      new.hpregen = new.hpregen * 15
    end
    if obj.stats.hp == new.max_hp then
      return obj:remove_effect("Regeneration")
    end
  end,
  fade_out = 55
})
DataW.effect_create({
  name = "Fear",
  effected_sprite = "spr_effects.fleeing",
  effected_colour = {
    200,
    255,
    200
  },
  stat_func = function(self, obj, old, new)
    new.speed = new.speed * 1.25
  end,
  fade_out = 55
})
local M = nilprotect({ })
M._poison = tosprite("spr_weapons.i-venom")
M._fire_resist = tosprite("spr_effects.i-r-fire-big")
M._ice_resist = tosprite("spr_effects.i-r-cold-big")
M._generic_resist = tosprite("spr_effects.i-protection-big")
M._poison_resist = tosprite("spr_effects.i-r-poison-big")
M._black_resist = tosprite("spr_effects.black-resist")
M._storm_resist = tosprite("spr_effects.shock-resist")
M._fire_power = tosprite("spr_effects.i-fire-big")
M._ice_power = tosprite("spr_effects.i-cold-big")
M._black_power = tosprite("spr_effects.black-power")
M._storm_power = tosprite("spr_effects.shock")
M._generic_power = tosprite("spr_effects.sword")
M._poison_power = tosprite("spr_effects.i-poison-big")
M._confusion = tosprite("spr_weapons.i-confusion")
M._vampirism = tosprite("spr_weapons.i-vampirism")
M._fleeing = tosprite("spr_effects.fleeing")
M._fear = tosprite("spr_effects.i-life-protection")
local draw_console_effect
draw_console_effect = function(sprite, text, xy, color)
  if color == nil then
    color = COL_PALE_YELLOW
  end
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
local draw_weapon_console_effect
draw_weapon_console_effect = function(player, sprite, text, xy, color)
  if color == nil then
    color = COL_PALE_YELLOW
  end
  player.weapon_sprite:draw({
    origin = Display.LEFT_CENTER
  }, {
    xy[1],
    xy[2] + 4
  })
  return draw_console_effect(sprite, text, xy, color)
end
DataW.effect_create({
  name = "FearWeapon",
  console_draw_func = function(self, player, get_next)
    return draw_weapon_console_effect(player, M._fear, "+10% chance fear", get_next())
  end,
  on_melee_func = function(self, attacker, defender, damage)
    if defender:has_effect("Fear") then
      return damage
    end
    if chance(.1) then
      local eff = defender:add_effect("Fear", 150)
    end
    return damage
  end
})
DataW.effect_create({
  name = "ConfusingWeapon",
  console_draw_func = function(self, player, get_next)
    return draw_weapon_console_effect(player, M._confusion, "+10% chance daze", get_next())
  end,
  on_melee_func = function(self, attacker, defender, damage)
    if defender:has_effect("Dazed") then
      return damage
    end
    if chance(.1 * self.n_derived) then
      local eff = defender:add_effect("Dazed", 70)
    end
    return damage
  end
})
DataW.effect_create({
  name = "StopOnceInRange"
})
DataW.effect_create({
  name = "Encumbered",
  stat_func = function(self, obj, old, new)
    new.speed = new.speed - 1
  end
})
DataW.effect_create({
  name = "Baleful Regeneration",
  effected_sprite = "spr_amulets.i-regeneration",
  effected_colour = {
    200,
    255,
    200
  },
  step_func = function(self, caster)
    local hp, max_hp, hpregen
    do
      local _obj_0 = caster:effective_stats()
      hp, max_hp, hpregen = _obj_0.hp, _obj_0.max_hp, _obj_0.hpregen
    end
    if hp >= max_hp then
      return caster:remove_effect("Baleful Regeneration")
    else
      local regen_rate = hpregen * ((function()
        if caster:has_effect("AmuletGreatPain") then
          return 21
        else
          return 10
        end
      end)())
      return caster:heal_hp(regen_rate)
    end
  end
})
local _list_0 = {
  {
    M._poison_resist,
    M._poison_power,
    "Green",
    {
      226,
      66,
      244
    }
  },
  {
    M._fire_resist,
    M._fire_power,
    "Red",
    {
      244,
      72,
      66
    }
  },
  {
    M._ice_resist,
    M._ice_power,
    "Blue",
    COL_WHITE
  },
  {
    M._storm_resist,
    M._storm_power,
    "White",
    COL_BLUE
  },
  {
    M._black_resist,
    M._black_power,
    "Black",
    COL_BLUE
  },
  {
    M._generic_resist,
    M._generic_power,
    "Slashing",
    {
      66,
      222,
      188
    }
  },
  {
    M._generic_resist,
    M._generic_power,
    "Bludgeon",
    {
      66,
      244,
      194
    }
  },
  {
    M._generic_resist,
    M._generic_power,
    "Piercing",
    {
      66,
      244,
      214
    }
  }
}
for _index_0 = 1, #_list_0 do
  local _des_0 = _list_0[_index_0]
  local resist_sprite, power_sprite, type, color
  resist_sprite, power_sprite, type, color = _des_0[1], _des_0[2], _des_0[3], _des_0[4]
  color = COL_WHITE
  DataW.additive_effect_create({
    name = tostring(type) .. "Resist",
    key = "resist",
    console_draw_func = function(self, player, get_next)
      local text = tostring(type) .. " Resist"
      local res
      if self.resist < 0 then
        res = self.resist
      else
        res = "+" .. self.resist
      end
      return draw_console_effect(resist_sprite, tostring(res) .. " " .. tostring(text), get_next(), (function()
        if self.resist >= 1 then
          return color
        else
          return COL_PALE_RED
        end
      end)())
    end
  })
  DataW.additive_effect_create({
    name = tostring(type) .. "Power",
    key = "power",
    console_draw_func = function(self, player, get_next)
      local text = tostring(type) .. " Power"
      local res
      if self.power < 0 then
        res = self.power
      else
        res = "+" .. self.power
      end
      return draw_console_effect(power_sprite, tostring(res) .. " " .. tostring(text), get_next(), (function()
        if self.power >= 0 then
          return color
        else
          return COL_PALE_RED
        end
      end)())
    end
  })
end
DataW.additive_effect_create({
  name = "PoisonedWeapon",
  key = "poison_percentage",
  console_draw_func = function(self, player, get_next)
    return draw_weapon_console_effect(player, M._poison, "+" .. tostring(math.floor(self.poison_percentage * 100)) .. "% chance poison", get_next())
  end,
  on_melee_func = function(self, attacker, defender, damage)
    if defender:has_effect("Poison") then
      return damage
    end
    local power = TypeEffectUtils.get_power(attacker, "Green")
    local resist = TypeEffectUtils.get_resistance(defender, "Green")
    local poison_chance = self:value()
    if chance(poison_chance * resist) then
      local eff = defender:add_effect("Poison", {
        time_left = 100,
        poison_rate = 25,
        attacker = attacker,
        damage = damage * (1 + power / 5),
        power = attacker:effective_stats().magic + power,
        type_resistance = resist,
        magic_percentage = 1.0
      })
    end
    return damage
  end
})
DataW.additive_effect_create({
  name = "Spiky",
  key = "recoil_percentage",
  console_draw_func = function(self, player, get_next)
    return draw_console_effect(tosprite("spr_spells.spectral_weapon"), "+" .. tostring(math.floor(self.recoil_percentage * 100)) .. "% melee recoil damage", get_next())
  end,
  on_receive_melee_func = function(self, attacker, defender, damage)
    local percentage_recoil = self:value()
    attacker:direct_damage(damage * percentage_recoil, defender)
    for _ in screens() do
      if defender.is_local_player and defender:is_local_player() then
        EventLog.add("You strike back with spikes!", COL_PALE_BLUE)
      end
    end
  end
})
DataW.effect_create({
  name = "AmuletBerserker"
})
DataW.effect_create({
  name = "AmuletGreatPain"
})
DataW.effect_create({
  name = "AmuletGreaterFire"
})
DataW.effect_create({
  name = "DiesOutsideOfSummonerRange",
  init_func = function(self, summon)
    self.summoner = false
  end,
  step_func = function(self, summon)
    assert(self.summoner, "No summoner set on DiesOutsideOfSummonerRange!")
    if not Map.object_visible(summon, summon.xy, self.summoner) then
      return summon:direct_damage(10)
    end
  end
})
DataW.effect_create({
  name = "DiesOnEndOrFirstAttack",
  init_func = function(self)
    self.n_animations = rand_range(0, 16)
  end,
  die = function(self, obj)
    BonusesUtils.create_animation(self, obj, "spr_bonuses.ghost", 0.5)
    return obj:die()
  end,
  remove_func = function(self, obj)
    return self:die(obj)
  end,
  on_melee_func = function(self, obj, defender, damage)
    self:die(obj)
    return damage
  end,
  on_projectile_func = function(self, obj, defender, xy)
    return self:die(obj)
  end
})
DataW.effect_create({
  name = "Summoner",
  init_func = function(self, caster)
    self.n_summons = 0
  end,
  step_func = function(self, caster)
    caster.summoned = caster.summoned or OrderedDict()
    self.n_summons = 0
    for mon, time in strictpairs(caster.summoned) do
      if not mon.destroyed then
        event_log("Desummoning monster (%s, id=%d) at (%.2f, %.2f)", mon.name, math.max(0, mon.id), mon.x, mon.y)
        local time_out = ((function()
          if mon.name == "Spectral Beast" then
            return (time > 600)
          else
            return (time > 1200)
          end
        end)())
        local diff_floor = (caster.map ~= mon.map)
        if time_out or diff_floor then
          mon:direct_damage(mon.stats.hp + 1)
        end
      end
      if mon.destroyed then
        caster.summoned[mon] = nil
      else
        caster.summoned[mon] = caster.summoned[mon] + 1
        self.n_summons = self.n_summons + 1
      end
    end
  end
})
DataW.effect_create({
  name = "Fortification",
  console_draw_func = function(self, player, get_next)
    return draw_console_effect(tosprite("spr_spells.statue_form"), "Gain defence when hit", get_next())
  end,
  init_func = function(self, caster)
    self.active_bonuses = OrderedDict()
    self.duration = 320
  end,
  stat_func = function(self, obj, old, new)
    for k, v in strictpairs(self.active_bonuses) do
      if v == 0 then
        for _ in screens() do
          EventLog.add("Your defence falls back down...", COL_PALE_BLUE)
        end
        self.active_bonuses[k] = nil
      else
        new.defence = new.defence + 2
        self.active_bonuses[k] = self.active_bonuses[k] - 1
      end
    end
  end,
  on_receive_damage_func = function(self, attacker, defender, damage)
    if not self.active_bonuses[attacker.id] then
      for _ in screens() do
        EventLog.add("Your defence rises due to getting hit!", COL_PALE_BLUE)
      end
      self.active_bonuses[attacker.id] = self.duration
    elseif self.active_bonuses[attacker.id] < self.duration then
      self.active_bonuses[attacker.id] = self.duration
    end
    return damage
  end
})
DataW.effect_create({
  name = "Ludaze",
  effected_sprite = "spr_spells.ludaze",
  can_use_rest = false,
  effected_colour = {
    200,
    200,
    255,
    200
  },
  fade_out = 10
})
DataW.effect_create({
  name = "Ice Form",
  effected_sprite = "spr_spells.iceform",
  can_use_rest = false,
  can_use_spells = false,
  can_use_stairs = true,
  can_use_weapons = false,
  effected_colour = {
    200,
    200,
    255
  },
  stat_func = function(effect, obj, old, new)
    new.resistance = new.resistance + 100
    new.magic_resistance = new.magic_resistance + 100
    new.speed = math.min(new.speed, 0.5)
  end,
  fade_out = 55
})
local _list_1 = {
  "",
  "Armour",
  "Amulet",
  "Ring",
  "Belt",
  "Weapon",
  "Legwear"
}
for _index_0 = 1, #_list_1 do
  local equip_slot = _list_1[_index_0]
  DataW.effect_create({
    name = tostring(equip_slot) .. "HealthGainOnKill",
    category = "EquipEffect",
    init_func = function(self, caster)
      self.kill_tracker = caster.kills
    end,
    step_func = function(self, caster)
      while caster.kills > self.kill_tracker do
        for _ in screens() do
          if caster:is_local_player() then
            EventLog.add("You regain health for killing!", COL_PALE_BLUE)
          end
        end
        caster:heal_hp(5 + caster.stats.level)
        play_sound("sound/swish-10.wav")
        BonusesUtils.create_animation({
          n_animations = 0
        }, caster, "spr_effects.health", 0.5)
        self.kill_tracker = self.kill_tracker + 1
      end
    end
  })
  DataW.effect_create({
    name = tostring(equip_slot) .. "ManaGainOnKill",
    category = "EquipEffect",
    init_func = function(self, caster)
      self.kill_tracker = caster.kills
    end,
    step_func = function(self, caster)
      while caster.kills > self.kill_tracker do
        for _ in screens() do
          if caster:is_local_player() then
            EventLog.add("You regain mana for killing!", COL_PALE_BLUE)
          end
        end
        caster:heal_mp(5 + caster.stats.level)
        play_sound("sound/swish-10.wav")
        BonusesUtils.create_animation({
          n_animations = 0
        }, caster, "spr_effects.mana", 0.5)
        self.kill_tracker = self.kill_tracker + 1
      end
    end
  })
end
DataW.effect_create({
  name = "PossiblySummonCentaurOnKill",
  category = "EquipEffect",
  init_func = function(self, caster)
    self.kill_tracker = caster.kills
  end,
  step_func = function(self, caster)
    if caster:has_effect("Summoning") then
      return
    end
    while caster.kills > self.kill_tracker do
      if chance(0.03 * self.n_derived) then
        for _ in screens() do
          EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
        end
        play_sound("sound/summon.ogg")
        local eff = caster:add_effect("Summoning", 5)
        eff.monster = "Centaur Hunter"
        eff.duration = 5
      end
      self.kill_tracker = self.kill_tracker + 1
    end
  end
})
DataW.effect_create({
  name = "SummonMummyOnKill",
  category = "EquipEffect",
  init_func = function(self, caster)
    self.kill_tracker = caster.kills
  end,
  step_func = function(self, caster)
    if caster:has_effect("Summoning") then
      return
    end
    while caster.kills > self.kill_tracker do
      for _ in screens() do
        EventLog.add("A mummy is summoned due to your graceful killing!!", COL_PALE_BLUE)
      end
      play_sound("sound/summon.ogg")
      local eff = caster:add_effect("Summoning", 5)
      eff.monster = "Mummy"
      eff.duration = 5
      self.kill_tracker = self.kill_tracker + 1
    end
  end
})
DataW.effect_create({
  name = "PossiblySummonStormElementalOnKill",
  category = "EquipEffect",
  init_func = function(self, caster)
    self.kill_tracker = caster.kills
  end,
  step_func = function(self, caster)
    if caster:has_effect("Summoning") then
      return
    end
    while caster.kills > self.kill_tracker do
      if chance(0.05 * self.n_derived) then
        for _ in screens() do
          EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
        end
        play_sound("sound/summon.ogg")
        local monster = "Storm Elemental"
        local eff = caster:add_effect("Summoning", 5)
        eff.monster = ((function()
          if type(monster) == "string" then
            return monster
          else
            return random_choice(monster)
          end
        end)())
        eff.duration = 5
      end
      self.kill_tracker = self.kill_tracker + 1
    end
  end
})
DataW.effect_create({
  name = "PossiblySummonGolemOnKill",
  category = "EquipEffect",
  init_func = function(self, caster)
    self.kill_tracker = caster.kills
  end,
  step_func = function(self, caster)
    while caster.kills > self.kill_tracker do
      if chance(0.03 * self.n_derived) then
        for _ in screens() do
          EventLog.add("A creature is summoned due to your graceful killing!!", COL_PALE_BLUE)
        end
        play_sound("sound/summon.ogg")
        local monster = "Golem"
        if not (caster:has_effect("Summoning")) then
          local eff = caster:add_effect("Summoning", 20)
          eff.monster = ((function()
            if type(monster) == "string" then
              return monster
            else
              return random_choice(monster)
            end
          end)())
          eff.duration = 5
        end
      end
      self.kill_tracker = self.kill_tracker + 1
    end
  end
})
DataW.effect_create({
  name = "Summoning",
  category = "Summon",
  effected_colour = COL_PALE_RED,
  fade_out = 10,
  init_func = function(self, caster)
    self.n_steps = 0
    self.delay = 1
    self.duration = self.time_left
    self.on_summon = do_nothing
    self.summon_xy = false
    self.start_map = caster.map
    caster.summoned = caster.summoned or OrderedDict()
  end,
  step_func = function(self, caster)
    self.n_steps = self.n_steps + 1
    if self.n_steps == self.delay then
      if caster.map == self.start_map then
        local ability = SpellObjects.SummonAbility.create({
          monster = self.monster,
          caster = caster,
          xy = self.summon_xy or caster.xy,
          duration = self.duration,
          on_summon = self.on_summon
        })
        return GameObject.add_to_level(ability)
      end
    end
  end
})
DataW.effect_create({
  name = "Reviving",
  remove_func = function(self, player)
    player.is_ghost = false
    player.stats.hp = player:effective_stats().max_hp
  end,
  draw_func = function(self, player, top_left_x, top_left_y)
    local xy = Display.to_screen_xy(player.xy)
    return Fonts.small:draw({
      origin = Display.CENTER
    }, xy, tostring(math.ceil(self.time_left / 60)))
  end
})
local AuraBase = {
  init = function(self)
    self.n_steps = 0
    self.total_time = self.time_left
    self.max_alpha = 0.15
    self.n_ramp = 25
  end,
  step = function(self)
    return assert(self.range ~= nil, "'range' must be assigned to Aura effects.")
  end,
  draw = function(self, inner_col, outer_col, x, y)
    self.n_steps = self.n_steps + 1
    local min = math.min(self.n_ramp, (function()
      if self.n_steps > self.total_time / 2 then
        return math.abs(self.n_steps - self.total_time)
      else
        return self.n_steps
      end
    end)())
    local alpha = (min / self.n_ramp)
    alpha = math.max(0.2, math.min(alpha, 1))
    alpha = alpha * self.max_alpha
    local xy = Display.to_screen_xy({
      x,
      y
    })
    Display.draw_circle(with_alpha(inner_col, alpha), xy, self.range)
    return Display.draw_circle_outline(with_alpha(outer_col, alpha), xy, self.range, 2)
  end
}
DataW.effect_create({
  name = "Fear Aura",
  category = "Aura",
  effected_colour = {
    200,
    200,
    255
  },
  fade_out = 100,
  effected_sprite = "spr_spells.cause_fear",
  init_func = function(self, caster)
    AuraBase.init(self, caster)
    self.max_alpha = 0.4
  end,
  step_func = function(self, caster)
    AuraBase.step(self, caster)
    local _list_2 = (Map.enemies_list(caster))
    for _index_0 = 1, #_list_2 do
      local _continue_0 = false
      repeat
        local mon = _list_2[_index_0]
        if mon:has_effect("Fear") then
          _continue_0 = true
          break
        end
        local dist = vector_distance({
          mon.x,
          mon.y
        }, {
          caster.x,
          caster.y
        })
        if dist < self.range then
          mon:add_effect("Fear", 300)
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end,
  draw_func = function(self, caster, top_left_x, top_left_y)
    return AuraBase.draw(self, COL_GRAY, COL_BLACK, caster.x, caster.y)
  end
})
DataW.effect_create({
  name = "Healing",
  category = "Aura",
  effected_sprite = "spr_amulets.healing",
  fade_out = 25,
  step_func = function(self, obj)
    local hpregen
    hpregen = obj:effective_stats().hpregen
    return obj:heal_hp(hpregen * 20)
  end
})
DataW.effect_create({
  name = "Healing Aura",
  category = "Aura",
  effected_sprite = "spr_amulets.healing",
  fade_out = 100,
  init_func = function(self, caster)
    AuraBase.init(self, caster)
    self.max_alpha = 0.4
  end,
  step_func = function(self, caster)
    AuraBase.step(self, caster)
    local _list_2 = Map.allies_list(caster)
    for _index_0 = 1, #_list_2 do
      local _continue_0 = false
      repeat
        local ally = _list_2[_index_0]
        if ally:has_effect("Healing") then
          _continue_0 = true
          break
        end
        local dist = vector_distance({
          ally.x,
          ally.y
        }, {
          caster.x,
          caster.y
        })
        if dist < self.range then
          ally:add_effect("Healing", 200)
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end,
  draw_func = function(self, caster, top_left_x, top_left_y)
    return AuraBase.draw(self, COL_GRAY, COL_WHITE, caster.x, caster.y)
  end
})
DataW.effect_create({
  name = "Daze Aura",
  category = "Aura",
  effected_sprite = "spr_amulets.light",
  fade_out = 100,
  init_func = function(self, caster)
    return AuraBase.init(self, caster)
  end,
  step_func = function(self, caster)
    AuraBase.step(self, caster)
    local _list_2 = (Map.enemies_list(caster))
    for _index_0 = 1, #_list_2 do
      local _continue_0 = false
      repeat
        local mon = _list_2[_index_0]
        if mon:has_effect("Dazed") then
          _continue_0 = true
          break
        end
        local dist = vector_distance({
          mon.x,
          mon.y
        }, {
          caster.x,
          caster.y
        })
        if dist < self.range then
          mon:add_effect("Dazed", 100)
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end,
  draw_func = function(self, caster, top_left_x, top_left_y)
    return AuraBase.draw(self, COL_PALE_YELLOW, COL_YELLOW, caster.x, caster.y)
  end
})
DataW.effect_create({
  name = "Sap Aura",
  category = "Aura",
  fade_out = 100,
  init_func = function(self, caster)
    AuraBase.init(self, caster)
    if caster.is_enemy then
      self.range = 90
    end
  end,
  step_func = function(self, caster)
    AuraBase.step(self, caster)
    if caster.is_enemy and not Map.object_visible(caster) then
      return
    end
    local _list_2 = (Map.enemies_list(caster))
    for _index_0 = 1, #_list_2 do
      local _continue_0 = false
      repeat
        local mon = _list_2[_index_0]
        if mon:has_effect("Sapped") then
          _continue_0 = true
          break
        end
        local dist = vector_distance({
          mon.x,
          mon.y
        }, {
          caster.x,
          caster.y
        })
        if dist < self.range then
          mon:add_effect("Sapped", 35)
          mon.stats.mp = math.max(0, mon.stats.mp - 10)
          for _ in screens() do
            if mon:is_local_player() then
              EventLog.add("Your MP is drained!", {
                200,
                200,
                255
              })
            elseif not mon.is_enemy then
              EventLog.add(mon.name .. "'s MP is drained!", {
                200,
                200,
                255
              })
            end
          end
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end,
  draw_func = function(self, caster, top_left_x, top_left_y)
    self.max_alpha = 0.35
    return AuraBase.draw(self, COL_PALE_BLUE, COL_BLUE, caster.x, caster.y)
  end
})
DataW.effect_create({
  name = "Pain Aura Anim Hack",
  category = "Aura",
  effected_sprite = "spr_spells.greaterpain",
  fade_out = 50,
  init_func = function(self, caster)
    AuraBase.init(self, caster)
    self.mp_gain = 10
    if caster.is_enemy then
      self.range = 90
    end
  end,
  step_func = function(self, caster)
    AuraBase.step(self, caster)
    if self.animation_only then
      return
    end
  end,
  draw_func = function(self, caster, top_left_x, top_left_y)
    self.max_alpha = 0.35
    if self.animation_only then
      self.max_alpha = self.max_alpha / 4
    elseif not caster.is_enemy and not caster:has_effect("Pained") then
      self.max_alpha = self.max_alpha / 2
    end
    return AuraBase.draw(self, COL_PALE_RED, COL_RED, caster.x, caster.y)
  end
})
DataW.effect_create({
  name = "Pain Aura",
  category = "Aura",
  effected_sprite = "spr_spells.greaterpain",
  fade_out = 50,
  init_func = function(self, caster)
    return AuraBase.init(self, caster)
  end,
  apply_func = function(self, caster, args)
    self.mp_gain = 10
    self.damage = args.damage
    self.range = args.range
  end,
  step_func = function(self, caster)
    AuraBase.step(self, caster)
    if self.animation_only then
      return
    end
    if caster.is_enemy and not Map.object_visible(caster) then
      return
    end
    local _list_2 = (Map.enemies_list(caster))
    for _index_0 = 1, #_list_2 do
      local _continue_0 = false
      repeat
        local mon = _list_2[_index_0]
        if mon:has_effect("Pained") then
          _continue_0 = true
          break
        end
        local dist = vector_distance({
          mon.x,
          mon.y
        }, {
          caster.x,
          caster.y
        })
        if dist < self.range then
          mon:add_effect("Pained", 50)
          play_pained_sound()
          caster:add_effect("Pained", 50)
          local stats = caster:effective_stats()
          local damage, power = self.damage, random(2, 5) + stats.magic
          power = power + TypeEffectUtils.get_power(caster, "Black")
          local type_multiplier = TypeEffectUtils.get_resistance(mon, "Black")
          if mon:damage(damage, power, 1, caster, type_multiplier) then
            stats = caster.stats
            local max_hp
            max_hp = mon:effective_stats().max_hp
            self.damage = self.damage + 5
            self.time_left = self.time_left + 100
            if caster:has_effect("AmuletGreatPain") then
              caster:heal_hp(max_hp * 2 / 16)
            else
              caster:heal_hp(max_hp / 16)
            end
          end
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end,
  draw_func = function(self, caster, top_left_x, top_left_y)
    self.max_alpha = 0.35
    if self.animation_only then
      self.max_alpha = self.max_alpha / 4
    elseif not caster.is_enemy and not caster:has_effect("Pained") then
      self.max_alpha = self.max_alpha / 2
    end
    return AuraBase.draw(self, COL_PALE_RED, COL_RED, caster.x, caster.y)
  end
})
DataW.effect_create({
  name = "VampiricWeapon",
  console_draw_func = function(self, player, get_next)
    return draw_weapon_console_effect(player, M._vampirism, "Heal +25% dealt", get_next())
  end,
  on_melee_func = function(self, attacker, defender, damage)
    for _ in screens() do
      if attacker:is_local_player() then
        EventLog.add("You steal the enemy's life!", {
          200,
          200,
          255
        })
      end
    end
    attacker:heal_hp(damage / 4 * self.n_derived)
    return damage
  end
})
DataW.effect_create({
  name = "Abolishment",
  stat_func = function(effect, obj, old, new)
    new.strength = new.strength + math.ceil(new.magic / 3)
    new.magic = 0
  end
})
DataW.effect_create({
  name = "EnemyHyperProjectile",
  stat_func = function(effect, obj, old, new)
    new.ranged_cooldown_multiplier = new.ranged_cooldown_multiplier * 0.1
  end
})
DataW.effect_create({
  name = "KnockbackWeapon",
  console_draw_func = function(self, player, get_next)
    return draw_weapon_console_effect(player, M._fleeing, "+10% chance of knockback", get_next())
  end,
  on_melee_func = function(self, attacker, defender, damage)
    if defender:has_effect("Thrown") then
      return damage
    end
    if chance(.1 * self.n_derived) then
      local thrown = defender:add_effect("Thrown", 20)
      thrown.angle = vector_direction({
        attacker.x,
        attacker.y
      }, {
        defender.x,
        defender.y
      })
    end
    return damage
  end
})
DataW.effect_create({
  name = "StunWeapon",
  console_draw_func = function(self, player, get_next)
    return draw_weapon_console_effect(player, M._fleeing, "+10% chance of stun", get_next())
  end,
  on_melee_func = function(self, attacker, defender, damage)
    if defender:has_effect("Stun") then
      return damage
    end
    if chance(.1 * self.n_derived) then
      local thrown = defender:add_effect("Stun", 20)
      thrown.angle = vector_direction({
        attacker.x,
        attacker.y
      }, {
        defender.x,
        defender.y
      })
    end
    return damage
  end
})
local share_damage
share_damage = function(target, damage, min_health)
  min_health = 0
  if target.stats.hp - damage < min_health then
    damage = math.max(0, target.stats.hp - min_health)
  end
  target.stats.hp = target.stats.hp - damage
end
local LAST_WARNING = GameVar.create("LAST_WARNING", -math.huge)
local _list_2 = {
  "Ranger",
  "Fighter",
  "Templar",
  "Rogue",
  "Green Mage",
  "Black Mage",
  "Necromancer",
  "Stormcaller",
  "Pyrocaster",
  "Blue Mage"
}
for _index_0 = 1, #_list_2 do
  local name = _list_2[_index_0]
  DataW.effect_create({
    name = name,
    stat_func = function(self, obj, old, new)
      if name ~= "Fighter" then
        new.melee_cooldown_multiplier = new.melee_cooldown_multiplier * 1.25
      end
      if name ~= "Ranger" or name ~= "Fighter" then
        new.ranged_cooldown_multiplier = new.ranged_cooldown_multiplier * 1.25
      end
    end,
    init_func = function(self, caster)
      self.kill_tracker = caster.kills
      self.links = { }
      self.n_animations = 0
      return LAST_WARNING:set(-math.huge)
    end,
    step_func = function(self, caster)
      if caster:has_effect("Summoner") then
        caster:add_effect("Summoner", 2)
      else
        local eff = caster:add_effect("Summoner", 2)
        eff.duration = 30
      end
      while caster.kills > self.kill_tracker do
        local _list_3 = World.players
        for _index_1 = 1, #_list_3 do
          local _des_0 = _list_3[_index_1]
          local instance, class_name
          instance, class_name = _des_0.instance, _des_0.class_name
          if class_name == "Necromancer" then
            for _ in screens() do
              if instance:is_local_player() then
                EventLog.add("You feed on the dying monster's energy!", COL_PALE_BLUE)
              end
            end
            local istats = instance.stats
            if istats.mp < istats.max_mp then
              BonusesUtils.create_animation(self, instance, "spr_bonuses.mana", 0.5)
            end
            instance:heal_mp(10)
          end
        end
        self.kill_tracker = self.kill_tracker + 1
      end
    end
  })
end
local DICE_ROLL_INTERVAL = 240
local DICE_BUFF_DURATION = 150
local DICE_ROLLS = {
  [1] = {
    kind = "fleet",
    label = "Fleet step",
    color = COL_PALE_GREEN
  },
  [2] = {
    kind = "guard",
    label = "Guard stance",
    color = COL_PALE_BLUE
  },
  [3] = {
    kind = "edge",
    label = "Killing edge",
    color = COL_PALE_RED
  },
  [4] = {
    kind = "focus",
    label = "Flow state",
    color = COL_PALE_YELLOW
  },
  [5] = {
    kind = "dash",
    label = "Dash ready",
    color = COL_WHITE
  },
  [6] = {
    kind = "jackpot",
    label = "Power ready",
    color = COL_PALE_YELLOW
  }
}
local dicebinder_roll
dicebinder_roll = function(effect, caster)
  local roll = math.random(1, 6)
  local entry = DICE_ROLLS[roll]
  effect.last_roll = roll
  effect.roll_color = entry.color
  effect.buff_kind = entry.kind
  effect.buff_time = DICE_BUFF_DURATION
  effect.next_roll = effect.roll_interval or DICE_ROLL_INTERVAL
  if roll == 5 then
    caster:set_spell_cooldown("Dash Attack", 0)
  elseif roll == 6 then
    caster:set_spell_cooldown("Power Strike", 0)
    caster:set_spell_cooldown("Dash Attack", 0)
    caster:heal_hp(6 + caster.stats.level * 2)
    caster:heal_mp(8 + caster.stats.level)
  end
  ImpactFX.world_burst({
    map = caster.map,
    xy = caster.xy,
    color = entry.color,
    label = "D6 " .. tostring(roll),
    radius = ((function()
      if roll >= 5 then
        return 78
      else
        return 56
      end
    end)()),
    duration = ((function()
      if roll >= 5 then
        return 42
      else
        return 30
      end
    end)())
  })
  if roll >= 5 then
    ImpactFX.screen_flash({
      map = caster.map,
      color = entry.color,
      alpha = ((function()
        if roll == 6 then
          return 0.24
        else
          return 0.16
        end
      end)()),
      duration = 14
    })
  end
  for _ in screens() do
    if caster:is_local_player() then
      EventLog.add("Skill die " .. tostring(roll) .. ": " .. tostring(entry.label) .. ".", entry.color)
    end
  end
end
DataW.effect_create({
  name = "Dicebinder",
  stat_func = function(self, obj, old, new)
    new.ranged_cooldown_multiplier = new.ranged_cooldown_multiplier * 1.25
    if self.buff_time and self.buff_time > 0 then
      local _exp_0 = self.buff_kind
      if "fleet" == _exp_0 then
        new.speed = new.speed + 0.8
      elseif "guard" == _exp_0 then
        new.defence = new.defence + 3
        new.willpower = new.willpower + 3
      elseif "edge" == _exp_0 then
        new.strength = new.strength + 3
      elseif "focus" == _exp_0 then
        new.spell_cooldown_multiplier = new.spell_cooldown_multiplier * 0.72
      elseif "jackpot" == _exp_0 then
        new.melee_cooldown_multiplier = new.melee_cooldown_multiplier * 0.82
        new.spell_cooldown_multiplier = new.spell_cooldown_multiplier * 0.82
      end
    end
  end,
  init_func = function(self, caster)
    self.roll_interval = DICE_ROLL_INTERVAL
    self.next_roll = 90
    self.last_roll = 0
    self.roll_color = COL_PALE_BLUE
    self.buff_kind = "none"
    self.buff_time = 0
  end,
  step_func = function(self, caster)
    self.next_roll = self.next_roll - 1
    if self.buff_time and self.buff_time > 0 then
      self.buff_time = self.buff_time - 1
    end
    if self.next_roll <= 0 then
      return dicebinder_roll(self, caster)
    end
  end,
  draw_func = function(self, caster)
    return ProceduralEffects.draw_die_badge(caster, {
      roll = self.last_roll or 0,
      remaining = self.next_roll or 0,
      interval = self.roll_interval or DICE_ROLL_INTERVAL,
      color = self.roll_color or COL_PALE_BLUE
    })
  end
})
DataW.effect_create({
  name = "Dash Attack",
  effected_sprite = "spr_effects.i-stealth",
  can_use_rest = false,
  effected_colour = {
    200,
    200,
    255,
    255
  },
  fade_out = 10,
  init_func = function(self, caster)
    if Map.object_visible(caster) then
      play_sound("sound/swish-12.wav")
    end
    self.steps = 0
    self.n_hits = 0
    self.attacked = { }
  end,
  apply_func = function(self, caster, args)
    self.dir_vector = {
      math.cos(args.angle) * 16,
      math.sin(args.angle) * 16
    }
  end,
  step_func = function(self, caster)
    self.steps = self.steps + 1
    local dxy = self.dir_vector
    if caster.configure_dir then
      dxy = caster:configure_dir(dxy)
    end
    if dxy[1] == self.dir_vector[1] and dxy[2] == self.dir_vector[2] then
      self.dir_vector = GameObject.simulate_bounce(caster, self.dir_vector)
      dxy = self.dir_vector
    end
    local xy = {
      caster.x + dxy[1],
      caster.y + dxy[2]
    }
    if Map.object_tile_check(caster, xy) then
      if Map.object_visible(caster) then
        play_sound("sound/door.ogg")
      end
      caster:remove_effect("Dash Attack")
      return
    end
    caster.xy = xy
    if caster:has_ranged_weapon() then
      return
    end
    local _list_3 = Map.enemies_list(caster)
    for _index_0 = 1, #_list_3 do
      local _continue_0 = false
      repeat
        local mon = _list_3[_index_0]
        local dist = vector_distance(mon.xy, caster.xy)
        if dist < caster.target_radius + mon.target_radius then
          if self.attacked[mon.id] then
            _continue_0 = true
            break
          end
          self.attacked[mon.id] = true
          for _ in screens() do
            if caster:is_local_player() then
              EventLog.add("You strike as you pass!", {
                200,
                200,
                255
              })
            end
          end
          self.n_hits = self.n_hits + 1
          caster:melee(mon, 1.0)
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end,
  draw_func = function(self, caster)
    for i = 1, math.min(self.steps, 12) do
      local xy = {
        caster.x - self.dir_vector[1] * i,
        caster.y - self.dir_vector[2] * i
      }
      local screen_xy = Display.to_screen_xy(xy)
      caster.sprite:draw({
        color = {
          255,
          255,
          255,
          200 - 30 * i
        },
        origin = Display.CENTER
      }, screen_xy)
    end
  end,
  stat_func = function(self, caster, old, new)
    new.speed = 0
    new.strength = new.strength + caster.stats.level
    new.defence = new.defence + caster.stats.level
    new.willpower = new.willpower + caster.stats.level
  end
})
local enemy_init
enemy_init = function(enemy)
  return nil
end
local enemy_step
enemy_step = function(enemy)
  return nil
end
local enemy_berserker_init
enemy_berserker_init = function(enemy)
  enemy.berserk_time = 500
end
local enemy_berserker_step
enemy_berserker_step = function(enemy)
  enemy.berserk_time = enemy.berserk_time - 1
  if enemy.berserk_time <= 0 and Map.object_visible(enemy) then
    enemy:add_effect("Berserk", 300)
    enemy.berserk_time = 500
  end
end
DataW.effect_create({
  name = "Enraging",
  apply_func = function(self, mon)
    self.damage_tracker = 0
    self.damage_interval = mon.stats.max_hp / 3
    self.next_hp_threshold = mon.stats.max_hp - self.damage_interval
  end,
  on_receive_damage_func = function(self, attacker, mon, dmg)
    self.damage_tracker = self.damage_tracker + dmg
    local new_hp = mon.stats.hp - dmg
    if new_hp < self.next_hp_threshold then
      for _ in screens() do
        EventLog.add(((function()
          if mon.unique then
            return ""
          else
            return "The "
          end
        end)()) .. mon.name .. " gets mad!", {
          255,
          255,
          255
        })
      end
      mon:add_effect("Charging", 100)
      self.next_hp_threshold = self.next_hp_threshold - self.damage_interval
    end
    return dmg
  end
})
DataW.effect_create({
  name = "Charging",
  stat_func = function(self, mon, old, new)
    new.speed = new.speed * (function()
      if mon.name == "Gragh" then
        return 2.5
      else
        return 4
      end
    end)()
    if self.n_steps > self.n_ramp then
      new.melee_cooldown_multiplier = new.melee_cooldown_multiplier * (function()
        if mon.name == "Gragh" then
          return 0.75
        else
          return 0.5
        end
      end)()
    end
    if mon.name ~= "Gragh" then
      new.hpregen = new.hpregen * 2
    end
  end,
  effected_sprite = "spr_effects.i-loudness",
  effected_colour = {
    255,
    0,
    0
  },
  fade_out = 15,
  apply_func = function(self, mon)
    self.n_steps = 0
    self.n_ramp = 10
  end,
  step_func = function(self, mon)
    if self.n_steps % 60 == 0 then
      if Map.object_visible(mon) then
        play_sound("sound/wavy.ogg")
      end
    end
    self.n_steps = self.n_steps + 1
  end,
  on_melee_func = function(self, mon, defender, damage)
    do
      return nil
    end
    if self.n_steps > self.n_ramp then
      local thrown = defender:add_effect("Thrown", 10)
      thrown.angle = vector_direction({
        mon.x,
        mon.y
      }, {
        defender.x,
        defender.y
      })
      for _ in screens() do
        if not mon.is_enemy and mon:is_local_player() then
          EventLog.add("The " .. defender.name .. " is thrown back!", {
            200,
            200,
            255
          })
        elseif not defender.is_enemy and defender:is_local_player() then
          EventLog.add("You are thrown back!", {
            200,
            200,
            255
          })
        end
      end
    end
    return damage
  end
})
DataW.effect_create({
  name = "DebugMonsterEvalEffect",
  init_func = function(self)
    self.defender_count = 0
    self.attacker_count = 0
    self.melee_count = 0
    self.projectile_count = 0
    self.receive_melee_count = 0
  end,
  on_defend_func = function(self)
    self.defender_count = self.defender_count + 1
  end,
  on_attack_func = function(self)
    self.attacker_count = self.attacker_count + 1
  end,
  on_melee_func = function(self)
    self.melee_count = self.melee_count + 1
  end,
  on_projectile_func = function(self)
    self.projectile_count = self.projectile_count + 1
  end,
  on_receive_melee_func = function(self)
    self.receive_melee_count = self.receive_melee_count + 1
  end
})
return M
