local b2 = require('b2')
local Display = require("core.Display")
local GenerateUtils = require("maps.GenerateUtils")
local WIDTH, HEIGHT = 640, 480
math.randomseed(os.time())
local _body_distance
_body_distance = function()
  local input = b2.DistanceInput()
  local proxyA = b2.DistanceProxy()
  local proxyB = b2.DistanceProxy()
  local cache = b2.SimplexCache()
  local output = b2.DistanceOutput()
  return function(body1, body2)
    proxyA:Set(body1:GetFixtureList():GetShape(), 1)
    proxyB:Set(body2:GetFixtureList():GetShape(), 1)
    input.transformA = body1:GetTransform()
    input.transformB = body2:GetTransform()
    input.proxyA = proxyA
    input.proxyB = proxyB
    input.useRadii = true
    cache.count = 0
    b2.b2Distance(output, cache, input)
    return output.pointA, output.pointB, output.distance
  end
end
local body_distance = _body_distance()
local make_polygon
make_polygon = function(world, bd, points, density)
  if density == nil then
    density = 0.0
  end
  local shape = b2.PolygonShape()
  shape:Set(points)
  local body = world:CreateBody(bd)
  body:CreateFixture(shape, density)
  return body
end
local make_enclosure
make_enclosure = function(world, outer_area)
  local bd = b2.BodyDef()
  bd.position:Set(0, 0)
  return (function()
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, #outer_area do
      local x1, y1 = unpack(outer_area[i])
      local x2, y2 = unpack(outer_area[(i % #outer_area) + 1])
      local points = { }
      table.insert(points, {
        x1 - 1,
        y1 - 1
      })
      table.insert(points, {
        x2 + 1,
        y1 - 1
      })
      table.insert(points, {
        x2 + 1,
        y2 + 1
      })
      table.insert(points, {
        x1 - 1,
        y2 + 1
      })
      local _value_0
      do
        local _with_0 = world:CreateBody(bd)
        _with_0:CreateFixture((function()
          do
            local _with_1 = b2.PolygonShape()
            _with_1:Set(points)
            return _with_1
          end
        end)(), 1.0)
        _value_0 = _with_0
      end
      _accum_0[_len_0] = _value_0
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()
end
local make_dynamic_shape
make_dynamic_shape = function(world, size, xy)
  local x, y
  x, y = xy[1], xy[2]
  local points = GenerateUtils.skewed_ellipse_points(-size[1] / 2, -size[2] / 2, size[1], size[2], math.floor(math.random() * 8 + 4))
  local body = make_polygon(world, (function()
    do
      local _with_0 = b2.BodyDef()
      _with_0.type = b2.dynamicBody
      _with_0.position:Set(x, y)
      return _with_0
    end
  end)(), points, 1.0)
  return body
end
local create_polygons
create_polygons = function(world)
  return (function()
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, 10 do
      local size = {
        math.random() * WIDTH / 5 + 15,
        math.random() * HEIGHT / 5 + 15
      }
      if i > 5 then
        size[1] = size[1] * 2
        size[2] = size[2] * 2
      end
      local position = {
        math.random() * WIDTH,
        math.random() * HEIGHT
      }
      local _value_0 = make_dynamic_shape(world, size, position)
      _accum_0[_len_0] = _value_0
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()
end
local TIME_STEP = 1.0 / 60
local ITERS = 10
local make_simulation
make_simulation = function(outer_area)
  local drawer = b2.GLDrawer()
  local world = b2.World(b2.Vec2(0.0, 0.0))
  world:SetDebugDraw(drawer)
  drawer:SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
  local enclosure_parts = make_enclosure(world, outer_area)
  local polygons = create_polygons(world)
  local min_dist
  min_dist = function(body)
    local min_body = nil
    local min_d = math.huge
    for _index_0 = 1, #polygons do
      local _continue_0 = false
      repeat
        local b = polygons[_index_0]
        if b == body then
          _continue_0 = true
          break
        end
        local _, dist
        _, _, dist = body_distance(b, body)
        if dist < min_d then
          min_d = dist
          min_body = b
        end
        _continue_0 = true
      until true
      if not _continue_0 then
        break
      end
    end
    return min_body, min_d
  end
  return {
    step = function(self)
      local t = timer_create()
      for i = 1, 1 do
        for _index_0 = 1, #polygons do
          local body = polygons[_index_0]
          local pos = body:GetPosition()
          if pos.x < 1 or pos.x > WIDTH - 1 or pos.y < 1 or pos.y > HEIGHT - 1 then
            local position = {
              math.random() * WIDTH,
              math.random() * HEIGHT
            }
            body:SetTransform(b2.Vec2(position[1], position[2]), body:GetAngle())
          end
          local dx, dy = WIDTH / 2 - pos.x, HEIGHT / 2 - pos.y
          local mag = math.sqrt(dx * dx + dy * dy)
          dx = dx / mag * 1024
          dy = dy / mag * 1024
          body:SetLinearVelocity(b2.Vec2(dx, dy))
        end
        world:Step(TIME_STEP, ITERS, ITERS)
      end
      print(t:get_microseconds() / 1000)
      world:DrawDebugData()
      for _index_0 = 1, #polygons do
        local body = polygons[_index_0]
        local pos = body:GetPosition()
        local min_body, min_d = min_dist(body)
        local color = COL_WHITE
        local text = tostring(min_d)
        Display.set_drawing_region({
          0,
          0,
          WIDTH,
          HEIGHT
        })
        FONT:draw({
          color = color,
          origin = Display.CENTER
        }, {
          pos.x,
          pos.y
        }, text)
      end
    end
  }
end
local main
main = function()
  local enclosure = {
    {
      0,
      0
    },
    {
      WIDTH,
      0
    },
    {
      WIDTH,
      HEIGHT
    },
    {
      0,
      HEIGHT
    }
  }
  local sim = make_simulation(enclosure)
  Display.initialize("Demo", {
    WIDTH,
    HEIGHT
  }, false)
  rawset(_G, "FONT", Display.font_load("fonts/Gudea-Regular.ttf", 14))
  return Display.draw_loop(function()
    return sim:step()
  end)
end
return main()
