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
    local attacker_key
    if attacker and attacker.id then
      attacker_key = attacker.id
    else
      attacker_key = "environment"
    end
    if not self.active_bonuses[attacker_key] then
      for _ in screens() do
        EventLog.add("Your defence rises due to getting hit!", COL_PALE_BLUE)
      end
      self.active_bonuses[attacker_key] = self.duration
    elseif self.active_bonuses[attacker_key] < self.duration then
      self.active_bonuses[attacker_key] = self.duration
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
local DICE_ROLL_ANIMATION = 84
local DICE_ROLL_REVEAL_FRAMES = 18
local DICE_ROLL_FINAL_REVEAL_FRAMES = 8
local DICE_ROLL_TICK_PERIOD = 4
local DICE_MINOR_COOLDOWN_TRIM = 45
local DICE_ROLL_LOCK_STAGE_FRAMES = {
  14,
  34,
  58,
  DICE_ROLL_ANIMATION - DICE_ROLL_FINAL_REVEAL_FRAMES
}
local DICE_ROLL_LOCK_LABELS = {
  "PIPS",
  "TUMBLE",
  "BANK",
  "FACE"
}
local DICE_ROLL_SURGE_STAGE_FRAMES = {
  8,
  22,
  38,
  56,
  72
}
local DICE_ROLL_SURGE_LABELS = {
  "SHAKE",
  "RATTLE",
  "SPIN",
  "DROP",
  "LOCK"
}
local DICE_ROLL_LOCK_TOTAL = 4
local DICE_ROLL_SURGE_TOTAL = 5
local DICE_INITIAL_ROLL_DELAY = DICE_ROLL_INTERVAL - DICE_BUFF_DURATION - DICE_ROLL_ANIMATION
local DICE_FULL_BAG_MASK = 63
local DICE_TWO_PI = math.pi * 2
local DICE_FACE_BITS = {
  1,
  2,
  4,
  8,
  16,
  32
}
local DICE_ROLLS = {
  [1] = {
    kind = "fleet",
    label = "Fleet step",
    short = "SPD",
    color = COL_PALE_GREEN
  },
  [2] = {
    kind = "guard",
    label = "Guard stance",
    short = "GRD",
    color = COL_PALE_BLUE
  },
  [3] = {
    kind = "edge",
    label = "Killing edge",
    short = "DMG",
    color = COL_PALE_RED
  },
  [4] = {
    kind = "focus",
    label = "Flow state",
    short = "CDR",
    color = COL_PALE_YELLOW
  },
  [5] = {
    kind = "dash",
    label = "Dash ready",
    short = "DASH",
    color = COL_WHITE
  },
  [6] = {
    kind = "jackpot",
    label = "Power ready",
    short = "PWR",
    color = COL_PALE_YELLOW
  }
}
local dicebinder_increment
dicebinder_increment = function(effect, key, amount)
  if amount == nil then
    amount = 1
  end
  effect[key] = (effect[key] or 0) + amount
end
local dicebinder_face_available
dicebinder_face_available = function(mask, face)
  local bit = DICE_FACE_BITS[face] or 0
  return bit > 0 and math.floor(mask / bit) % 2 >= 1
end
local dicebinder_bag_count
dicebinder_bag_count = function(mask)
  local count = 0
  for face = 1, 6 do
    if dicebinder_face_available(mask, face) then
      count = count + 1
    end
  end
  return count
end
local dicebinder_remove_from_bag
dicebinder_remove_from_bag = function(mask, face)
  local bit = DICE_FACE_BITS[face] or 0
  if bit > 0 and dicebinder_face_available(mask, face) then
    return mask - bit
  end
  return mask
end
local dicebinder_mask_without_faces
dicebinder_mask_without_faces = function(mask, faces)
  local reduced = mask
  for _index_0 = 1, #faces do
    local face = faces[_index_0]
    reduced = dicebinder_remove_from_bag(reduced, face)
  end
  return reduced
end
local dicebinder_mask_only_faces
dicebinder_mask_only_faces = function(mask, faces)
  local filtered = 0
  for _index_0 = 1, #faces do
    local face = faces[_index_0]
    if dicebinder_face_available(mask, face) then
      filtered = filtered + (DICE_FACE_BITS[face] or 0)
    end
  end
  return filtered
end
local dicebinder_roll_lock_stage
dicebinder_roll_lock_stage = function(elapsed)
  local stage = 0
  for idx, frame in ipairs(DICE_ROLL_LOCK_STAGE_FRAMES) do
    if elapsed >= frame then
      stage = idx
    end
  end
  return stage
end
local dicebinder_roll_surge_stage
dicebinder_roll_surge_stage = function(elapsed)
  local stage = 0
  for idx, frame in ipairs(DICE_ROLL_SURGE_STAGE_FRAMES) do
    if elapsed >= frame then
      stage = idx
    end
  end
  return stage
end
local dicebinder_next_fair_roll
dicebinder_next_fair_roll = function(effect, avoid_face)
  if avoid_face == nil then
    avoid_face = 0
  end
  effect.pending_guard_kind = "none"
  local mask = effect.bag_mask or DICE_FULL_BAG_MASK
  local remaining = dicebinder_bag_count(mask)
  local refilled = false
  if remaining <= 0 then
    mask = DICE_FULL_BAG_MASK
    remaining = 6
    refilled = true
    dicebinder_increment(effect, "bag_refills")
  end
  local pick_mask = mask
  local pick_remaining = remaining
  if refilled and avoid_face and avoid_face > 0 and remaining > 1 and dicebinder_face_available(mask, avoid_face) then
    pick_mask = dicebinder_remove_from_bag(mask, avoid_face)
    pick_remaining = remaining - 1
    dicebinder_increment(effect, "anti_repeat_rerolls")
  end
  if (effect.current_high_streak or 0) >= 2 then
    local guarded_mask = dicebinder_mask_without_faces(pick_mask, {
      5,
      6
    })
    local guarded_remaining = dicebinder_bag_count(guarded_mask)
    if guarded_remaining > 0 then
      pick_mask = guarded_mask
      pick_remaining = guarded_remaining
      effect.pending_guard_kind = "bank"
      dicebinder_increment(effect, "high_streak_breaks")
    end
  elseif (effect.current_utility_streak or 0) >= 3 then
    local guarded_mask = dicebinder_mask_only_faces(pick_mask, {
      5,
      6
    })
    local guarded_remaining = dicebinder_bag_count(guarded_mask)
    if guarded_remaining > 0 then
      pick_mask = guarded_mask
      pick_remaining = guarded_remaining
      effect.pending_guard_kind = "surge"
      dicebinder_increment(effect, "low_streak_breaks")
    end
  end
  local pick = math.random(1, pick_remaining)
  local seen = 0
  for face = 1, 6 do
    if dicebinder_face_available(pick_mask, face) then
      seen = seen + 1
      if seen == pick then
        effect.bag_mask = dicebinder_remove_from_bag(mask, face)
        return face
      end
    end
  end
  effect.bag_mask = DICE_FULL_BAG_MASK - 1
  return 1
end
local dicebinder_near_miss_roll
dicebinder_near_miss_roll = function(roll)
  local direction
  if math.random(1, 2) == 1 then
    direction = -1
  else
    direction = 1
  end
  local near = roll + direction
  if near < 1 then
    near = 2
  elseif near > 6 then
    near = 5
  end
  return near
end
local dicebinder_visible_roll
dicebinder_visible_roll = function(effect)
  if (effect.rolling_time or 0) <= DICE_ROLL_FINAL_REVEAL_FRAMES and effect.pending_roll and effect.pending_roll > 0 then
    return effect.pending_roll
  end
  if (effect.rolling_time or 0) <= DICE_ROLL_REVEAL_FRAMES and effect.near_miss_roll and effect.near_miss_roll > 0 then
    return effect.near_miss_roll
  end
  if effect.preview_roll and effect.preview_roll > 0 then
    return effect.preview_roll
  end
  return 1
end
local dicebinder_record_refresh
dicebinder_record_refresh = function(effect, caster, spell_name, total_key, effective_key)
  dicebinder_increment(effect, total_key)
  if caster:spell_cooldown(spell_name) > 0 then
    return dicebinder_increment(effect, effective_key)
  end
end
local dicebinder_trim_cooldown
dicebinder_trim_cooldown = function(effect, caster, spell_name, frames, total_key, effective_key)
  local current = caster:spell_cooldown(spell_name) or 0
  if not (current > 0) then
    return 0
  end
  local shaved = math.min(current, frames)
  caster:set_spell_cooldown(spell_name, math.max(0, current - frames))
  dicebinder_increment(effect, total_key, shaved)
  dicebinder_increment(effect, effective_key)
  return shaved
end
local dicebinder_record_roll
dicebinder_record_roll = function(effect, caster, roll, entry, previous_roll)
  dicebinder_increment(effect, "rolls_total")
  dicebinder_increment(effect, "roll_" .. tostring(roll) .. "_count")
  if roll >= 5 then
    dicebinder_increment(effect, "high_rolls")
    effect.current_high_streak = (effect.current_high_streak or 0) + 1
    effect.longest_high_streak = math.max(effect.longest_high_streak or 0, effect.current_high_streak)
    effect.current_utility_streak = 0
  else
    dicebinder_increment(effect, "utility_rolls")
    effect.current_high_streak = 0
    effect.current_utility_streak = (effect.current_utility_streak or 0) + 1
    effect.longest_utility_streak = math.max(effect.longest_utility_streak or 0, effect.current_utility_streak)
  end
  if previous_roll and previous_roll > 0 then
    if previous_roll == roll then
      dicebinder_increment(effect, "repeat_rolls")
    else
      dicebinder_increment(effect, "roll_variety_switches")
    end
  end
  if roll == 5 then
    return dicebinder_record_refresh(effect, caster, "Dash Attack", "dash_refreshes", "dash_effective_refreshes")
  elseif roll == 6 then
    dicebinder_increment(effect, "jackpot_rolls")
    dicebinder_record_refresh(effect, caster, "Dash Attack", "dash_refreshes", "dash_effective_refreshes")
    return dicebinder_record_refresh(effect, caster, "Power Strike", "power_refreshes", "power_effective_refreshes")
  end
end
local dicebinder_emit_roll_fx
dicebinder_emit_roll_fx = function(caster, roll, entry)
  local major = roll >= 5
  ImpactFX.world_burst({
    map = caster.map,
    xy = caster.xy,
    color = entry.color,
    label = "D6 " .. tostring(roll),
    radius = ((function()
      if major then
        return 82
      else
        return 58 + roll * 4
      end
    end)()),
    duration = ((function()
      if major then
        return 46
      else
        return 30 + roll * 2
      end
    end)())
  })
  local pip_count = math.max(2, math.min(6, roll))
  local spread = 26 + roll * 5
  for idx = 1, pip_count do
    local angle = (GameState.frame or 0) * 0.037 + idx * math.pi * 2 / pip_count
    ImpactFX.world_burst({
      map = caster.map,
      xy = {
        caster.x + math.cos(angle) * spread,
        caster.y + math.sin(angle) * spread
      },
      color = entry.color,
      radius = ((function()
        if major then
          return 28
        else
          return 18
        end
      end)()),
      duration = 18 + roll * 3,
      drift = 0.18
    })
  end
  if roll >= 4 then
    return ImpactFX.screen_flash({
      map = caster.map,
      color = entry.color,
      alpha = ((function()
        if roll == 6 then
          return 0.24
        elseif roll == 5 then
          return 0.16
        else
          return 0.10
        end
      end)()),
      duration = ((function()
        if roll == 6 then
          return 16
        else
          return 12
        end
      end)())
    })
  end
end
local dicebinder_emit_roll_lock_fx
dicebinder_emit_roll_lock_fx = function(effect, caster, stage, roll, entry)
  dicebinder_increment(effect, "roll_lock_cues")
  local pip_count = math.min(6, math.max(2, stage * 2))
  dicebinder_increment(effect, "roll_suspense_pips", pip_count)
  local color
  if stage >= 3 then
    color = entry.color
  else
    color = COL_WHITE
  end
  local label = DICE_ROLL_LOCK_LABELS[stage] or "LOCK"
  local radius = 26 + stage * 14
  ImpactFX.world_burst({
    map = caster.map,
    xy = caster.xy,
    color = color,
    label = label,
    radius = 42 + stage * 14,
    duration = 18 + stage * 4,
    drift = 0.18
  })
  local frame = GameState.frame or 0
  for idx = 1, pip_count do
    local angle = frame * 0.061 + idx * DICE_TWO_PI / pip_count + stage * 0.42
    ImpactFX.world_burst({
      map = caster.map,
      xy = {
        caster.x + math.cos(angle) * radius,
        caster.y + math.sin(angle) * radius
      },
      color = color,
      radius = 12 + stage * 3,
      duration = 14 + stage * 4,
      drift = 0.08
    })
  end
  if stage >= 3 then
    return ImpactFX.screen_flash({
      map = caster.map,
      color = entry.color,
      alpha = (function()
        if roll >= 5 then
          return 0.10
        else
          return 0.055
        end
      end)(),
      duration = 7
    })
  end
end
local dicebinder_emit_roll_surge_fx
dicebinder_emit_roll_surge_fx = function(effect, caster, stage, roll, entry)
  dicebinder_increment(effect, "roll_surge_cues")
  local color
  if stage >= 4 then
    color = entry.color
  else
    color = COL_WHITE
  end
  local label = DICE_ROLL_SURGE_LABELS[stage] or "ROLL"
  local radius = 30 + stage * 12
  ImpactFX.world_burst({
    map = caster.map,
    xy = caster.xy,
    color = color,
    label = label,
    radius = radius,
    duration = 14 + stage * 4,
    drift = 0.10
  })
  local frame = GameState.frame or 0
  local pip_count = math.min(6, 2 + stage)
  for idx = 1, pip_count do
    local angle = frame * (0.09 + stage * 0.010) + idx * DICE_TWO_PI / pip_count
    ImpactFX.world_burst({
      map = caster.map,
      xy = {
        caster.x + math.cos(angle) * (radius * 0.62),
        caster.y + math.sin(angle) * (radius * 0.62)
      },
      color = color,
      radius = 9 + stage * 2,
      duration = 11 + stage * 3,
      drift = 0.06
    })
  end
  if stage >= 5 then
    return ImpactFX.screen_flash({
      map = caster.map,
      color = entry.color,
      alpha = (function()
        if roll >= 5 then
          return 0.12
        else
          return 0.06
        end
      end)(),
      duration = 8
    })
  end
end
local dicebinder_emit_roll_guard_fx
dicebinder_emit_roll_guard_fx = function(effect, caster)
  local guard = effect.roll_guard_kind or "none"
  if guard == "none" then
    return
  end
  dicebinder_increment(effect, "roll_guard_cues")
  local label
  if guard == "surge" then
    label = "SURGE"
  else
    label = "BANK"
  end
  local color
  if guard == "surge" then
    color = COL_PALE_YELLOW
  else
    color = COL_PALE_BLUE
  end
  ImpactFX.world_burst({
    map = caster.map,
    xy = caster.xy,
    color = color,
    label = label,
    radius = (function()
      if guard == "surge" then
        return 84
      else
        return 66
      end
    end)(),
    duration = 34,
    drift = 0.12
  })
  return ImpactFX.screen_flash({
    map = caster.map,
    color = color,
    alpha = (function()
      if guard == "surge" then
        return 0.08
      else
        return 0.045
      end
    end)(),
    duration = 8
  })
end
local dicebinder_update_roll_lock
dicebinder_update_roll_lock = function(effect, caster, elapsed)
  local stage = dicebinder_roll_lock_stage(elapsed)
  local previous = effect.roll_lock_stage or 0
  if not (stage > previous) then
    return
  end
  local roll = dicebinder_visible_roll(effect)
  local entry = DICE_ROLLS[roll] or DICE_ROLLS[1]
  for lock_stage = previous + 1, stage do
    dicebinder_emit_roll_lock_fx(effect, caster, lock_stage, roll, entry)
  end
  effect.roll_lock_stage = stage
end
local dicebinder_update_roll_surge
dicebinder_update_roll_surge = function(effect, caster, elapsed)
  local stage = dicebinder_roll_surge_stage(elapsed)
  local previous = effect.roll_surge_stage or 0
  if not (stage > previous) then
    return
  end
  local roll = dicebinder_visible_roll(effect)
  local entry = DICE_ROLLS[roll] or DICE_ROLLS[1]
  for surge_stage = previous + 1, stage do
    dicebinder_emit_roll_surge_fx(effect, caster, surge_stage, roll, entry)
  end
  effect.roll_surge_stage = stage
end
local dicebinder_emit_roll_tick_fx
dicebinder_emit_roll_tick_fx = function(effect, caster, elapsed)
  if not (elapsed > 0 and elapsed % DICE_ROLL_TICK_PERIOD == 0) then
    return
  end
  local total = math.max(1, effect.rolling_total or DICE_ROLL_ANIMATION)
  local pct = math.max(0, math.min(1, elapsed / total))
  local roll = dicebinder_visible_roll(effect)
  local entry = DICE_ROLLS[roll] or DICE_ROLLS[1]
  local pip_bonus
  if pct > 0.65 then
    pip_bonus = 1
  else
    pip_bonus = 0
  end
  local pip_count = math.max(3, math.min(6, roll + pip_bonus))
  local radius = 24 + pct * 44
  local burst_radius = 10 + pct * 12
  local frame = GameState.frame or 0
  dicebinder_increment(effect, "roll_buildup_fx")
  for idx = 1, pip_count do
    local angle = frame * (0.10 + pct * 0.05) + idx * DICE_TWO_PI / pip_count + pct * 1.4
    ImpactFX.world_burst({
      map = caster.map,
      xy = {
        caster.x + math.cos(angle) * radius,
        caster.y + math.sin(angle) * radius
      },
      color = entry.color,
      radius = burst_radius,
      duration = 10 + math.floor(pct * 9),
      drift = 0.10
    })
    if pct > 0.52 and idx % 2 == 0 then
      local counter_angle = -angle + pct * 2.1
      ImpactFX.world_burst({
        map = caster.map,
        xy = {
          caster.x + math.cos(counter_angle) * (radius * 0.58),
          caster.y + math.sin(counter_angle) * (radius * 0.58)
        },
        color = entry.color,
        radius = burst_radius * 0.72,
        duration = 8 + math.floor(pct * 8),
        drift = 0.06
      })
    end
  end
  if (effect.rolling_time or 0) <= DICE_ROLL_FINAL_REVEAL_FRAMES and not effect.roll_reveal_fx_emitted then
    effect.roll_reveal_fx_emitted = true
    dicebinder_increment(effect, "roll_reveal_cues")
    return ImpactFX.screen_flash({
      map = caster.map,
      color = entry.color,
      alpha = (function()
        if roll == 6 then
          return 0.14
        elseif roll == 5 then
          return 0.10
        else
          return 0.05
        end
      end)(),
      duration = 8
    })
  end
end
local dicebinder_resolve_roll
dicebinder_resolve_roll = function(effect, caster)
  local previous_roll = effect.last_roll or 0
  local roll
  if effect.pending_roll and effect.pending_roll > 0 then
    roll = effect.pending_roll
  else
    roll = dicebinder_next_fair_roll(effect, previous_roll)
  end
  local entry = DICE_ROLLS[roll]
  effect.last_roll = roll
  effect.roll_color = entry.color
  effect.buff_kind = entry.kind
  effect.buff_time = DICE_BUFF_DURATION
  effect.pending_roll = 0
  effect.near_miss_roll = 0
  effect.roll_guard_kind = "none"
  effect.preview_roll = roll
  effect.rolling_time = 0
  effect.next_roll = math.max(1, (effect.roll_interval or DICE_ROLL_INTERVAL) - DICE_ROLL_ANIMATION)
  dicebinder_record_roll(effect, caster, roll, entry, previous_roll)
  if roll == 4 then
    dicebinder_trim_cooldown(effect, caster, "Dash Attack", DICE_MINOR_COOLDOWN_TRIM, "cooldown_trim_frames", "dash_trim_effective")
    dicebinder_trim_cooldown(effect, caster, "Power Strike", math.floor(DICE_MINOR_COOLDOWN_TRIM * 0.75), "cooldown_trim_frames", "power_trim_effective")
  elseif roll == 5 then
    caster:set_spell_cooldown("Dash Attack", 0)
  elseif roll == 6 then
    caster:set_spell_cooldown("Power Strike", 0)
    caster:set_spell_cooldown("Dash Attack", 0)
    caster:heal_hp(4 + caster.stats.level)
    caster:heal_mp(8 + caster.stats.level)
  end
  dicebinder_increment(effect, "roll_finale_fx")
  dicebinder_emit_roll_fx(caster, roll, entry)
  for _ in screens() do
    if caster:is_local_player() then
      EventLog.add("Skill die " .. tostring(roll) .. ": " .. tostring(entry.label) .. ".", entry.color)
    end
  end
end
local dicebinder_start_roll
dicebinder_start_roll = function(effect, caster)
  effect.pending_roll = dicebinder_next_fair_roll(effect, effect.last_roll or 0)
  effect.roll_guard_kind = effect.pending_guard_kind or "none"
  effect.near_miss_roll = dicebinder_near_miss_roll(effect.pending_roll)
  effect.preview_roll = math.random(1, 6)
  effect.rolling_time = DICE_ROLL_ANIMATION
  effect.rolling_total = DICE_ROLL_ANIMATION
  effect.roll_reveal_fx_emitted = false
  effect.roll_lock_stage = 0
  effect.roll_surge_stage = 0
  effect.roll_spin_seed = math.random(1, 360)
  effect.roll_color = COL_WHITE
  dicebinder_increment(effect, "roll_animations_total")
  ImpactFX.world_burst({
    map = caster.map,
    xy = caster.xy,
    color = COL_WHITE,
    label = "D6",
    radius = 72,
    duration = DICE_ROLL_ANIMATION
  })
  return dicebinder_emit_roll_guard_fx(effect, caster)
end
local dicebinder_step_roll_animation
dicebinder_step_roll_animation = function(effect, caster)
  dicebinder_increment(effect, "roll_animation_frames")
  local previous_shown = dicebinder_visible_roll(effect)
  effect.rolling_time = math.max(0, (effect.rolling_time or 0) - 1)
  local elapsed = (effect.rolling_total or DICE_ROLL_ANIMATION) - effect.rolling_time
  local period
  if elapsed < 24 then
    period = 2
  elseif elapsed < 48 then
    period = 3
  else
    period = 5
  end
  if effect.rolling_time <= DICE_ROLL_FINAL_REVEAL_FRAMES then
    if effect.pending_roll and effect.pending_roll > 0 then
      effect.preview_roll = effect.pending_roll
    else
      effect.preview_roll = effect.preview_roll or 1
    end
  elseif effect.rolling_time <= DICE_ROLL_REVEAL_FRAMES then
    if effect.near_miss_roll and effect.near_miss_roll > 0 then
      effect.preview_roll = effect.near_miss_roll
    else
      effect.preview_roll = effect.preview_roll or 1
    end
  elseif elapsed % period == 0 then
    local spin_seed = effect.roll_spin_seed or 1
    local jump = 1 + ((spin_seed + elapsed) % 5)
    effect.preview_roll = ((effect.preview_roll or 0) + jump - 1) % 6 + 1
  end
  if dicebinder_visible_roll(effect) ~= previous_shown then
    dicebinder_increment(effect, "roll_face_swaps")
  end
  dicebinder_update_roll_surge(effect, caster, elapsed)
  dicebinder_update_roll_lock(effect, caster, elapsed)
  dicebinder_emit_roll_tick_fx(effect, caster, elapsed)
  if effect.rolling_time <= 0 then
    return dicebinder_resolve_roll(effect, caster)
  end
end
DataW.effect_create({
  name = "Dicebinder",
  stat_func = function(self, obj, old, new)
    new.ranged_cooldown_multiplier = new.ranged_cooldown_multiplier * 1.25
    if self.buff_time and self.buff_time > 0 then
      local _exp_0 = self.buff_kind
      if "fleet" == _exp_0 then
        new.speed = new.speed + 0.7
      elseif "guard" == _exp_0 then
        new.defence = new.defence + 2
        new.willpower = new.willpower + 2
      elseif "edge" == _exp_0 then
        new.strength = new.strength + 2
        new.melee_cooldown_multiplier = new.melee_cooldown_multiplier * 0.97
      elseif "focus" == _exp_0 then
        new.spell_cooldown_multiplier = new.spell_cooldown_multiplier * 0.84
      elseif "dash" == _exp_0 then
        new.speed = new.speed + 1.0
        new.melee_cooldown_multiplier = new.melee_cooldown_multiplier * 0.96
      elseif "jackpot" == _exp_0 then
        new.speed = new.speed + 0.45
        new.melee_cooldown_multiplier = new.melee_cooldown_multiplier * 0.90
        new.spell_cooldown_multiplier = new.spell_cooldown_multiplier * 0.90
      end
    end
  end,
  init_func = function(self, caster)
    self.roll_interval = DICE_ROLL_INTERVAL
    self.next_roll = DICE_INITIAL_ROLL_DELAY
    self.last_roll = 0
    self.roll_color = COL_PALE_BLUE
    self.buff_kind = "none"
    self.buff_time = 0
    self.pending_roll = 0
    self.pending_guard_kind = "none"
    self.roll_guard_kind = "none"
    self.near_miss_roll = 0
    self.preview_roll = 0
    self.rolling_time = 0
    self.rolling_total = DICE_ROLL_ANIMATION
    self.roll_reveal_fx_emitted = false
    self.roll_lock_stage = 0
    self.roll_surge_stage = 0
    self.roll_spin_seed = 1
    self.bag_mask = DICE_FULL_BAG_MASK
    self.bag_refills = 0
    self.anti_repeat_rerolls = 0
    self.roll_animations_total = 0
    self.roll_animation_frames = 0
    self.roll_buildup_fx = 0
    self.roll_face_swaps = 0
    self.roll_reveal_cues = 0
    self.roll_lock_cues = 0
    self.roll_surge_cues = 0
    self.roll_finale_fx = 0
    self.roll_suspense_pips = 0
    self.roll_guard_cues = 0
    self.cooldown_trim_frames = 0
    self.dash_trim_effective = 0
    self.power_trim_effective = 0
    self.rolls_total = 0
    self.high_rolls = 0
    self.utility_rolls = 0
    self.current_high_streak = 0
    self.longest_high_streak = 0
    self.current_utility_streak = 0
    self.longest_utility_streak = 0
    self.high_streak_breaks = 0
    self.low_streak_breaks = 0
    self.repeat_rolls = 0
    self.roll_variety_switches = 0
    self.dash_refreshes = 0
    self.dash_effective_refreshes = 0
    self.power_refreshes = 0
    self.power_effective_refreshes = 0
    self.jackpot_rolls = 0
    self.buff_active_frames = 0
    for roll = 1, 6 do
      self["roll_" .. tostring(roll) .. "_count"] = 0
    end
  end,
  step_func = function(self, caster)
    if self.buff_time and self.buff_time > 0 then
      dicebinder_increment(self, "buff_active_frames")
      if self.buff_kind and self.buff_kind ~= "none" then
        dicebinder_increment(self, tostring(self.buff_kind) .. "_frames")
      end
      self.buff_time = self.buff_time - 1
    end
    if self.rolling_time and self.rolling_time > 0 then
      dicebinder_step_roll_animation(self, caster)
      return
    end
    self.next_roll = self.next_roll - 1
    if self.next_roll <= 0 then
      return dicebinder_start_roll(self, caster)
    end
  end,
  draw_func = function(self, caster)
    local rolling = self.rolling_time and self.rolling_time > 0
    local shown_roll
    if rolling then
      shown_roll = dicebinder_visible_roll(self)
    else
      shown_roll = self.last_roll or 0
    end
    local entry = DICE_ROLLS[shown_roll]
    local rolling_label
    if rolling then
      local guard = self.roll_guard_kind or "none"
      local elapsed = (self.rolling_total or DICE_ROLL_ANIMATION) - (self.rolling_time or 0)
      if guard == "surge" and elapsed <= 22 then
        rolling_label = "SURGE"
      elseif guard == "bank" and elapsed <= 22 then
        rolling_label = "BANK"
      elseif (self.rolling_time or 0) <= DICE_ROLL_FINAL_REVEAL_FRAMES and entry then
        rolling_label = entry.short
      elseif (self.rolling_time or 0) <= DICE_ROLL_REVEAL_FRAMES then
        rolling_label = "DROP"
      else
        rolling_label = "ROLL"
      end
    else
      rolling_label = nil
    end
    return ProceduralEffects.draw_die_badge(caster, {
      roll = shown_roll,
      remaining = self.next_roll or 0,
      interval = self.roll_interval or DICE_ROLL_INTERVAL,
      color = (rolling and entry and entry.color) or self.roll_color or COL_PALE_BLUE,
      label = rolling_label or (entry and entry.short) or "",
      buff_remaining = self.buff_time or 0,
      buff_duration = DICE_BUFF_DURATION,
      rolling = rolling,
      rolling_remaining = self.rolling_time or 0,
      rolling_total = self.rolling_total or DICE_ROLL_ANIMATION,
      lock_stage = self.roll_lock_stage or 0,
      lock_total = DICE_ROLL_LOCK_TOTAL,
      surge_stage = self.roll_surge_stage or 0,
      surge_total = DICE_ROLL_SURGE_TOTAL
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
