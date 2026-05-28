local GameObject = require("core.GameObject")
local Map = require("core.Map")
local Display = require("core.Display")
local ObjectUtils = require("objects.ObjectUtils")
local LuaGameObject = require("objects.LuaGameObject")
local M = nilprotect({ })
M._spell_wall = Display.image_load("spr_spells/spell-wall.png")
M._spell_arrow = Display.image_load("spr_spells/arrow.png")
M.SpellWall = LuaGameObject.type_create()
local SPELL_WALL_RADIUS = 15
local SPELL_WALL_DEPTH = 100
local SPELL_WALL_DURATION = 150
M.SpellWall.try_solidify = function(self)
  local tile_xy = ObjectUtils.tile_xy(self, true)
  if self.solidified then
    local collisions = Map.rectangle_collision_check(self.map, {
      self.x - 16,
      self.y - 16,
      self.x + 16,
      self.y + 16
    }, self)
    collisions = table.filter(collisions, function(self)
      return self.solid
    end)
    if #collisions == 0 then
      return
    end
    self.solidified = false
    Map.tile_set_solid(self.map, tile_xy, false)
    return
  end
  if Map.tile_is_solid(self.map, tile_xy) then
    return
  end
  local collisions = Map.rectangle_collision_check(self.map, {
    self.x - 16,
    self.y - 16,
    self.x + 16,
    self.y + 16
  }, self)
  collisions = table.filter(collisions, function(self)
    return self.solid
  end)
  if #collisions ~= 0 then
    return
  end
  Map.tile_set_solid(self.map, tile_xy, true)
  self.solidified = true
end
M.SpellWall.init = function(self, args)
  local points, point_index
  points, point_index = args.points, args.point_index
  M.SpellWall.parent_init(self, points[point_index], SPELL_WALL_RADIUS, true, SPELL_WALL_DEPTH)
  self.traits = self.traits or { }
  self.solidified = false
  self.sprite = M._spell_wall
  self.n_steps = 0
end
M.SpellWall.on_map_init = function(self)
  return self:try_solidify()
end
M.SpellWall.on_step = function(self)
  self.n_steps = self.n_steps + 1
  if self.n_steps >= SPELL_WALL_DURATION then
    local tile_xy = ObjectUtils.tile_xy(self, true)
    if self.solidified then
      Map.tile_set_solid(self.map, tile_xy, false)
    end
    return GameObject.destroy(self)
  else
    return self:try_solidify()
  end
end
M.SpellWall.on_draw = function(self)
  if Display.object_within_view(self) then
    local alpha = (self.n_steps / SPELL_WALL_DURATION) * 2
    if alpha > 1.0 then
      alpha = 1 - (alpha - 1)
    end
    alpha = math.max(0, math.min(alpha, 1))
    return ObjectUtils.screen_draw(self.sprite, self.xy, alpha)
  end
end
M._skull_trap_sprite = tosprite("spr_effects.deathnote")
M.SkullTrap = LuaGameObject.type_create()
M.SkullTrap.init = function(self, args)
  local points, point_index, vx, vy, caster, duration
  points, point_index, vx, vy, caster, duration = args.points, args.point_index, args.vx, args.vy, args.caster, args.duration
  M.SkullTrap.parent_init(self, points[point_index], 8, true, SPELL_WALL_DEPTH)
  self.traits = self.traits or { }
  self.sprite = M._skull_trap_sprite
  self.n_steps = 0
  self.caster = caster
  self.duration = duration
  self.n_ramp = 45
  self.vx = vx
  self.vy = vy
end
M.SkullTrap.on_step = function(self)
  if Map.radius_tile_check(self.map, {
    self.x + self.vx,
    self.y
  }, self.radius) then
    self.vx = 0
  end
  if Map.radius_tile_check(self.map, {
    self.x,
    self.y + self.vy
  }, self.radius) then
    self.vy = 0
  end
  self.x = self.x + self.vx
  self.y = self.y + self.vy
  self.n_steps = self.n_steps + 1
  if self.n_steps >= self.duration then
    local tile_xy = ObjectUtils.tile_xy(self, true)
    if self.solidified then
      Map.tile_set_solid(self.map, tile_xy, false)
    end
    return GameObject.destroy(self)
  else
    local collisions = Map.rectangle_collision_check(self.map, {
      self.x - 8,
      self.y - 8,
      self.x + 8,
      self.y + 8
    }, self)
    local had_collision = false
    for _index_0 = 1, #collisions do
      local _continue_0 = false
      repeat
        local col = collisions[_index_0]
        if not col.is_combat_object then
          _continue_0 = true
          break
        end
        if self.collided[col.id] and self.collided[col.id] + 45 > self.n_steps then
          _continue_0 = true
          break
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    if had_collision then
      collisions = Map.rectangle_collision_check(self.map, {
        self.x - 64,
        self.y - 64,
        self.x + 64,
        self.y + 64
      }, self)
      for _index_0 = 1, #collisions do
        local col = collisions[_index_0]
        self.collided[col.id] = self.n_steps
        local eff = self.caster:effective_stats()
        col:damage(eff.magic / 2, 2 + eff.magic * 0.2, 0, 1, self.caster)
      end
    end
  end
end
M.SkullTrap.on_draw = function(self)
  if Display.object_within_view(self) then
    local min = math.min(self.n_ramp, (function()
      if self.n_steps > self.duration / 2 then
        return math.abs(self.n_steps - self.duration)
      else
        return self.n_steps
      end
    end)())
    local alpha = (min / self.n_ramp)
    alpha = math.max(0, math.min(alpha, 1))
    return ObjectUtils.screen_draw(self.sprite, self.xy, alpha)
  end
end
M.SpellSpikes = LuaGameObject.type_create()
M.SpellSpikes.init = function(self, args)
  local points, point_index, caster, duration
  points, point_index, caster, duration = args.points, args.point_index, args.caster, args.duration
  M.SpellSpikes.parent_init(self, points[point_index], 16, true, SPELL_WALL_DEPTH)
  self.traits = self.traits or { }
  self.sprite = M._spell_arrow
  self.n_steps = 0
  self.caster = caster
  self.duration = duration
  self.n_ramp = 45
  self.collided = { }
end
M.SpellSpikes.on_step = function(self)
  self.n_steps = self.n_steps + 1
  if self.n_steps >= self.duration then
    local tile_xy = ObjectUtils.tile_xy(self, true)
    if self.solidified then
      Map.tile_set_solid(self.map, tile_xy, false)
    end
    return GameObject.destroy(self)
  else
    local collisions = Map.rectangle_collision_check(self.map, {
      self.x - 8,
      self.y - 8,
      self.x + 8,
      self.y + 8
    }, self)
    for _index_0 = 1, #collisions do
      local _continue_0 = false
      repeat
        local col = collisions[_index_0]
        if not col.is_combat_object then
          _continue_0 = true
          break
        end
        if self.collided[col.id] and self.collided[col.id] + 45 > self.n_steps then
          _continue_0 = true
          break
        end
        self.collided[col.id] = self.n_steps
        local eff = self.caster:effective_stats()
        col:damage(eff.magic / 2, 2 + eff.magic * 0.2, 0, 1)
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end
end
M.SpellSpikes.on_draw = function(self)
  if Display.object_within_view(self) then
    local min = math.min(self.n_ramp, (function()
      if self.n_steps > self.duration / 2 then
        return math.abs(self.n_steps - self.duration)
      else
        return self.n_steps
      end
    end)())
    local alpha = (min / self.n_ramp)
    alpha = math.max(0, math.min(alpha, 1))
    return ObjectUtils.screen_draw(self.sprite, self.xy, alpha)
  end
end
M.SummonAbility = LuaGameObject.type_create({
  init = function(self, args)
    local monster, on_summon, caster, xy, duration
    monster, on_summon, caster, xy, duration = args.monster, args.on_summon, args.caster, args.xy, args.duration
    M.SummonAbility.parent_init(self, xy, 16, true, SPELL_WALL_DEPTH)
    self.duration = duration
    self.caster = caster
    self.n_steps = 0
    self.on_summon = on_summon or do_nothing
    self.monster = monster
  end,
  on_map_init = function(self)
    for i = 1, 100 do
      local _continue_0 = false
      repeat
        do
          local x, y
          do
            local _obj_0 = self.xy
            x, y = _obj_0[1], _obj_0[2]
          end
          x = x + random(-64, 64)
          y = y + random(-64, 64)
          if not Map.radius_visible({
            x,
            y
          }, 1) then
            _continue_0 = true
            break
          end
          local collisions = Map.rectangle_collision_check(self.map, {
            x - 16,
            y - 16,
            x + 16,
            y + 16
          }, self)
          collisions = table.filter(collisions, function(self)
            return self.solid
          end)
          if #collisions > 0 then
            _continue_0 = true
            break
          end
          if Map.object_tile_check(self, {
            x,
            y
          }) then
            _continue_0 = true
            break
          end
          self.xy = {
            x,
            y
          }
          break
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
  end,
  on_step = function(self)
    self.n_steps = self.n_steps + 1
    if self.n_steps >= self.duration then
      local mon = GameObject.enemy_create({
        type = self.monster,
        xy = self.xy,
        team = self.caster.team
      })
      if type(self.on_summon) == "function" then
        self.on_summon(mon)
      else
        print("***** WARNING: on_summon was not function:")
        pretty(self.on_summon)
      end
      self.caster.summoned[mon] = 1
      return GameObject.destroy(self)
    end
  end,
  on_draw = function(self)
    if Map.object_visible(self) then
      local alpha = self.n_steps / self.duration
      local sprite = monster_sprite(self.monster)
      return ObjectUtils.screen_draw(sprite, self.xy, alpha, nil, nil, COL_GRAY)
    end
  end
})
return M
