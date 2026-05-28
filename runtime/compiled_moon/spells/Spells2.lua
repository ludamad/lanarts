local EventLog = require("ui.EventLog")
local GameObject = require("core.GameObject")
local GameState = require("core.GameState")
local Map = require("core.Map")
local World = require("core.World")
local Bresenham = require("core.Bresenham")
local Display = require("core.Display")
local SpellObjects = require("objects.SpellObjects")
local DataW = require("DataWrapped")
GameObject = require("core.GameObject")
require("spells.DefineFireSpells")
require("spells.DefineWhiteSpells")
require("spells.DefineBlackSpells")
local INFINITE_DURATION = 2 ^ 30
local mon_title
mon_title = function(mon)
  if mon.unique then
    return mon.name
  else
    return "the " .. tostring(mon.name)
  end
end
DataW.spell_create({
  name = "Minor Missile",
  spr_spell = "spr_effects.energy",
  description = "A low cost neutral bolt of energy. Hits a single target. The bolt can bounce off walls safely.",
  projectile = {
    speed = 6,
    damage_multiplier = 0.7,
    can_wall_bounce = false,
    on_damage = function(self, victim, final_damage, victim_died)
      if victim_died and self.origin then
        play_sound("sound/swish-10.wav")
        GameObject.animation_create({
          map = self.origin.map,
          xy = {
            self.origin.x + 10,
            self.origin.y
          },
          sprite = "spr_effects.mana",
          duration = 25,
          velocity = {
            -1,
            -1
          }
        })
        return self.origin:heal_mp(10)
      end
    end
  },
  mp_cost = 0,
  cooldown = 40
})
DataW.spell_create({
  name = "Magic Arrow",
  description = "Allows you to create an arrow of pure energy, requires a bow. Does well against strudy opponents.",
  spr_spell = "magic arrow",
  mp_cost = 20,
  projectile = {
    speed = 7.25,
    types = {
      "Piercing"
    },
    spr_attack = "crystal spear",
    range = 150,
    speed = 9,
    power = {
      base = {
        0,
        0
      },
      magic = 0.5,
      strength = 0.5
    }
  },
  cooldown = 30,
  resist_modifier = 0.5,
  prereq_func = function(self)
    return self.stats.weapon_type == "bows"
  end
})
DataW.spell_create({
  name = "Fireball",
  spr_spell = "fire ball",
  description = "A great, strong bolt of fire. Hits a single target.",
  projectile = {
    types = {
      "Red"
    },
    speed = 5,
    damage_multiplier = 2.00
  },
  mp_cost = 30,
  cooldown = 35
})
DataW.spell_create({
  name = "Mephitize",
  spr_spell = "spr_spells.cloud",
  description = "A debilitating ring of clouds that cause damage as well as reduced defenses and speed over time.",
  projectile = {
    speed = 2,
    cooldown = 105,
    types = {
      "Green"
    },
    on_hit_func = function(self, target, atkstats)
      return target:add_effect("Poison", {
        poison_rate = 25,
        time_left = 100,
        attacker = self.caster,
        damage = atkstats.damage,
        power = atkstats.power,
        magic_percentage = atkstats.magic_percentage,
        physical = atkstats.physical
      })
    end
  },
  mp_cost = 20,
  cooldown = 35,
  spell_cooldown = 800
})
DataW.spell_create({
  name = "Trepidize",
  spr_spell = "spr_spells.cause_fear",
  description = "An insidious apparition that instills the fear of death in enemies it hits.",
  projectile = {
    speed = 8,
    on_hit_func = function(self, target, atkstats)
      return target:add_effect("Fear", 100)
    end
  },
  mp_cost = 0,
  cooldown = 35,
  spell_cooldown = 1600
})
DataW.spell_create({
  name = "Water Bolt",
  description = "A short range frosty bolt of cold. Hits a single target.",
  types = {
    "Blue"
  },
  spr_spell = "spr_effects.iceball",
  projectile = {
    speed = 5,
    damage_multiplier = 1.35
  },
  mp_cost = 10,
  cooldown = 35
})
DataW.projectile_create({
  name = "Skullthrow",
  weapon_class = "magic",
  spr_spell = "spr_spells.skullthrow",
  types = {
    "Black"
  },
  range = 160,
  damage_type = {
    magic = 0.5,
    physical = 0.5
  },
  speed = 7.25,
  can_wall_bounce = true,
  mp_cost = 4,
  cooldown = 80,
  on_hit_func = function(self, target, atkstats)
    return target:add_effect("Poison", {
      time_left = 20,
      damage = atkstats.damage,
      attacker = self.caster,
      power = atkstats.power,
      poison_rate = 25,
      magic_percentage = atkstats.magic_percentage,
      physical = atkstats.physical
    })
  end
})
DataW.spell_create({
  name = "Ludaze",
  types = {
    "Black"
  },
  spr_spell = "spr_spells.ludaze",
  damage_type = {
    magic = 0.5,
    physical = 0.5
  },
  description = "Dazes and poisons all enemies in sight.",
  mp_cost = 25,
  cooldown = 0,
  spell_cooldown = 800,
  can_cast_with_held_key = true,
  fallback_to_melee = false,
  action_func = function(caster, x, y)
    caster:add_effect("Ludaze", 30)
    if Map.object_visible(caster) then
      play_sound("sound/ludaze.ogg")
    end
    local _list_0 = (Map.enemies_list(caster))
    for _index_0 = 1, #_list_0 do
      local _continue_0 = false
      repeat
        local mon = _list_0[_index_0]
        if not Map.object_visible(mon, mon.xy, caster) then
          _continue_0 = true
          break
        end
        mon:add_effect("Dazed", 100)
        target:add_effect("Poison", {
          time_left = 100,
          attacker = self.caster,
          damage = 8,
          power = caster.stats.magic,
          poison_rate = 25,
          magic_percentage = 1
        })
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    for _ in screens() do
      if caster:is_local_player() then
        EventLog.add("You daze all enemies in sight!", {
          200,
          200,
          255
        })
      elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " dazes all enemies in sight!", {
          200,
          200,
          255
        })
      end
    end
  end,
  autotarget_func = function(caster)
    return caster.x, caster.y
  end,
  prereq_func = function(caster)
    if not caster:has_effect("Berserk") and not caster:has_effect("Exhausted") and not caster:has_effect("Ice Form") then
      local _list_0 = (Map.enemies_list(caster))
      for _index_0 = 1, #_list_0 do
        local mon = _list_0[_index_0]
        if Map.object_visible(mon, mon.xy, caster) then
          return true
        end
      end
      EventLog.add("No monsters in sight!", COL_PALE_RED)
      return false
    end
    return false
  end
})
DataW.spell_create({
  name = "Ice Form",
  types = {
    "Blue"
  },
  spr_spell = "spr_spells.iceform",
  description = "Initiates Ice Form, a powerful ability for safe dungeoneering, preventing attacks and spells, and lowering speed drastically, but providing near immunity for 10 seconds.",
  mp_cost = 40,
  cooldown = 100,
  damage_type = {
    magic = 0.5,
    physical = 0.5
  },
  spell_cooldown = 1600,
  can_cast_with_held_key = false,
  fallback_to_melee = false,
  action_func = function(caster, x, y)
    caster:add_effect("Ice Form", 300)
    for _ in screens() do
      if caster:is_local_player() then
        EventLog.add("You enter a glacial state!", {
          200,
          200,
          255
        })
      elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " enters a glacial state!", {
          200,
          200,
          255
        })
      end
    end
  end,
  autotarget_func = function(caster)
    return caster.x, caster.y
  end,
  prereq_func = function(caster)
    if not caster.can_rest then
      EventLog.add("Ice Form requires perfect concentration!", {
        200,
        200,
        255
      })
      return false
    end
    return not caster:has_effect("Berserk") and not caster:has_effect("Exhausted") and not caster:has_effect("Ice Form")
  end
})
DataW.spell_create({
  name = "Baleful Regeneration",
  types = {
    "Black"
  },
  spr_spell = "spr_spells.regeneration",
  description = "You tap into necromancy to very quickly bind your wounds, until your are fully healed or its power runs out. Takes longer to run out the more willpower you have.",
  mp_cost = 0,
  cooldown = 35,
  can_cast_with_held_key = false,
  fallback_to_melee = false,
  spell_cooldown = 800,
  prereq_func = function(caster)
    if caster.stats.hp == caster:effective_stats().max_hp then
      return false
    end
    return not caster:has_effect("Baleful Regeneration") and not caster:has_effect("Exhausted")
  end,
  autotarget_func = function(caster)
    return caster.x, caster.y
  end,
  action_func = function(caster, x, y)
    local willpower
    willpower = caster:effective_stats().willpower
    caster:add_effect("Baleful Regeneration", 60 * 6 + (willpower - 7) * 30)
    for _ in screens() do
      if caster:is_local_player() then
        EventLog.add("You start to regenerate quickly!", {
          200,
          200,
          255
        })
        play_sound("sound/berserk.ogg")
      else
        EventLog.add(caster.name .. " starts to regenerate quickly!", {
          200,
          200,
          255
        })
      end
    end
  end
})
DataW.spell_create({
  name = "Summon Dark Aspect",
  types = {
    "Black"
  },
  spr_spell = "spr_spells.summon",
  description = "You summon a dark companion, at the cost of health. The companion is stronger depending on the caster's willpower.",
  mp_cost = 0,
  cooldown = 45,
  can_cast_with_held_key = false,
  fallback_to_melee = false,
  spell_cooldown = 320,
  autotarget_func = function(caster)
    return caster.x, caster.y
  end,
  prereq_func = function(caster)
    if not caster:has_effect("Necromancer") then
      if caster:is_local_player() then
        EventLog.add("You must be a necromancer to cast this spell!", {
          200,
          200,
          255
        })
      end
      return false
    end
    local amount = math.max(1, math.ceil((caster:effective_stats().willpower - 8) / 4))
    amount = math.min(amount, caster.stats.level)
    local n_summons
    n_summons = caster:get_effect("Summoner").n_summons
    if n_summons >= amount then
      if caster:is_local_player() then
        EventLog.add("You cannot currently control more than " .. tostring(amount) .. " aspects!", {
          200,
          200,
          255
        })
      end
      return false
    end
    return not caster:has_effect("Exhausted") and not (caster:has_effect("Summoning"))
  end,
  action_func = function(caster, x_unused, y_unused)
    if Map.object_visible(caster) then
      play_sound("sound/summon.ogg")
    end
    local monster = "Spectral Beast"
    if not (caster:has_effect("Summoning")) then
      local eff = caster:add_effect("Summoning", 5)
      eff.on_summon = function(obj)
        local willpower = caster:effective_stats().willpower
        local hp_bonus = math.floor(willpower * 3)
        local stat_bonus = math.floor(willpower / 3)
        obj.stats.hp = obj.stats.hp + hp_bonus
        obj.stats.max_hp = obj.stats.max_hp + hp_bonus
        obj.stats.strength = obj.stats.strength + stat_bonus
        obj.stats.magic = obj.stats.magic + stat_bonus
        obj.stats.defence = obj.stats.defence + stat_bonus
        obj.stats.willpower = obj.stats.willpower + stat_bonus
      end
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
})
DataW.spell_create({
  name = "Dash Attack",
  types = {
    "Green"
  },
  spr_spell = "expedite",
  description = "Dash in a straight line, hitting all enemies in your path.",
  mp_cost = 6,
  cooldown = 30,
  can_cast_with_held_key = false,
  fallback_to_melee = true,
  spell_cooldown = 15,
  action_func = function(self, x, y)
    local effect = self:add_effect("Dash Attack", {
      time_left = 20,
      angle = vector_direction(self.xy, {
        x,
        y
      })
    })
    for _ in screens() do
      if self:is_local_player() then
        EventLog.add("You dash valiantly forward!", {
          200,
          200,
          255
        })
      end
    end
  end,
  prereq_func = function(self)
    return not self:has_ranged_weapon()
  end,
  autotarget_func = function(self)
    local dx, dy
    do
      local _obj_0 = self.last_moved_direction
      dx, dy = _obj_0[1], _obj_0[2]
    end
    return self.x + dx * 32, self.y + dy * 32
  end
})
DataW.spell_create({
  name = "Link of Loyalty",
  types = {
    "Black"
  },
  spr_spell = "spr_spells.forgelink",
  description = "You summon a linked companion near an enemy, sending them to immediate combat but taking damage whenever they take damage. The type of companion depends on your amulet's summoning aspect.",
  mp_cost = 20,
  cooldown = 0,
  can_cast_with_held_key = false,
  fallback_to_melee = false,
  spell_cooldown = 40,
  prereq_func = function(caster)
    local n_summons
    n_summons = caster:get_effect("Summoner").n_summons
    if n_summons >= 2 then
      for _ in screens() do
        if caster:is_local_player() then
          EventLog.add("You cannot currently control more than " .. tostring(2) .. " aspects!", {
            200,
            200,
            255
          })
        end
      end
      return false
    end
    return not caster:has_effect("Exhausted") and not (caster:has_effect("Summoning"))
  end,
  action_func = function(caster, x, y)
    play_sound("sound/lifelink.ogg")
    local monster = "Cloud Elemental"
    if not (caster:has_effect("Summoning")) then
      local eff = caster:add_effect("Summoning", 20)
      eff.summon_xy = {
        x,
        y
      }
      eff.on_summon = function(obj)
        local lifelink = obj:add_effect("Lifelink", INFINITE_DURATION)
        lifelink.linker = caster
        return append(caster:get_effect("Lifelinker").links, obj)
      end
      eff.monster = monster
      eff.duration = 5
    end
  end
})
return DataW.spell_create({
  name = "Unlink",
  types = {
    "Black"
  },
  spr_spell = "spr_spells.unlink",
  description = "You break the link with all life-linked monsters. Monsters you have summoned are returned to their domain. Heals a small amount of health per monster.",
  mp_cost = 0,
  cooldown = 0,
  can_cast_with_held_key = false,
  fallback_to_melee = false,
  spell_cooldown = 40,
  prereq_func = function(caster)
    local n_summons
    n_summons = caster:get_effect("Summoner").n_summons
    if n_summons == 0 then
      for _ in screens() do
        if caster:is_local_player() then
          EventLog.add("You have no lifelinked monsters!", {
            200,
            200,
            255
          })
        end
      end
      return false
    end
    return not caster:has_effect("Exhausted") and not (caster:has_effect("Summoning"))
  end,
  action_func = function(caster, x, y)
    local eff = caster:get_effect("Lifelinker")
    play_sound("sound/unlink.ogg")
    local links
    links, eff.links = eff.links, { }
    for _index_0 = 1, #links do
      local _continue_0 = false
      repeat
        local link = links[_index_0]
        if link.destroyed then
          _continue_0 = true
          break
        end
        link:direct_damage(link.stats.hp + 1)
        caster:heal_hp(25)
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end,
  autotarget_func = function(caster)
    return caster.x, caster.y
  end
})
