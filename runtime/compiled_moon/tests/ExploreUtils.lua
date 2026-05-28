local GameState = require("core.GameState")
local World = require("core.World")
local GameObject = require("core.GameObject")
local PathFinding = require("core.PathFinding")
local Map = require("core.Map")
local Keyboard = require("core.Keyboard")
local ObjectUtils = require("objects.ObjectUtils")
local HARDCODED_AI_SEED = 12345657
local ASTAR_BUFFER = PathFinding.astar_buffer_create()
local _sim
_sim = function(k)
  return GameState._simulate_key_press(Keyboard[k])
end
local _collisions
_collisions = function(player, type)
  if type == nil then
    type = nil
  end
  local collisions = Map.rectangle_collision_check(player.map, {
    player.x - 8,
    player.y - 8,
    player.x + 8,
    player.y + 8
  }, player)
  if not type then
    return collisions
  end
  return table.filter(collisions, function(self)
    return GameObject.get_type(self) == type
  end)
end
local _objects
_objects = function(player, type)
  if type == nil then
    type = nil
  end
  local objects = Map.objects_list(player.map)
  if not type then
    return objects
  end
  return table.filter(objects, function(self)
    return GameObject.get_type(self) == type
  end)
end
local portal_planner
portal_planner = function(player)
  return nilprotect({
    portal_next = { },
    level_connections = { },
    last_touched_portal = false,
    _current_label = function(self)
      return Map.map_label(player.map)
    end,
    _get_label_distances = function(self, root_label)
      local distance_levels = {
        {
          root_label
        }
      }
      local distance_to = {
        [root_label] = 1
      }
      local dist = 1
      while dist <= #distance_levels do
        local labels = distance_levels[dist]
        for _index_0 = 1, #labels do
          local label = labels[_index_0]
          local _list_0 = (self.level_connections[label] or { })
          for _index_1 = 1, #_list_0 do
            local conn = _list_0[_index_1]
            if not distance_to[conn] then
              distance_levels[dist + 1] = distance_levels[dist + 1] or { }
              append(distance_levels[dist + 1], conn)
              distance_to[conn] = dist + 1
            end
          end
        end
        dist = dist + 1
      end
      return distance_to
    end,
    step = function(self)
      if self.last_touched_portal and not self.portal_next[self.last_touched_portal] then
        local map
        map = self.last_touched_portal.map
        if player.map ~= map then
          self.portal_next[self.last_touched_portal] = Map.map_label(map)
        end
        self.level_connections[self:_current_label()] = self.level_connections[self:_current_label()] or { }
        append(self.level_connections[self:_current_label()], Map.map_label(map))
        self.level_connections[Map.map_label(map)] = self.level_connections[Map.map_label(map)] or { }
        append(self.level_connections[Map.map_label(map)], self:_current_label())
      end
      local _list_0 = _collisions(player, "feature")
      for _index_0 = 1, #_list_0 do
        local portal = _list_0[_index_0]
        self.last_touched_portal = portal
        break
      end
    end,
    closest_portal = function(self, map_label)
      if map_label == self:_current_label() then
        return nil
      end
      local distances = self:_get_label_distances(map_label)
      local current_distance = distances[self:_current_label()]
      if current_distance == nil then
        return nil
      end
      local closest, closest_distance = nil, math.huge
      local _list_0 = _objects(player, "feature")
      for _index_0 = 1, #_list_0 do
        local _continue_0 = false
        repeat
          local portal = _list_0[_index_0]
          local next = self.portal_next[portal]
          if not next then
            _continue_0 = true
            break
          end
          if distances[next] == nil then
            _continue_0 = true
            break
          end
          if distances[next] < current_distance then
            local obj_dist = vector_distance(portal.xy, player.xy)
            if obj_dist < closest_distance then
              closest = portal
              closest_distance = obj_dist
            end
          end
          _continue_0 = true
        until true
        if not _continue_0 then
          break
        end
      end
      return closest
    end,
    unused_portals = function(self)
      local portals = _objects(player, "feature")
      return table.filter(portals, function(p)
        return not p.has_been_used
      end)
    end
  })
end
local path_planner
path_planner = function(player)
  return nilprotect({
    stored_path = { },
    coord = false,
    get_next_coord = function(self)
      while true do
        local next = self.stored_path[self.coord]
        if not next then
          return nil
        end
        local xy
        xy = player.xy
        if vector_distance(next, xy) >= 6 then
          return next
        end
        self.coord = self.coord + 1
      end
    end,
    target = false,
    target_map = false,
    n_till_refresh = 0,
    step = function(self)
      self.n_till_refresh = math.max(self.n_till_refresh - 1, 0)
    end,
    set_path_towards = function(self, obj)
      if not obj or obj.destroyed then
        self.target = false
        self.target_map = false
        self.stored_path = { }
        self.coord = 1
        return
      end
      local map, tile_xy
      map, tile_xy = player.map, player.tile_xy
      self.target = obj
      self.target_map = map
      self.stored_path = ASTAR_BUFFER:calculate_path(map, tile_xy, obj.tile_xy)
      self.coord = 2
    end,
    next_direction_towards = function(self)
      if player.map ~= self.target_map then
        self.target_map = false
        self.target = false
      end
      if not self.target then
        return nil
      end
      if self.n_till_refresh <= 0 then
        self:set_path_towards(self.target)
        self.n_till_refresh = 32
      end
      local coord = self:get_next_coord()
      if not coord then
        return nil
      end
      local tx, ty
      tx, ty = coord[1], coord[2]
      local x, y
      x, y = player.x, player.y
      local dx, dy = tx - x, ty - y
      if dx ~= 0 then
        dx = dx / math.abs(dx)
      end
      if dy ~= 0 then
        dy = dy / math.abs(dy)
      end
      return {
        dx,
        dy
      }
    end
  })
end
local ai_state
ai_state = function(player)
  return {
    portal_planner = portal_planner(player),
    path_planner = path_planner(player),
    key_items = {
      "Azurite Key",
      "Dandelite Key"
    },
    rng = require("mtwist").create(HARDCODED_AI_SEED),
    step = function(self)
      self.path_planner:step()
      return self.portal_planner:step()
    end,
    next_key_item = function(self)
      local _list_0 = self.key_items
      for _index_0 = 1, #_list_0 do
        local _continue_0 = false
        repeat
          do
            local item = _list_0[_index_0]
            if require("core.GlobalData").keys_picked_up[item] then
              _continue_0 = true
              break
            end
            if require("core.GlobalData").lanarts_picked_up[item] then
              _continue_0 = true
              break
            end
            return item
          end
          _continue_0 = true
        until true
        if not _continue_0 then
          break
        end
      end
      return nil
    end,
    queued_movements = { },
    get_next_direction = function(self)
      if #self.queued_movements > 0 then
        local dir = self.queued_movements[#self.queued_movements]
        self.queued_movements[#self.queued_movements] = nil
        return dir
      end
      local dir = self.path_planner:next_direction_towards()
      if dir then
        return dir
      end
      local next_obj = nil
      local _list_0 = _objects(player, "item")
      for _index_0 = 1, #_list_0 do
        local obj = _list_0[_index_0]
        if obj.type == self:next_key_item() then
          next_obj = obj
          break
        end
      end
      if not next_obj then
        if self:next_key_item() == "Azurite Key" then
          for i = 2, 1, -1 do
            if next_obj then
              break
            end
            next_obj = self.portal_planner:closest_portal("Snake Pit " .. i)
          end
          for i = 3, 1, -1 do
            if next_obj then
              break
            end
            next_obj = self.portal_planner:closest_portal("Temple " .. i)
          end
        elseif self.key_items[2] == "" then
          for i = 3, 1, -1 do
            if next_obj then
              break
            end
            next_obj = self.portal_planner:closest_portal("Outpost " .. i)
          end
        end
      end
      if next_obj then
        self.path_planner:set_path_towards(next_obj)
        return self.path_planner:next_direction_towards()
      end
      return nil
    end,
    _closest = function(self, objs)
      local o = nil
      local min_dist_sqr = math.huge
      for _index_0 = 1, #objs do
        local obj = objs[_index_0]
        local dx, dy = obj.x - player.x, obj.y - player.y
        local dist_sqr = dx * dx + dy * dy
        if dist_sqr < min_dist_sqr then
          min_dist_sqr = dist_sqr
          o = obj
        end
      end
      return assert(o)
    end,
    get_next_wander_direction = function(self)
      local portals = _objects(player, "actor")
      local objs = table.filter(portals, function(p)
        return p.team ~= player.team
      end)
      if #objs == 0 then
        objs = self.portal_planner:unused_portals()
      end
      local next_obj
      if #objs == 0 then
        objs = _objects(player, "feature")
        next_obj = self.rng:random_choice(objs)
      else
        next_obj = self:_closest(objs)
      end
      self.path_planner:set_path_towards(next_obj)
      local dir = self.path_planner:next_direction_towards()
      return dir
    end
  }
end
return {
  portal_planner = portal_planner,
  path_planner = path_planner,
  ai_state = ai_state,
  ASTAR_BUFFER = ASTAR_BUFFER
}
