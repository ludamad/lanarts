local B2World
B2World = require("maps.B2World").B2World
local B2Utils = require("maps.B2Utils")
local b2world_from_args
b2world_from_args = function(args)
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
local _try_tunnel
_try_tunnel = function(b2world, index_a, index_b)
  local TUNNEL_WIDTH = 2
  local DIST_THRESHOLD = 0.0
  local bodies = b2world.dynamic_bodies
  if #bodies < 2 then
    error("Cannot connect a set of just one body!")
  end
  local body_a, body_b = bodies[index_a], bodies[index_b]
  local p1, p2, dist = B2Utils.body_distance(body_a, body_b)
  if not p1 or not p2 then
    return nil
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
  for _index_0 = 1, #bodies do
    local _continue_0 = false
    repeat
      local b = bodies[_index_0]
      if b == body_a or b == body_b then
        _continue_0 = true
        break
      end
      if B2Utils.ray_cast(b.fixtures, p1, p2) then
        return nil
      end
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return { }, {
    x1,
    y1
  }, {
    x2,
    y2
  }
end
local _edges_fully_connected
_edges_fully_connected = function(edges)
  local visited
  do
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, #edges do
      _accum_0[_len_0] = false
      _len_0 = _len_0 + 1
    end
    visited = _accum_0
  end
  local nodes = {
    1
  }
  while #nodes > 0 do
    local next_nodes = { }
    for _index_0 = 1, #nodes do
      local node = nodes[_index_0]
      visited[node] = true
      table.insert_all(next_nodes, edges[node])
    end
    nodes = next_nodes
  end
  for _index_0 = 1, #visited do
    local v = visited[_index_0]
    if not v then
      return false
    end
  end
  return true
end
local _try_connect_map_regions
_try_connect_map_regions = function(b2world, create_graph)
  local tunnel_bodies = { }
  local edges
  do
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, #b2world.dynamic_bodies do
      _accum_0[_len_0] = { }
      _len_0 = _len_0 + 1
    end
    edges = _accum_0
  end
  local pos_pairs = { }
  local create_edge
  create_edge = function(index_a, index_b)
    index_a, index_b = math.min(index_a, index_b), math.max(index_a, index_b)
    if table.contains(edges[index_a], index_b) then
      return false
    end
    local body, p1, p2 = _try_tunnel(b2world, index_a, index_b)
    if body then
      assert(p1 and p2)
      append(pos_pairs, {
        p1,
        p2
      })
      append(edges[index_a], index_b)
      return true
    end
    return false
  end
  if not create_graph(create_edge) then
    return nil
  end
  if not _edges_fully_connected(edges) then
    for _index_0 = 1, #tunnel_bodies do
      local body = tunnel_bodies[_index_0]
      b2world:remove_body(body)
    end
    return nil
  end
  return pos_pairs
end
local connect_map_regions
connect_map_regions = function(args, max_tries)
  if max_tries == nil then
    max_tries = 100
  end
  local b2world = b2world_from_args(args)
  for i = 1, max_tries do
    local pos_pairs = _try_connect_map_regions(b2world, args.create_graph)
    if pos_pairs then
      return pos_pairs
    end
  end
  return nil
end
return nilprotect({
  b2world_from_args = b2world_from_args,
  connect_map_regions = connect_map_regions
})
