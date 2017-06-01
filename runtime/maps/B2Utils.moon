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

body_distance = () ->
    return (body1, body2) ->
        mp1,mp2,mdist = nil,nil, math.huge
        b1F,b2F = body1\GetFixtureList(), body2\GetFixtureList()
        t1,t2 = body1\GetTransform(), body2\GetTransform()
        while b1F ~= nil
            s1 = b1F\GetShape()
            b2FIter = b2F
            while b2FIter ~= nil
                s2 = b2FIter\GetShape()
                p1, p1, dist = shape_distance(s1, t1, s2, t2)
                if dist < mdist
                    mp1 = p1
                    mp2 = p2
                    mdist = dist
                b2F = b2FIter\GetNext()
            b1F = b1F\GetNext()
        return mp1,mp2,mdist

return {
    :body_distance, :shape_distance
}

