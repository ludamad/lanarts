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
        for fix in *fixs
            if fix\RayCast(input, output)
                return true
ray_cast = _ray_cast()

shape_set_distance = (t1, fixs1, t2, fixs2) ->
    mp1,mp2,mdist = nil,nil, math.huge
    for fix1 in *fixs1
        s1 = fix1\GetShape()
        for fix2 in *fixs2
            s2 = fix2\GetShape()
            p1, p2, dist = shape_distance(s1, t1, s2, t2)
            if dist < mdist
                mp1, mp2, mdist = p1, p2, dist
    return mp1,mp2,mdist

return {
    :shape_distance, :shape_set_distance, :ray_cast
}

