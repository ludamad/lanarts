local b2 = require('b2')
local _shape_distance
_shape_distance = function()
  local input = b2.DistanceInput()
  local proxyA = b2.DistanceProxy()
  local proxyB = b2.DistanceProxy()
  local cache = b2.SimplexCache()
  local output = b2.DistanceOutput()
  return function(shape1, transform1, shape2, transform2)
    proxyA:Set(shape1, 1)
    proxyB:Set(shape2, 1)
    input.transformA = transform1
    input.transformB = transform2
    input.proxyA = proxyA
    input.proxyB = proxyB
    input.useRadii = true
    cache.count = 0
    b2.b2Distance(output, cache, input)
    return output.pointA, output.pointB, output.distance
  end
end
local shape_distance = _shape_distance()
local _ray_cast
_ray_cast = function()
  local input = b2.RayCastInput()
  input.maxFraction = 1
  local output = b2.RayCastOutput()
  return function(fixs, p1, p2)
    input.p1 = p1
    input.p2 = p2
    local ind = 0
    for _index_0 = 1, #fixs do
      local fix = fixs[_index_0]
      if fix:RayCast(output, input, ind) then
        return true
      end
      ind = ind + 1
    end
    return false
  end
end
local ray_cast = _ray_cast()
local shape_set_distance
shape_set_distance = function(t1, fixs1, t2, fixs2)
  local mp1, mp2, mdist = nil, nil, math.huge
  local mf1, mf2 = nil, nil
  for _index_0 = 1, #fixs1 do
    local fix1 = fixs1[_index_0]
    local s1 = fix1:GetShape()
    for _index_1 = 1, #fixs2 do
      local fix2 = fixs2[_index_1]
      local s2 = fix2:GetShape()
      local p1, p2, dist = shape_distance(s1, t1, s2, t2)
      if dist < mdist then
        mp1, mp2, mdist = p1, p2, dist
        mf1, mf2 = fix1, fix2
      end
    end
  end
  return mp1, mp2, mdist, mf1, mf2
end
local create_body
create_body = function(world, polygons, density, x, y)
  if density == nil then
    density = 0.0
  end
  if x == nil then
    x = 0.0
  end
  if y == nil then
    y = 0.0
  end
  local body = world:CreateBody((function()
    do
      local _with_0 = b2.BodyDef()
      if density > 0.0 then
        _with_0.type = b2.dynamicBody
      end
      _with_0.position:Set(x, y)
      return _with_0
    end
  end)())
  local fixtures
  do
    local _accum_0 = { }
    local _len_0 = 1
    for _index_0 = 1, #polygons do
      local shape = polygons[_index_0]
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
  return {
    body = body,
    fixtures = fixtures,
    is_dynamic = density > 0.0
  }
end
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
  return shape_set_distance(t1, fixs1, t2, fixs2)
end
local set_velocities_to_fixed_set
set_velocities_to_fixed_set = function(bodies, fixed_bodies, clump_once_near)
  if clump_once_near == nil then
    clump_once_near = false
  end
  for _index_0 = 1, #bodies do
    local _continue_0 = false
    repeat
      local o = bodies[_index_0]
      local pos = o.body:GetPosition()
      local p1, p2, dist = body_distance(o, fixed_bodies)
      if not p1 or not p2 then
        _continue_0 = true
        break
      end
      if clump_once_near and #bodies > 1 then
        if dist < 100 then
          p1, p2, dist = body_distance(o, bodies)
        end
      end
      local dx, dy = (p2.x - p1.x), (p2.y - p1.y)
      o.body:SetLinearVelocity(b2.Vec2(dx, dy))
      _continue_0 = true
    until true
    if not _continue_0 then
      break
    end
  end
  return nil
end
local set_velocities_to_point
set_velocities_to_point = function(bodies, x, y)
  for _index_0 = 1, #bodies do
    local _des_0 = bodies[_index_0]
    local body
    body = _des_0.body
    local pos = body:GetPosition()
    local dx, dy = (x - pos.x), (y - pos.y)
    body:SetLinearVelocity(b2.Vec2(dx, dy))
  end
  return nil
end
local transform_polygon
transform_polygon = function(t, polygon)
  for _index_0 = 1, #polygon do
    local point = polygon[_index_0]
    local x, y
    x, y = point[1], point[2]
    local vec = b2.b2Mul(t, b2.Vec2(x, y))
    point[1], point[2] = vec.x, vec.y
  end
end
return {
  shape_distance = shape_distance,
  shape_set_distance = shape_set_distance,
  ray_cast = ray_cast,
  create_body = create_body,
  body_distance = body_distance,
  set_velocities_to_fixed_set = set_velocities_to_fixed_set,
  set_velocities_to_point = set_velocities_to_point,
  transform_polygon = transform_polygon
}
