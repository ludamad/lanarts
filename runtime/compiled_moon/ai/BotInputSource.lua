local ProgrammableInputSource = require("input.ProgrammableInputSource")
local GameState = require("core.GameState")
local World = require("core.World")
local GameObject = require("core.GameObject")
local PathFinding = require("core.PathFinding")
local Map = require("core.Map")
local ExploreUtils = require("tests.ExploreUtils")
World = require("core.World")
local ObjectUtils = require("objects.ObjectUtils")
local HARDCODED_AI_SEED = 12345678
local load_from_global
local initialize
initialize = function(self)
  local GlobalData = require("core.GlobalData")
  if GlobalData.ai_state then
    load_from_global(self)
    return
  end
  random_seed(HARDCODED_AI_SEED)
  self._lpx = 0
  self._lpy = 0
  self._ai_state = ExploreUtils.ai_state(self.player)
  self._rng = require("mtwist").create(HARDCODED_AI_SEED)
  self._used_portals = { }
  self._queued = { }
  self._n_same_square = 0
  self._use_portal = false
  self.player:gain_xp(100000)
  self.player.stats.max_hp = 100000
  self.player.stats.hp = 100000
end
load_from_global = function(self)
  local GlobalData = require("core.GlobalData")
  for key, value in pairs(GlobalData.ai_state or { }) do
    self[key] = value
  end
end
local save_to_global
save_to_global = function(self)
  local GlobalData = require("core.GlobalData")
  local ai_state = { }
  for key, value in pairs(self) do
    ai_state[key] = value
  end
  GlobalData.ai_state = ai_state
end
return newtype({
  parent = ProgrammableInputSource,
  init = function(self, player)
    ProgrammableInputSource.init(self, player)
    return initialize(self)
  end,
  step = function(self)
    self._ai_state:step()
    local player = self.player
    local dir = nil
    self:_fighter_action()
    if self._lpx == player.x and self._lpy == player.y then
      self._n_same_square = self._n_same_square + 1
    else
      self._n_same_square = 0
    end
    self._lpx, self._lpy = player.x, player.y
  end,
  save_to_global = save_to_global,
  _goto_random_portal = function(self)
    local portals = { }
    local _list_0 = Map.objects_list(self.player.map)
    for _index_0 = 1, #_list_0 do
      local obj = _list_0[_index_0]
      if GameObject.get_type(obj) == "feature" then
        append(portals, obj)
      end
    end
    local portal = self._rng:random_choice(portals)
    self.player.xy = portal.xy
    append(self._queued, {
      1,
      1
    })
    self._use_portal = true
  end,
  _should_end = function(self)
    if not self.input_source then
      return false
    end
    if os.getenv("LANARTS_OVERWORLD_TEST") and self._n_inputs > 10000 then
      return true
    end
    local player = self.player
    if self._n_same_square < 100 then
      return false
    end
    local _list_0 = Map.objects_list(player.map)
    for _index_0 = 1, #_list_0 do
      local obj = _list_0[_index_0]
      if GameObject.get_type(obj) == "actor" and obj.team ~= player.team then
        return false
      end
    end
    return true
  end,
  _try_move_action = function(self, dx, dy)
    self:set("move_direction", {
      dx,
      dy
    })
    return true
  end,
  _fighter_action = function(self)
    local USE_DASH = self._rng:randomf() < 0.01
    local USE_KNOCK = self._rng:randomf() < 0.1
    local USE_ITEM = self._rng:randomf() < 0.1
    local SELL_ITEM = self._rng:randomf() < 0.1
    local RANDOM_MOVE = self._rng:randomf() < 0.01
    local dir = nil
    if not RANDOM_MOVE and not self._use_portal then
      if USE_DASH then
        self:set("use_spell_slot", 2)
      elseif USE_KNOCK then
        self:set("use_spell_slot", 1)
      elseif USE_ITEM then
        self:set("use_item_slot", self._rng:random(0, 40))
      elseif SELL_ITEM and #self.player:inventory() > 30 then
        self:set("sell_item_slot", self._rng:random(0, 40))
      end
    end
    if not self._use_portal and self:_attack_action() and not USE_DASH and not USE_KNOCK and not USE_ITEM then
      dir = {
        0,
        0
      }
    end
    if #self._queued > 0 then
      dir = self._queued[1]
      table.remove(self._queued, 1)
    end
    if not dir and RANDOM_MOVE then
      dir = {
        0,
        0
      }
      while dir[1] == 0 and dir[2] == 0 do
        local dx = self._rng:random_choice({
          -1,
          0,
          1
        })
        local dy = self._rng:random_choice({
          -1,
          0,
          1
        })
        dir = {
          dx,
          dy
        }
      end
      if dir then
        for i = 1, 4 do
          append(self._queued, dir)
        end
      end
    end
    if not dir then
      dir = self:_try_explore()
    end
    if not dir then
      dir = self._ai_state:get_next_direction()
      if dir then
        for i = 1, 4 do
          append(self._queued, dir)
        end
      end
    end
    if not dir then
      dir = self._ai_state:get_next_wander_direction()
      if dir then
        for i = 1, 4 do
          append(self._queued, dir)
        end
      end
    end
    if dir then
      self:set("move_direction", dir)
    end
    self._use_portal = false
    return true
  end,
  _attack_action = function(self)
    local player = self.player
    local enemies = Map.enemies_list(self.player)
    for _index_0 = 1, #enemies do
      local enemy = enemies[_index_0]
      if Map.object_visible(enemy, enemy.xy, self.player) then
        if player.class_name == 'Necromancer' then
          if player.stats.hp < player.stats.max_hp * 0.3 and not player:has_effect("Baleful Regeneration") then
            self:set("use_spell_slot", 1)
          else
            self:set("use_spell_slot", 0)
          end
        elseif player.class_name == 'Fighter' or player.class_name == 'Templar' then
          self:set("should_use_weapon", true)
        else
          self:set("use_spell_slot", self._rng:random(0, 4))
        end
        return true
      end
    end
    return false
  end,
  _try_rest_if_needed = function(self)
    local player = self.player
    if not player:can_benefit_from_rest() then
      return false
    end
    local enemies = Map.enemies_list(player)
    for _index_0 = 1, #enemies do
      local enemy = enemies[_index_0]
      if Map.object_visible(enemy, enemy.xy, player) then
        return false
      end
    end
    return true
  end,
  _try_explore = function(self)
    local player = self.player
    return player:direction_towards_unexplored()
  end,
  _n_same_square = 0
})
