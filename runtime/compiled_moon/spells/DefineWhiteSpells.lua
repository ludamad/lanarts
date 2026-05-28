local GameObject = require("core.GameObject")
local Map = require("core.Map")
local Display = require("core.Display")
local DataW = require("DataWrapped")
local ObjectUtils = require("objects.ObjectUtils")
local SpellUtils = require("spells.SpellUtils")
local TypeEffectUtils = require("spells.TypeEffectUtils")
local M = nilprotect({
  _flash = tosprite("spr_effects.tornado"),
  _bolt = tosprite("spr_spells.spell_icon_lightning_bolt")
})
local BOSS_STUN_CHANCE_MULTIPLIER = 0.35
local BOSS_STUN_DURATION_MULTIPLIER = 0.45
local BOSS_SPEAR_PUSH_CHANCE = 0.05
local try_stun
try_stun = function(caster, mon, cooldown)
  if cooldown == nil then
    cooldown = nil
  end
  local white_power = TypeEffectUtils.get_power(caster, "White")
  local resist = math.min(1.25, TypeEffectUtils.get_resistance(mon, "White"))
  if not mon.is_enemy then
    resist = 0.5
    white_power = -5
  end
  local stun_chance = resist
  local stun_duration = cooldown or (100 + white_power * 10) * resist
  if mon.unique then
    stun_chance = math.min(1, stun_chance) * BOSS_STUN_CHANCE_MULTIPLIER
    stun_duration = stun_duration * BOSS_STUN_DURATION_MULTIPLIER
  end
  if chance(stun_chance) then
    mon:add_effect("Stunned", math.max(1, math.floor(stun_duration)))
    if Map.object_visible(mon) then
      return play_sound("sound/ringfire-hit.ogg")
    end
  end
end
local Flash = SpellUtils.spell_object_type({
  types = {
    "White"
  },
  init = function(self, args)
    self._damage = args.damage
    self.cooldown = args.cooldown or 50
    self.damage_cooldowns = OrderedDict()
  end,
  _on_kill = function(self, obj)
    return nil
  end,
  on_step = function(self)
    if self.caster.is_ghost then
      GameObject.destroy(self)
      return
    end
    if self.caster.destroyed or self.caster.map ~= self.map then
      return
    end
    return self:for_all_rings(function(x, y, damage_cooldown)
      local _list_0 = Map.radius_collision_check(self.map, 15, {
        x,
        y
      })
      for _index_0 = 1, #_list_0 do
        local _continue_0 = false
        repeat
          local mon = _list_0[_index_0]
          if damage_cooldown[mon] or (mon.team == nil) or mon.team == self.caster.team then
            _continue_0 = true
            break
          end
          if not mon.destroyed and self:do_damage(mon, self._damage) then
            self:_on_kill(mon)
          end
          try_stun(self.caster, mon)
          damage_cooldown[mon] = self.cooldown
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
  for_all_rings = function(self, f)
    local rad = 1
    for dy = -rad, rad do
      for dx = -rad, rad do
        local _continue_0 = false
        repeat
          if dx * dx + dy * dy > rad * rad then
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
  end,
  on_draw = function(self)
    if self.caster.destroyed or self.caster.map ~= self.map then
      return
    end
    local alpha = 1.0
    if self:in_time_slice(0.0, 0.5) then
      alpha = self:get_progress(0.0, 0.5)
    elseif self:in_time_slice(0.5, 1) then
      alpha = 1.0 - self:get_progress(0.5, 1)
    else
      alpha = 0
    end
    local frame = self:time_passed()
    return self:for_all_rings(function(x, y)
      return ObjectUtils.screen_draw(M._bolt, {
        x,
        y
      }, alpha / 2, frame)
    end)
  end
})
M.Flash = Flash
local interpolate
interpolate = function(val, min, max, omin, omax)
  val = math.max(min, math.min(max, val))
  local w1 = max - min
  local w2 = omax - omin
  return (val - min) * (w2 / w1) + omin
end
DataW.spell_create({
  name = "Chain Lightning",
  description = "A slow, powerful blast of lightning. The blast can bounce off an enemy twice before dissipating.",
  spr_spell = "spr_spells.spell_icon_chain_lightning",
  projectile = {
    types = {
      "White"
    },
    spr_attack = "charge_effected",
    speed = 4,
    number_of_target_bounces = 3,
    on_hit_func = function(self, target, atkstats)
      local white_power = TypeEffectUtils.get_power(self.caster, "White")
      local prob = interpolate(white_power, 0, 5, 0.05, 0.2)
      if chance(prob) then
        return try_stun(self.caster, target, 30)
      end
    end
  },
  mp_cost = 20,
  cooldown = 65,
  spell_cooldown = 65
})
DataW.spell_create({
  name = "Energy Spear",
  description = "Allows you to create a durable spear of lightning.",
  spr_spell = "spr_effects.lightningbolt",
  types = {
    "White"
  },
  projectile = SpellUtils.passing_projectile({
    speed = 5,
    damage_multiplier = 1.65,
    n_steps = 500 / 8,
    can_wall_bounce = true,
    redamage_cooldown = 40,
    on_map_init = function(self)
      self.bonus = 0.9
    end,
    on_hit_func = function(self, target, atkstats)
      local push_chance
      if target.unique then
        push_chance = BOSS_SPEAR_PUSH_CHANCE
      else
        push_chance = .25
      end
      if not target.destroyed and chance(push_chance) then
        target:go_towards_if_free({
          self.vx * self.speed / 4,
          self.vy * self.speed / 4
        })
      end
      if self.attacked_map[target] ~= nil then
        return nil
      end
      local white_power = TypeEffectUtils.get_power(self.caster, "White")
      local prob = interpolate(white_power, 0, 5, 0.05, 0.2)
      if chance(prob) then
        try_stun(self.caster, target, 30)
      end
      self.bonus = self.bonus + 0.1
      atkstats.damage = atkstats.damage * self.bonus
    end
  }),
  mp_cost = 12,
  cooldown = 35
})
local draw_console_text
draw_console_text = function(xy, texts)
  local x, y
  x, y = xy[1], xy[2]
  for _index_0 = 1, #texts do
    local _des_0 = texts[_index_0]
    local color, text
    color, text = _des_0[1], _des_0[2]
    x = x + font_cached_load(settings.font, 10):draw({
      color = color,
      origin = Display.LEFT_CENTER
    }, {
      x,
      y
    }, text)
  end
  return nil
end
local draw_console_effect
draw_console_effect = function(xy, sprite, texts)
  local x, y
  x, y = xy[1], xy[2]
  sprite:draw({
    origin = Display.LEFT_CENTER
  }, {
    x,
    y + 4
  })
  return draw_console_text({
    x + Map.TILE_SIZE + 4,
    y
  }, texts)
end
DataW.spell_create({
  name = "Tornado Storm",
  description = "Tornados of swirling death.",
  spr_spell = "spr_effects.tornado",
  types = {
    "White"
  },
  console_draw_func = function(self, get_next)
    draw_console_effect(get_next(), M._flash, {
      {
        COL_PALE_GREEN,
        "Tornados burst into lightning"
      }
    })
    return draw_console_effect(get_next(), M._bolt, {
      {
        COL_PALE_GREEN,
        "5"
      },
      {
        COL_PALE_YELLOW,
        " damage per second"
      }
    })
  end,
  projectile = SpellUtils.passing_projectile({
    speed = 4,
    damage_multiplier = 0.9,
    n_steps = 30,
    can_wall_bounce = false,
    on_map_init = function(self)
      self.cx, self.cy = self.caster.x, self.caster.y
    end,
    on_step = function(self)
      if self.map ~= self.caster.map then
        GameObject.destroy(self)
        return
      end
      local dx, dy = self.caster.x - self.cx, self.caster.y - self.cy
      self.x = self.x + dx
      self.y = self.y + dy
      self.cx, self.cy = self.caster.x, self.caster.y
    end,
    on_deinit = function(self)
      return GameObject.add_to_level(Flash.create({
        caster = self.caster,
        duration = 30,
        damage = 10,
        xy = self.xy
      }))
    end,
    redamage_cooldown = 40
  }),
  mp_cost = 25,
  cooldown = 35,
  spell_cooldown = 300
})
DataW.effect_create({
  name = "Stunned",
  effected_sprite = "spr_bonuses.stun",
  effected_colour = {
    200,
    200,
    200
  },
  can_use_rest = false,
  can_use_spells = false,
  can_use_stairs = false,
  can_use_weapons = false,
  apply_func = function(self, obj)
    self.ox, self.oy = obj.x, obj.y
  end,
  step_func = function(self, obj)
    obj.x, obj.y = self.ox, self.oy
  end
})
return M
