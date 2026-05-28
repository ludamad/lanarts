local b2 = require('b2')
local GenerateUtils = require("maps.GenerateUtils")
local Keys = require("core.Keyboard")
local DebugUtils = require("maps.DebugUtils")
local B2Utils = require("maps.B2Utils")
local B2World
B2World = require("maps.B2World").B2World
local B2ShapeConnector = newtype({
  init = function(self, b2world)
    self.world = b2world.world
    self.all_bodies = { }
    self.bodies = { }
    self.tunnels = { }
    self.connection_sets = { }
    return self:_init(b2world.dynamic_bodies, b2world.static_bodies)
  end,
  _init = function(self, dynamic_bodies, static_bodies)
    for _index_0 = 1, #dynamic_bodies do
      local _des_0 = dynamic_bodies[_index_0]
      local body, fixtures
      body, fixtures = _des_0.body, _des_0.fixtures
      local o = {
        body = body,
        fixtures = fixtures,
        connections = { }
      }
      append(self.all_bodies, o)
      append(self.connection_sets, {
        o
      })
      append(self.bodies, o)
    end
    for _index_0 = 1, #static_bodies do
      local o = static_bodies[_index_0]
      append(self.all_bodies, o)
    end
    return nil
  end,
  _get_set_index_of = function(self, body)
    for i = 1, #self.connection_sets do
      if table.contains(self.connection_sets[i], body) then
        return i
      end
    end
    return error("Body should be contained in a connection set!")
  end,
  _record_connection = function(self, body_a, body_b)
    append(body_a.connections, body_b)
    append(body_b.connections, body_a)
    local ind_a = self:_get_set_index_of(body_a)
    local ind_b = self:_get_set_index_of(body_b)
    if ind_a ~= ind_b then
      local C = self.connection_sets
      local _list_0 = C[ind_b]
      for _index_0 = 1, #_list_0 do
        local body = _list_0[_index_0]
        append(C[ind_a], body)
      end
      local last = C[#C]
      C[ind_b] = last
      C[#C] = nil
    end
  end,
  _try_connection = function(self, index_a, index_b)
    local TUNNEL_WIDTH = 2
    local DIST_THRESHOLD = 0.0
    if #self.bodies < 2 then
      error("Cannot connect a set of just one body!")
    end
    local body_a, body_b = self.bodies[index_a], self.bodies[index_b]
    if table.contains(body_a.connections, body_b) then
      return false
    end
    local p1, p2, dist = B2Utils.body_distance(body_a, body_b)
    if not p1 or not p2 then
      return false
    end
    local x1, y1 = p1.x, p1.y
    local x2, y2 = p2.x, p2.y
    local dx, dy
    if dist <= DIST_THRESHOLD then
      local bp1 = body_a.body:GetPosition()
      dx, dy = p2.x - bp1.x, p2.y - bp1.y
    else
      dx, dy = p2.x - p1.x, p2.y - p1.y
    end
    local mag = math.sqrt(dx * dx + dy * dy)
    dx, dy = dx / mag * TUNNEL_WIDTH, dy / mag * TUNNEL_WIDTH
    x1 = x1 - (dx * TUNNEL_WIDTH * 2)
    y1 = y1 - (dy * TUNNEL_WIDTH * 2)
    x2 = x2 + (dx * TUNNEL_WIDTH * 2)
    y2 = y2 + (dy * TUNNEL_WIDTH * 2)
    local _list_0 = self.all_bodies
    for _index_0 = 1, #_list_0 do
      local _continue_0 = false
      repeat
        local b = _list_0[_index_0]
        if b == body_a or b == body_b then
          _continue_0 = true
          break
        end
        if B2Utils.ray_cast(b.fixtures, p1, p2) then
          return false
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    local shape = {
      {
        x1 - dy,
        y1 + dx
      },
      {
        x1 + dy,
        y1 - dx
      },
      {
        x2 + dy,
        y2 - dx
      },
      {
        x2 - dy,
        y2 + dx
      }
    }
    local tunnel_body = self.world:CreateBody((function()
      do
        local _with_0 = b2.BodyDef()
        _with_0.position:Set(0, 0)
        return _with_0
      end
    end)())
    tunnel_body:CreateFixture((function()
      do
        local _with_0 = b2.PolygonShape()
        _with_0:Set(shape)
        return _with_0
      end
    end)(), 0.0)
    append(self.tunnels, {
      index_a = index_a,
      index_b = index_b,
      polygon = shape
    })
    self:_record_connection(body_a, body_b)
    return true
  end,
  _try_random_connection = function(self, rng)
    local index_a = rng:random(1, #self.bodies)
    local index_b = rng:random(index_a + 1, #self.bodies + 1)
    return self:_try_connection(index_a, index_b)
  end,
  do_random_connection = function(self, rng)
    local MAX_TRIALS = 100
    for trial = 1, MAX_TRIALS do
      if self:_try_random_connection(rng) then
        return true
      end
    end
    return false
  end,
  do_ensure_connected = function(self, rng)
    return nil
  end
})
local _spread_shapes
_spread_shapes = function(args)
  local b2world = assert(args.b2world)
  local n_iterations = args.n_iterations or 100
  local n_subiterations = args.n_subiterations or 10
  local clump_once_near = args.clump_once_near or false
  local mode = args.mode or 'towards_fixed_shapes'
  b2world:visualize("Starting point")
  for i = 1, n_iterations do
    if mode == 'towards_fixed_shapes' then
      B2Utils.set_velocities_to_fixed_set(b2world.dynamic_bodies, b2world.static_bodies, clump_once_near)
    elseif mode == 'towards_center' then
      B2Utils.set_velocities_to_point(b2world.dynamic_bodies, 0, 0)
    else
      error("Unrecognized mode '" .. tostring(mode) .. "'!")
    end
    b2world.world:Step(0.1, n_subiterations, n_subiterations)
    if i % 10 == 0 then
      b2world:visualize("Iteration " .. tostring(i))
    end
  end
  for i, _des_0 in ipairs(b2world.dynamic_bodies) do
    local body
    body = _des_0.body
    args.sync(i, body)
  end
end
local _b2_world_from_args
_b2_world_from_args = function(args)
  local b2world = B2World.create()
  local _list_0 = args.regions
  for _index_0 = 1, #_list_0 do
    local region = _list_0[_index_0]
    b2world:add_body(region.polygons)
  end
  local _list_1 = (args.fixed_polygons or { })
  for _index_0 = 1, #_list_1 do
    local polygon = _list_1[_index_0]
    b2world:add_body({
      polygon
    }, 0.0)
  end
  return b2world
end
local spread_map_regions
spread_map_regions = function(args)
  local regions = args.regions
  return _spread_shapes({
    b2world = _b2_world_from_args(args),
    n_iterations = args.n_iterations,
    n_subiterations = args.n_subiterations,
    clump_once_near = args.clump_once_near,
    mode = args.mode,
    sync = function(i, body)
      local r = regions[i]
      local t = body:GetTransform()
      local _list_0 = r.polygons
      for _index_0 = 1, #_list_0 do
        local p = _list_0[_index_0]
        B2Utils.transform_polygon(t, p)
      end
    end
  })
end
local connect_map_regions
connect_map_regions = function(args)
  assert(args.rng, "Need 'rng' object")
  local b2world = _b2_world_from_args(args)
  local connector = B2ShapeConnector.create(b2world)
  for i = 1, args.n_connections or #args.regions do
    if not connector:do_random_connection(args.rng) then
      break
    end
    b2world:visualize("Random connection " .. tostring(i))
  end
  local _list_0 = connector.tunnels
  for _index_0 = 1, #_list_0 do
    local tunnel = _list_0[_index_0]
    append(args.regions[tunnel.index_a].tunnels, tunnel)
  end
end
local visualize_map_regions
visualize_map_regions = function(args)
  if not DebugUtils.is_debug_visualization() then
    return
  end
  local b2world = _b2_world_from_args(args)
  return b2world:visualize(args.title)
end
return {
  connect_map_regions = connect_map_regions,
  spread_map_regions = spread_map_regions,
  visualize_map_regions = visualize_map_regions,
  has_overlaps = function(args)
    local b2world = _b2_world_from_args(args)
    return b2world:has_overlaps()
  end,
  create_b2world = _b2_world_from_args
}
