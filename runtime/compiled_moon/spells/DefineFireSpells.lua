local EventLog = require("ui.EventLog")
local GameObject = require("core.GameObject")
local Map = require("core.Map")
local DataW = require("DataWrapped")
local ObjectUtils = require("objects.ObjectUtils")
local SpellUtils = require("spells.SpellUtils")
local TypeEffectUtils = require("spells.TypeEffectUtils")
local draw_console_text, draw_console_effect
do
  local _obj_0 = require('ui.ConsoleUtils')
  draw_console_text, draw_console_effect = _obj_0.draw_console_text, _obj_0.draw_console_effect
end
local M = nilprotect({
  _fire = tosprite("spr_effects.fire-anim"),
  _burning = tosprite("spr_effects.burning")
})
local RingFireBase
RingFireBase = function(extension)
  return SpellUtils.spell_object_type(table.merge({
    types = {
      "Red"
    },
    base_init = function(self, args)
      assert(self._damage)
      self.cooldown = args.cooldown or 20
    end,
    _on_kill = function(self, obj)
      return nil
    end,
    _on_damage = function(self, obj)
      return nil
    end,
    base_on_step = function(self)
      if self.caster.is_ghost then
        GameObject.destroy(self)
        return
      end
      if self.caster.destroyed or self.caster.map ~= self.map then
        return
      end
      if self:every(20) then
        play_sound("sound/ringfire-loop.ogg")
      end
      return self:for_all_rings(function(x, y, damage_cooldown)
        local _list_0 = Map.radius_collision_check(self.map, 15, {
          x,
          y
        })
        for _index_0 = 1, #_list_0 do
          local _continue_0 = false
          repeat
            local obj = _list_0[_index_0]
            if damage_cooldown[obj] or (obj.team == nil) or obj.team == self.caster.team then
              _continue_0 = true
              break
            end
            if not obj.destroyed then
              if self:do_damage(obj, self._damage) then
                self:_on_kill(obj)
              end
              self:_on_damage(obj, self._damage)
            end
            play_sound("sound/ringfire-hit.ogg")
            damage_cooldown[obj] = self.cooldown
            _continue_0 = true
          until true
          if not _continue_0 then
            break
          end
        end
        for k, v in strictpairs(damage_cooldown) do
          v = v - 1
          damage_cooldown[k] = ((function()
            if v > 0 then
              return v
            else
              return nil
            end
          end)())
        end
      end)
    end,
    on_step = function(self)
      return self:base_on_step()
    end,
    on_draw = function(self)
      if self.caster.destroyed or self.caster.map ~= self.map then
        return
      end
      local alpha = 1.0
      if self:in_time_slice(0.0, 0.1) then
        alpha = self:get_progress(0.0, 0.1)
      elseif self:in_time_slice(0.9, 1) then
        alpha = 1.0 - self:get_progress(0.9, 1)
      end
      local frame = self:time_passed()
      return self:for_all_rings(function(x, y)
        return ObjectUtils.screen_draw(M._fire, {
          x,
          y
        }, alpha / 2, frame)
      end)
    end
  }, extension))
end
M.RingFireBase = RingFireBase
local SpawnedFire
SpawnedFire = RingFireBase({
  _damage = 0.2,
  init = function(self, args)
    self.damage_cooldowns = OrderedDict()
    self.initial_duration = args.duration
    self.fire_radius = args.fire_radius or 1
    return self:base_init(args)
  end,
  _on_kill = function(self, obj)
    return nil
  end,
  _on_damage = function(self, obj)
    local divider
    if self.fire_radius == 0 then
      divider = 1
    else
      divider = 1
    end
    if chance(.05 / divider) then
      local mon_name = SpellUtils.mon_title(obj)
      SpellUtils.message(self.caster, "Suddenly, " .. tostring(mon_name) .. " catches on fire!", COL_PALE_RED)
      return GameObject.add_to_level(SpawnedFire.create({
        caster = self.caster,
        fire_radius = 0,
        xy = obj.xy,
        cooldown = self.cooldown,
        duration = self.initial_duration
      }))
    end
  end,
  for_all_rings = function(self, f)
    local rad = self.fire_radius
    for dy = -rad, rad do
      for dx = -rad, rad do
        local _continue_0 = false
        repeat
          if math.abs(dx) + math.abs(dy) == 2 then
            _continue_0 = true
            break
          end
          local cx, cy
          do
            local _obj_0 = self.xy
            cx, cy = _obj_0[1], _obj_0[2]
          end
          local x, y = cx + dx * Map.TILE_SIZE, cy + dy * Map.TILE_SIZE
          if not self.caster:within_field_of_view({
            x,
            y
          }) then
            _continue_0 = true
            break
          end
          self.damage_cooldowns[dy] = self.damage_cooldowns[dy] or OrderedDict()
          self.damage_cooldowns[dy][dx] = self.damage_cooldowns[dy][dx] or OrderedDict()
          f(x, y, self.damage_cooldowns[dy][dx])
          _continue_0 = true
        until true
        if not _continue_0 then
          break
        end
      end
    end
  end
})
M.SpawnedFire = SpawnedFire
local RingOfFire = RingFireBase({
  _damage = 0.7,
  init = function(self, args)
    self.damage_cooldowns = OrderedDict()
    self.fireballs = { }
    self.angle_offset = 0
    self.cooldown = 30
    return self:base_init(args)
  end,
  _create_rings = function(self, radius_bonus)
    if radius_bonus == nil then
      radius_bonus = 0
    end
    local sub_radius = 20
    local n_points = 14 + self:caster_type_power()
    local radius = sub_radius * n_points / (2 * math.pi) + radius_bonus
    local step_angle = (2 * math.pi) / n_points
    return (function()
      local _accum_0 = { }
      local _len_0 = 1
      for i = 0, n_points - 1 do
        local angle = self.angle_offset + step_angle * i
        local _value_0 = {
          math.sin(angle) * radius,
          math.cos(angle) * radius
        }
        _accum_0[_len_0] = _value_0
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)()
  end,
  _on_kill = function(self, obj)
    return nil
  end,
  _on_damage = function(self, obj)
    return nil
  end,
  on_map_deinit = function(self)
    if self.caster ~= nil and not self.caster.destroyed then
      self.caster.__vision_radius_override = nil
    end
  end,
  on_step = function(self)
    if self.caster.destroyed or self.caster.map ~= self.map then
      return
    end
    self.caster.__vision_radius_override = 9
    self.angle_offset = self.angle_offset + (math.pi / 10 / 10)
    return self:base_on_step()
  end,
  for_all_rings = function(self, f)
    local radius_percentage = nil
    for i = 1, 10 do
      if self:percent_passed() <= i / 10 then
        radius_percentage = self:get_progress((i - 1) / 10, i / 10)
        break
      end
    end
    local rings = self:_create_rings(30 - 30 * radius_percentage)
    for idx = 1, #rings do
      local x, y
      do
        local _obj_0 = rings[idx]
        x, y = _obj_0[1], _obj_0[2]
      end
      self.damage_cooldowns[idx] = self.damage_cooldowns[idx] or OrderedDict()
      f(x + self.caster.x, y + self.caster.y, self.damage_cooldowns[idx])
    end
  end
})
M.RingOfFire = RingOfFire
DataW.effect_create({
  name = "Ring of Flames Stat Boost",
  stat_func = function(effect, obj, old, new)
    new.defence = new.defence + 4
    new.willpower = new.willpower + 4
  end
})
DataW.spell_create({
  name = "Ring of Flames",
  description = "",
  types = {
    "Red"
  },
  spr_spell = "spr_spells.spell_icon_ring_of_flames",
  prereq_func = function(caster)
    return true
  end,
  autotarget_func = function(caster)
    return caster.x, caster.y
  end,
  action_func = function(caster, x, y)
    local duration = 450 + 100 * bounds_percentage(TypeEffectUtils.get_power(caster, 'Red'), 0, 4)
    GameObject.add_to_level(RingOfFire.create({
      caster = caster,
      duration = duration
    }))
    return caster:add_effect("Ring of Flames Stat Boost", duration)
  end,
  console_draw_func = function(self, get_next)
    local damage = math.floor(RingOfFire._damage * 60 / 5)
    local power = self:effective_stats().magic + TypeEffectUtils.get_power(self, "Red")
    draw_console_effect(get_next(), tosprite("spr_spells.spell_icon_ring_of_flames"), {
      {
        COL_PALE_GREEN,
        damage
      },
      {
        COL_PALE_YELLOW,
        " damage per second"
      }
    })
    get_next()
    return draw_console_text(get_next(), {
      {
        COL_PALE_YELLOW,
        "Power: "
      },
      {
        COL_PALE_GREEN,
        power
      }
    })
  end,
  mp_cost = 30,
  spell_cooldown = 800,
  cooldown = 100
})
DataW.effect_create({
  name = "Dragonform",
  can_use_rest = false,
  fade_out = 5,
  effected_colour = {
    255,
    160,
    160
  },
  effected_sprite = "spr_effects.inner_flame",
  apply_func = function(self, caster, time_left)
    self.kill_tracker = caster.kills
    self.max_time = math.max(self.max_time or 0, time_left)
    self.extensions = 0
    self.fire_sprite_name = caster.sprite_name
    caster.sprite_name = 'spr_enemies.dragons.fire_dragon'
  end,
  stat_func = function(self, obj, old, new)
    new.defence = new.defence + 6
    new.strength = new.strength + 6
    new.willpower = new.willpower + 6
    new.speed = new.speed * 0.8
  end,
  remove_func = function(self, caster)
    caster.sprite_name = self.fire_sprite_name
  end,
  step_func = function(self, caster)
    local time_passed = self.max_time - self.time_left
    if time_passed % 20 == 0 then
      play_sound("sound/ringfire-loop.ogg")
    end
    local diff = math.max(caster.kills - self.kill_tracker, 0)
    for i = 1, diff do
      self.time_left = math.min(self.max_time * 1.5, self.time_left + 10)
      for _ in screens() do
        if caster:is_local_player() then
          EventLog.add("Your inner fire grows ...", {
            200,
            200,
            255
          })
        end
      end
      self.extensions = self.extensions + 1
    end
    self.kill_tracker = caster.kills
  end
})
DataW.spell_create({
  name = "Dragonform",
  description = "A form of great fire. For a limited time, you become a slow but immensely powerful dragon.",
  types = {
    "Red"
  },
  spr_spell = 'spr_enemies.dragons.fire_dragon',
  prereq_func = function(caster)
    return true
  end,
  autotarget_func = function(caster)
    return caster.x, caster.y
  end,
  action_func = function(caster, x, y)
    return caster:add_effect("Dragonform", 500 + 300 * bounds_percentage(caster.stats.level, 0, 10))
  end,
  console_draw_func = function(self, get_next)
    draw_console_effect(get_next(), tosprite("fire bolt"), {
      {
        COL_PALE_GREEN,
        "4x Fire Bolts"
      }
    })
    return draw_console_effect(get_next(), tosprite("spr_effects.fire-anim"), {
      {
        COL_PALE_GREEN,
        "Fire Bolts explode"
      }
    })
  end,
  mp_cost = 30,
  spell_cooldown = 1600,
  cooldown = 0
})
DataW.spell_create({
  name = "Fire Bolt",
  spr_spell = "fire bolt",
  description = "A fast bolt of fire. Hits a single target.",
  types = {
    "Red"
  },
  projectile = {
    speed = 7,
    damage_multiplier = 1.2,
    spr_attack = "fire bolt",
    on_damage = function(self, target, damage)
      if self.caster.destroyed then
        return
      end
      local r_pow = TypeEffectUtils.get_power(self.caster, 'Red')
      if self.caster:has_effect("Dragonform") then
        SpellUtils.message(self.caster, "Fire springs forth from the fire bolt!", COL_PALE_BLUE)
        return GameObject.add_to_level(SpawnedFire.create({
          fire_radius = 1,
          caster = self.caster,
          xy = self.xy,
          damage = damage,
          duration = 80
        }))
      elseif chance(0.1 + 0.02 * r_pow) then
        SpellUtils.message(self.caster, "Fire springs forth from the fire bolt!", COL_PALE_BLUE)
        return GameObject.add_to_level(SpawnedFire.create({
          fire_radius = 0,
          caster = self.caster,
          xy = self.xy,
          damage = damage,
          duration = 200
        }))
      end
    end
  },
  mp_cost = 10,
  cooldown = 35
})
return M
