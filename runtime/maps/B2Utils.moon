b2 = require 'b2'
_shape_distance = () ->
    input = b2.DistanceInput()
    proxyA = b2.DistanceProxy()
    proxyB = b2.DistanceProxy()
    cache = b2.SimplexCache()
    output = b2.DistanceOutput()

    return (shape1, transform1, shape2, transform2) ->
        proxyA\Set(shape1,1)
        proxyB\Set(shape2,1)
        input.transformA = transform1
        input.transformB = transform2
        input.proxyA = proxyA
        input.proxyB = proxyB
        input.useRadii = true
        cache.count = 0
        b2.b2Distance(output, cache, input)
        return output.pointA, output.pointB, output.distance
shape_distance = _shape_distance()

_ray_cast = () ->
    input = b2.RayCastInput()
    input.maxFraction = 1
    output = b2.RayCastOutput()
    return (fixs, p1, p2) ->
        input.p1 = p1
        input.p2 = p2
        ind = 0
        for fix in *fixs
            if fix\RayCast(output, input, ind)
                return true
            ind += 1
        return false
ray_cast = _ray_cast()

shape_set_distance = (t1, fixs1, t2, fixs2) ->
    mp1,mp2,mdist = nil,nil, math.huge
    mf1,mf2 = nil,nil
    for fix1 in *fixs1
        s1 = fix1\GetShape()
        for fix2 in *fixs2
            s2 = fix2\GetShape()
            p1, p2, dist = shape_distance(s1, t1, s2, t2)
            if dist < mdist
                mp1, mp2, mdist = p1, p2, dist
                mf1, mf2 = fix1, fix2
    return mp1,mp2,mdist,mf1,mf2

-- create_body:
--   Create a body in `world`. If density == 0.0 then the object is static.
create_body = (world, polygons, density = 0.0, x = 0.0, y = 0.0) ->
    body = world\CreateBody with b2.BodyDef()
        .type = b2.dynamicBody if density > 0.0
        .position\Set(x, y)
    fixtures = for shape in *polygons
        body\CreateFixture with b2.PolygonShape()
            \Set(shape),
            density
    return {:body, :fixtures, is_dynamic: density > 0.0}

local body_distance
_body_distance = (o1, others) ->
    mp1,mp2,mdist = nil,nil, math.huge
    for o2 in *others
        if o1 == o2
            continue
        p1,p2,dist = body_distance(o1, o2)
        if dist < mdist
            mp1,mp2,mdist=p1,p2,dist
    return mp1,mp2,mdist

body_distance = (o1, o2) ->
    if o2.body == nil -- Is it possibly an object list?
        return _body_distance(o1, o2)
    t1, fixs1 = o1.body\GetTransform(), o1.fixtures
    t2, fixs2 = o2.body\GetTransform(), o2.fixtures
    return shape_set_distance(t1, fixs1, t2, fixs2)

set_velocities_to_fixed_set = (bodies, fixed_bodies, clump_once_near = false) ->
    for o in *bodies
        pos = o.body\GetPosition()
        p1, p2, dist = body_distance(o, fixed_bodies)
        if not p1 or not p2
            continue
        if clump_once_near and #bodies > 1
            if dist < 100
                p1, p2, dist = body_distance(o, bodies)
        dx, dy = (p2.x - p1.x), (p2.y - p1.y)
        o.body\SetLinearVelocity(b2.Vec2(dx, dy))
    return nil

set_velocities_to_point = (bodies, x, y) ->
    for {:body} in *bodies
        pos = body\GetPosition()
        dx, dy = (x - pos.x), (y - pos.y)
        body\SetLinearVelocity(b2.Vec2(dx, dy))
    return nil

transform_polygon = (t, polygon) ->
    for point in *polygon
        {x, y} = point
        vec = b2.b2Mul(t, b2.Vec2(x, y))
        point[1], point[2] = vec.x, vec.y

return {
    :shape_distance, :shape_set_distance, :ray_cast, :create_body
    :body_distance, :set_velocities_to_fixed_set, :set_velocities_to_point, :transform_polygon
}
