local b2 = require('b2')
local Display = require("core.Display")
local GenerateUtils = require("maps.GenerateUtils")
local Keys = require("core.Keyboard")
local WIDTH, HEIGHT = 640, 480
local B2Utils = require("maps.B2Utils")
local rng = require("mtwist").create(os.time())
local make_polygon
make_polygon = function(w, h, points)
  return GenerateUtils.skewed_ellipse_points(rng, {
    0,
    0
  }, {
    w,
    h
  }, points)
end
local transform_polygon
transform_polygon = function(t, polygon)
  return (function()
    local _accum_0 = { }
    local _len_0 = 1
    for _index_0 = 1, #polygon do
      local _des_0 = polygon[_index_0]
      local x, y
      x, y = _des_0[1], _des_0[2]
      local vec = b2.b2Mul(t, b2.Vec2(x, y))
      local _value_0 = {
        vec.x,
        vec.y
      }
      _accum_0[_len_0] = _value_0
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()
end
local Shape = newtype({
  init = function(self, polygons, outer_polygon)
    if outer_polygon == nil then
      outer_polygon = false
    end
    self.polygons = polygons
    self.outer_polygon = outer_polygon
    self.x = 0
    self.y = 0
  end,
  b2Update = function(self, body, shift_polygons)
    if shift_polygons == nil then
      shift_polygons = false
    end
    local t = body:GetTransform()
    return self:apply_transform(t, shift_polygons)
  end,
  apply_transform = function(self, t, shift_polygons)
    if shift_polygons == nil then
      shift_polygons = false
    end
    local x, y
    do
      local _obj_0 = t.p
      x, y = _obj_0.x, _obj_0.y
    end
    if shift_polygons then
      self.x, self.y = 0, 0
    else
      self.x, self.y = x, y
      t.p:Set(0, 0)
    end
    do
      local _accum_0 = { }
      local _len_0 = 1
      local _list_0 = self.polygons
      for _index_0 = 1, #_list_0 do
        local shape = _list_0[_index_0]
        _accum_0[_len_0] = transform_polygon(t, shape)
        _len_0 = _len_0 + 1
      end
      self.polygons = _accum_0
    end
    if self.outer_polygon then
      self.outer_polygon = transform_polygon(t, self.outer_polygon)
    end
    return t.p:Set(x, y)
  end,
  translate = function(self, x, y)
    local _list_0 = self.polygons
    for _index_0 = 1, #_list_0 do
      local shape = _list_0[_index_0]
      for _index_1 = 1, #shape do
        local point = shape[_index_1]
        point[1] = point[1] + x
        point[2] = point[2] + y
      end
    end
    return nil
  end,
  bbox = function(self)
    local x1, y1 = math.huge, math.huge
    local x2, y2 = -math.huge, -math.huge
    local _list_0 = self.polygons
    for _index_0 = 1, #_list_0 do
      local shape = _list_0[_index_0]
      for _index_1 = 1, #shape do
        local _des_0 = shape[_index_1]
        local x, y
        x, y = _des_0[1], _des_0[2]
        x1, y1 = math.min(x1, x), math.min(y1, y)
        x2, y2 = math.max(x2, x), math.max(y2, y)
      end
    end
    return {
      x1,
      y1,
      x2,
      y2
    }
  end,
  b2Body = function(self, world, density)
    if density == nil then
      density = 0.0
    end
    local body = world:CreateBody((function()
      do
        local _with_0 = b2.BodyDef()
        if density > 0.0 then
          _with_0.type = b2.dynamicBody
        end
        _with_0.position:Set(self.x, self.y)
        return _with_0
      end
    end)())
    local fixtures
    do
      local _accum_0 = { }
      local _len_0 = 1
      local _list_0 = self.polygons
      for _index_0 = 1, #_list_0 do
        local shape = _list_0[_index_0]
        _accum_0[_len_0] = body:CreateFixture((function()
          do
            local _with_0 = b2.PolygonShape()
            _with_0:Set(shape)
            return _with_0
          end
        end)(), density)
        _len_0 = _len_0 + 1
      end
      fixtures = _accum_0
    end
    local outer_fixture
    if self.outer_polygon then
      outer_fixture = body:CreateFixture((function()
        do
          local _with_0 = b2.PolygonShape()
          _with_0:Set(outer_polygon)
          return _with_0
        end
      end)(), density)
    else
      outer_fixture = nil
    end
    return body, outer_fixture, fixtures
  end
})
local body_distance
local _body_distance
_body_distance = function(o1, others)
  local mp1, mp2, mdist = nil, nil, math.huge
  for _index_0 = 1, #others do
    local _continue_0 = false
    repeat
      local o2 = others[_index_0]
      if o1 == o2 then
        _continue_0 = true
        break
      end
      local p1, p2, dist = body_distance(o1, o2)
      if dist < mdist then
        mp1, mp2, mdist = p1, p2, dist
      end
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return mp1, mp2, mdist
end
body_distance = function(o1, o2)
  if o2.body == nil then
    return _body_distance(o1, o2)
  end
  local t1, fixs1 = o1.body:GetTransform(), o1.fixtures
  local t2, fixs2 = o2.body:GetTransform(), o2.fixtures
  return B2Utils.shape_set_distance(t1, fixs1, t2, fixs2)
end
local scale_polygon
scale_polygon = function(points, scale_x, scale_y)
  local cx, cy = 0, 0
  for _index_0 = 1, #points do
    local _des_0 = points[_index_0]
    local x, y
    x, y = _des_0[1], _des_0[2]
    cx, cy = cx + x, cy + y
  end
  cx, cy = cx / #points, cy / #points
  return (function()
    local _accum_0 = { }
    local _len_0 = 1
    for _index_0 = 1, #points do
      local _des_0 = points[_index_0]
      local x, y
      x, y = _des_0[1], _des_0[2]
      x = (x - cx) * scale_x + cx
      y = (y - cy) * scale_y + cy
      local _value_0 = {
        x,
        y
      }
      _accum_0[_len_0] = _value_0
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()
end
local B2ShapeSet = newtype({
  init = function(self, world, shapes, fixed_shapes)
    self.shapes = assert(shapes)
    self.fixed_shapes = assert(fixed_shapes)
    do
      local _accum_0 = { }
      local _len_0 = 1
      for _index_0 = 1, #shapes do
        local shape = shapes[_index_0]
        local body, outer_fixture, fixtures = shape:b2Body(world, 1.0)
        local _value_0 = {
          body = body,
          outer_fixture = outer_fixture,
          fixtures = fixtures,
          shape = shape
        }
        _accum_0[_len_0] = _value_0
        _len_0 = _len_0 + 1
      end
      self.b2shapes = _accum_0
    end
    do
      local _accum_0 = { }
      local _len_0 = 1
      for _index_0 = 1, #fixed_shapes do
        local shape = fixed_shapes[_index_0]
        local body, outer_fixture, fixtures = shape:b2Body(world, 0.0)
        local _value_0 = {
          body = body,
          outer_fixture = outer_fixture,
          fixtures = fixtures,
          shape = shape
        }
        _accum_0[_len_0] = _value_0
        _len_0 = _len_0 + 1
      end
      self.b2fixed_shapes = _accum_0
    end
  end,
  drift_shapes = function(self, scale_x, scale_y)
    local _list_0 = self.shapes
    for _index_0 = 1, #_list_0 do
      local shape = _list_0[_index_0]
      shape.x = shape.x * scale_x
      shape.y = shape.y * scale_y
    end
    return nil
  end,
  drift_bodies = function(self, scale_x, scale_y)
    local _list_0 = self.b2shapes
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local body
      body = _des_0.body
      local p = body:GetPosition()
      p.x = p.x * scale_x
      p.y = p.y * scale_y
    end
    return nil
  end,
  update_shapes = function(self)
    local _list_0 = self.b2shapes
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local body, shape
      body, shape = _des_0.body, _des_0.shape
      shape.x = body:GetPosition().x
      shape.y = body:GetPosition().y
      do
        local _accum_0 = { }
        local _len_0 = 1
        local _list_1 = shape.polygons
        for _index_1 = 1, #_list_1 do
          local polygon = _list_1[_index_1]
          do
            local _accum_1 = { }
            local _len_1 = 1
            for _index_2 = 1, #polygon do
              local _des_1 = polygon[_index_2]
              local px, py
              px, py = _des_1[1], _des_1[2]
              local x, y
              do
                local _obj_0 = body:GetWorldPoint(b2.Vec2(px, py))
                x, y = _obj_0.x, _obj_0.y
              end
              local _value_0 = {
                x - shape.x,
                y - shape.y
              }
              _accum_1[_len_1] = _value_0
              _len_1 = _len_1 + 1
            end
            _accum_0[_len_0] = _accum_1
          end
          _len_0 = _len_0 + 1
        end
        shape.polygons = _accum_0
      end
    end
    return nil
  end,
  to_shape = function(self, tunnels)
    if tunnels == nil then
      tunnels = { }
    end
    local polygons = { }
    local _list_0 = {
      self.b2shapes,
      tunnels
    }
    for _index_0 = 1, #_list_0 do
      local shape_sets = _list_0[_index_0]
      for _index_1 = 1, #shape_sets do
        local _des_0 = shape_sets[_index_1]
        local body, shape
        body, shape = _des_0.body, _des_0.shape
        local _list_1 = shape.polygons
        for _index_2 = 1, #_list_1 do
          local polygon = _list_1[_index_2]
          append(polygons, (function()
            local _accum_0 = { }
            local _len_0 = 1
            for _index_3 = 1, #polygon do
              local _des_1 = polygon[_index_3]
              local px, py
              px, py = _des_1[1], _des_1[2]
              local x, y
              do
                local _obj_0 = body:GetWorldPoint(b2.Vec2(px, py))
                x, y = _obj_0.x, _obj_0.y
              end
              local _value_0 = {
                x,
                y
              }
              _accum_0[_len_0] = _value_0
              _len_0 = _len_0 + 1
            end
            return _accum_0
          end)())
        end
      end
    end
    return Shape.create(polygons)
  end,
  unique_within = function(self, o1, o2, p1, p2)
    local _list_0 = self.b2shapes
    for _index_0 = 1, #_list_0 do
      local _continue_0 = false
      repeat
        local o = _list_0[_index_0]
        if o == o1 or o == o2 then
          _continue_0 = true
          break
        end
        if B2Utils.ray_cast(o.fixtures, p1, p2) then
          return false
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    return true
  end,
  set_velocities_to_fixed_set = function(self, clump_once_near)
    if clump_once_near == nil then
      clump_once_near = false
    end
    local _list_0 = self.b2shapes
    for _index_0 = 1, #_list_0 do
      local o = _list_0[_index_0]
      local pos = o.body:GetPosition()
      local p1, p2, dist = body_distance(o, self.b2fixed_shapes)
      if clump_once_near and #self.b2shapes > 1 then
        if dist < 100 then
          p1, p2, dist = body_distance(o, self.b2shapes)
        end
      end
      local dx, dy = (p2.x - p1.x), (p2.y - p1.y)
      o.body:SetLinearVelocity(b2.Vec2(dx, dy))
    end
    return nil
  end,
  has_overlaps = function(self)
    local _list_0 = self.b2shapes
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local body
      body = _des_0.body
      if body:OverlapsOtherBody() then
        return true
      end
    end
    return false
  end,
  set_velocities_to_point = function(self, x, y)
    local _list_0 = self.b2shapes
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local body
      body = _des_0.body
      local pos = body:GetPosition()
      local dx, dy = (x - pos.x), (y - pos.y)
      body:SetLinearVelocity(b2.Vec2(dx, dy))
    end
    return nil
  end,
  set_velocities_to_zero = function(self)
    local _list_0 = self.b2shapes
    for _index_0 = 1, #_list_0 do
      local _des_0 = _list_0[_index_0]
      local body
      body = _des_0.body
      body:SetLinearVelocity(b2.Vec2(0, 0))
    end
    return nil
  end
})
local B2ShapeConnector = newtype({
  init = function(self, world, b2shape_set)
    self.world = world
    self.all_bodies = { }
    self.bodies = { }
    self.tunnels = { }
    self.connection_sets = { }
    return self:_init(b2shape_set.b2shapes, b2shape_set.b2fixed_shapes)
  end,
  _init = function(self, b2shapes, b2fixed_shapes)
    for _index_0 = 1, #b2shapes do
      local _des_0 = b2shapes[_index_0]
      local body, shape, fixtures
      body, shape, fixtures = _des_0.body, _des_0.shape, _des_0.fixtures
      local o = {
        body = body,
        shape = shape,
        fixtures = fixtures,
        connections = { }
      }
      append(self.all_bodies, o)
      append(self.connection_sets, {
        o
      })
      append(self.bodies, o)
    end
    for _index_0 = 1, #b2fixed_shapes do
      local o = b2fixed_shapes[_index_0]
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
  _try_random_connection = function(self, rng)
    local TUNNEL_WIDTH = 2
    local DIST_THRESHOLD = 0.0
    if #self.bodies < 2 then
      error("Cannot connect a set of just one body!")
    end
    local index_a = rng:random(1, #self.bodies)
    local index_b = rng:random(index_a + 1, #self.bodies + 1)
    local body_a, body_b = self.bodies[index_a], self.bodies[index_b]
    if table.contains(body_a.connections, body_b) then
      return false
    end
    local p1, p2, dist = body_distance(body_a, body_b)
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
      body = tunnel_body,
      shape = Shape.create({
        shape
      })
    })
    self:_record_connection(body_a, body_b)
    return true
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
local VIS_W, VIS_H = 800, 600
local _FIRST_VISUALIZE = true
local visualize_world
visualize_world = function(world)
  return function(title)
    if _FIRST_VISUALIZE then
      Display.initialize("Demo", {
        VIS_W,
        VIS_H
      }, false)
      Display.set_world_region({
        -VIS_W / 2,
        -VIS_H / 2,
        VIS_W / 2,
        VIS_H / 2
      })
      _FIRST_VISUALIZE = false
    end
    return require("core.GameState").game_loop(function()
      local font = font_cached_load("fonts/Gudea-Regular.ttf", 14)
      world:DrawDebugData()
      if Keys.key_pressed("N") then
        return true
      end
      Display.reset_blend_func()
      font:draw({
        color = COL_WHITE,
        origin = Display.CENTER
      }, {
        100,
        100
      }, title)
      return false
    end)
  end
end
local spread_shapes
spread_shapes = function(args)
  local shapes = assert(args.shapes)
  local fixed_shapes = assert(args.fixed_shapes)
  local n_iterations = args.n_iterations or 100
  local n_subiterations = args.n_subiterations or 10
  local scale = args.scale or {
    1,
    1
  }
  local clump_once_near = args.clump_once_near or false
  local return_compound_shape = args.return_compound_shape or false
  local mode = args.mode or 'towards_fixed_shapes'
  local connect_after = args.connect_after or false
  local world = b2.World(b2.Vec2(0.0, 0.0))
  local visualize
  if args.visualize then
    visualize = visualize_world(world)
  else
    visualize = do_nothing
  end
  local b2shapes = B2ShapeSet.create(world, shapes, fixed_shapes)
  if args.visualize then
    world:SetDebugDraw((function()
      do
        local _with_0 = b2.GLDrawer()
        _with_0:SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
        return _with_0
      end
    end)())
  end
  visualize("Starting point")
  for i = 1, n_iterations do
    if mode == 'towards_fixed_shapes' then
      b2shapes:set_velocities_to_fixed_set(clump_once_near)
    elseif mode == 'towards_center' then
      b2shapes:set_velocities_to_point(0, 0)
    else
      error("Unrecognized mode '" .. tostring(mode) .. "'!")
    end
    world:Step(0.1, n_subiterations, n_subiterations)
    if i % 10 == 0 then
      visualize("Iteration " .. tostring(i))
    end
  end
  local tunnels = { }
  if connect_after then
    rng = assert(args.rng, "Need 'rng' object for connect_after=True!")
    local connector = B2ShapeConnector.create(world, b2shapes)
    for i = 1, 100 do
      if not connector:do_random_connection(rng) then
        break
      end
      visualize("Random connection " .. tostring(i))
    end
    tunnels = connector.tunnels
  end
  local compound_shape
  if return_compound_shape then
    compound_shape = b2shapes:to_shape(tunnels)
  else
    compound_shape = nil
  end
  b2shapes:update_shapes()
  return compound_shape
end
local sample_shape
sample_shape = function()
  local make_shapes
  make_shapes = function(n)
    local polygons
    do
      local _accum_0 = { }
      local _len_0 = 1
      for i = 4, 3 + n do
        _accum_0[_len_0] = make_polygon(i * 10, i * 10, i * math.random() + 3)
        _len_0 = _len_0 + 1
      end
      polygons = _accum_0
    end
    return (function()
      local _accum_0 = { }
      local _len_0 = 1
      for _index_0 = 1, #polygons do
        local poly = polygons[_index_0]
        local shape = Shape.create({
          poly
        })
        shape.x, shape.y = rng:randomf(-100, 100), rng:randomf(-100, 100)
        local _value_0 = shape
        _accum_0[_len_0] = _value_0
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)()
  end
  return spread_shapes({
    rng = rng,
    shapes = make_shapes(4),
    fixed_shapes = {
      Shape.create({
        make_polygon(8, 8)
      })
    },
    n_iterations = 50,
    mode = 'towards_fixed_shapes',
    clump_once_near = true,
    connect_after = true,
    return_compound_shape = true
  })
end
local spread_vis
spread_vis = function()
  local make_shapes
  make_shapes = function(n)
    local polygons
    do
      local _accum_0 = { }
      local _len_0 = 1
      for i = 4, 3 + n do
        _accum_0[_len_0] = make_polygon(i * 10, i * 10, i * math.random() + 3)
        _len_0 = _len_0 + 1
      end
      polygons = _accum_0
    end
    return (function()
      local _accum_0 = { }
      local _len_0 = 1
      for _index_0 = 1, #polygons do
        local poly = polygons[_index_0]
        local shape = Shape.create({
          poly
        })
        shape.x, shape.y = rng:randomf(-100, 100), rng:randomf(-100, 100)
        local _value_0 = shape
        _accum_0[_len_0] = _value_0
        _len_0 = _len_0 + 1
      end
      return _accum_0
    end)()
  end
  local timer = timer_create()
  local shapes = { }
  for i = 1, 4 do
    local compound_shape = spread_shapes({
      rng = rng,
      shapes = make_shapes(4),
      fixed_shapes = {
        Shape.create({
          make_polygon(8, 8)
        })
      },
      n_iterations = 50,
      mode = 'towards_fixed_shapes',
      clump_once_near = true,
      connect_after = true,
      return_compound_shape = true
    })
    compound_shape.x, compound_shape.y = rng:randomf(-500, 500), rng:randomf(-500, 500)
    append(shapes, compound_shape)
  end
  local compound = spread_shapes({
    shapes = shapes,
    rng = rng,
    visualize = true,
    fixed_shapes = {
      Shape.create({
        make_polygon(8, 8)
      })
    },
    n_iterations = 300,
    n_subiterations = 10,
    mode = 'towards_fixed_shapes',
    return_compound_shape = true,
    clump_once_near = true,
    connect_after = true
  })
  return print("Running took", timer:get_milliseconds())
end
local COLORS = {
  COL_BLACK,
  COL_WHITE,
  COL_BABY_BLUE,
  COL_PALE_YELLOW,
  COL_PALE_RED,
  COL_PALE_GREEN,
  COL_PALE_BLUE,
  COL_LIGHT_GRAY
}
while true do
  local SourceMap = require("core.SourceMap")
  local shape = sample_shape()
  local bbox = shape:bbox()
  local w, h = bbox[3] - bbox[1], bbox[4] - bbox[2]
  shape:translate(-bbox[1], -bbox[2])
  assert(w > 0 and h > 0, "w, h should be > 0, " .. tostring(w) .. ", " .. tostring(h))
  local tw, th = 8, 8
  Display.initialize("Demo", {
    800,
    600
  }, false)
  Display.set_world_region({
    0,
    0,
    w * tw,
    h * th
  })
  local map = SourceMap.map_create({
    rng = rng,
    label = "Test",
    size = {
      w,
      h
    },
    flags = SourceMap.FLAG_SOLID
  })
  local timer = timer_create()
  for i = 1, #shape.polygons do
    local polygon = shape.polygons[i]
    SourceMap.polygon_apply({
      map = map,
      area = {
        2,
        2,
        w - 2,
        h - 2
      },
      operator = {
        remove = SourceMap.FLAG_SOLID,
        add = SourceMap.FLAG_SEETHROUGH,
        content = 1
      },
      points = scale_polygon(polygon, 1.1, 1.1)
    })
  end
  print("Time spent rasterizing polygons: ", timer:get_milliseconds())
  require("core.GameState").game_loop(function()
    local row = { }
    for y = 1, h do
      SourceMap.get_row_content(map, row, 1, w, y)
      for x = 1, w do
        local color = COLORS[(row[x] % #COLORS) + 1]
        if color == COL_BLACK and row[x] ~= 0 then
          color = COL_GOLD
        end
        Display.draw_rectangle(color, {
          (x - 1) * tw,
          (y - 1) * th,
          x * tw,
          y * th
        })
      end
    end
    if Keys.key_pressed('R') then
      return true
    end
    return nil
  end)
end
