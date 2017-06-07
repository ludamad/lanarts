b2 = require 'b2'
Display = require "core.Display"
GenerateUtils = require "maps.GenerateUtils"
WIDTH, HEIGHT = 640, 480

math.randomseed(os.time())

_body_distance = () ->
    input = b2.DistanceInput()
    proxyA = b2.DistanceProxy()
    proxyB = b2.DistanceProxy()
    cache = b2.SimplexCache()
    output = b2.DistanceOutput()

    return (body1, body2) ->
        proxyA\Set(body1\GetFixtureList()\GetShape(),1)
        proxyB\Set(body2\GetFixtureList()\GetShape(),1)
        input.transformA = body1\GetTransform()
        input.transformB = body2\GetTransform()
        input.proxyA = proxyA
        input.proxyB = proxyB
        input.useRadii = true
        cache.count = 0
        b2.b2Distance(output, cache, input)
        return output.pointA, output.pointB, output.distance

body_distance = _body_distance()

make_polygon = (world, bd, points, density = 0.0) ->
    shape = b2.PolygonShape()
    shape\Set(points)
    body = world\CreateBody(bd)
    body\CreateFixture(shape, density)
    return body

make_enclosure = (world, outer_area) ->
    bd = b2.BodyDef()
    bd.position\Set(0, 0)
    -- Return for loop (makes a list)
    return for i=1,#outer_area 
        x1, y1 = unpack(outer_area[i])
        x2, y2 = unpack(outer_area[(i % #outer_area) + 1])
        points = {}
        table.insert(points, {x1 - 1, y1 - 1})
        table.insert(points, {x2 + 1, y1 - 1})
        table.insert(points, {x2 + 1, y2 + 1})
        table.insert(points, {x1 - 1, y2 + 1})
        with world\CreateBody(bd)
            \CreateFixture with b2.PolygonShape()
                \Set(points),
                1.0

make_dynamic_shape = (world, size, xy) ->
    -- Define the dynamic body. We set its position and call the body factory.
    {x, y} = xy
    points = GenerateUtils.skewed_ellipse_points(-size[1]/2,-size[2]/2,size[1],size[2], math.floor(math.random()*8 + 4))
    body = make_polygon world,
        with b2.BodyDef()
            .type = b2.dynamicBody
            .position\Set(x, y),
        points,
        1.0
    --for i=1,10
    --    points2 = GenerateUtils.skewed_ellipse_points(i*-size[1]/2,i*-size[2]/2,size[1],size[2], math.floor(math.random()*8 + 4))
    --    with body
    --        \CreateFixture with b2.PolygonShape()
    --            \Set(points2),
    --            1.0
    return body

create_polygons = (world) ->
    -- Return list:
    return for i=1,10
        size = {math.random() * WIDTH / 5  + 15, math.random() *HEIGHT / 5 + 15}
        if i > 5
            size[1] *= 2
            size[2] *= 2
        position = {math.random() * WIDTH, math.random() * HEIGHT}
        make_dynamic_shape(world, size, position)

TIME_STEP = 1.0 / 60
ITERS = 10
make_simulation = (outer_area) ->
    -- Create drawing engine
    drawer = b2.GLDrawer()
    world = b2.World(b2.Vec2(0.0,0.0)) -- No gravity
    world\SetDebugDraw(drawer)
    drawer\SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
    -- Populate simulation
    enclosure_parts = make_enclosure(world, outer_area)
    polygons = create_polygons(world)
    min_dist = (body) ->
        min_body = nil
        min_d = math.huge 
        for b in *polygons
            if b == body
                continue
            _, _, dist = body_distance(b, body)
            if dist < min_d
                min_d = dist
                min_body = b
        return min_body, min_d
    return {
        step: () =>
            t = timer_create()
            for i=1,1--0--60
                for body in *polygons
                    pos = body\GetPosition()
                    if pos.x < 1 or pos.x > WIDTH - 1 or pos.y < 1 or pos.y > HEIGHT - 1
                        position = {math.random() * WIDTH, math.random() * HEIGHT}
                        body\SetTransform(b2.Vec2(position[1], position[2]), body\GetAngle())
                    dx, dy = WIDTH/2 - pos.x, HEIGHT/2 - pos.y
                    --angle = math.atan2(dy, dx)
                    --tx, ty = math.cos(angle) * 10 + WIDTH/2, math.sin(angle) * 10 + HEIGHT/2
                    --dx, dy = tx - pos.x, ty - pos.y
                    mag = math.sqrt(dx*dx + dy*dy) 
                    --if mag > 20
                    dx = dx / mag * 1024 --* body\GetMass()
                    dy = dy / mag * 1024 --* body\GetMass()
                    -- TODO test for lingering objects to teleport them
                    body\SetLinearVelocity(b2.Vec2(dx, dy))
                world\Step(TIME_STEP, ITERS, ITERS)
            print(t\get_microseconds() / 1000)
            world\DrawDebugData()
            for body in *polygons
                pos = body\GetPosition()
                min_body, min_d = min_dist(body)
                color = COL_WHITE
                text = "#{min_d}"
                -- Hack for dealing with box2d clobbering:
                Display.set_drawing_region({0, 0, WIDTH, HEIGHT})
                FONT\draw({color: color, origin: Display.CENTER}, {pos.x, pos.y}, text)
    }

main = () ->
    enclosure = {
        {0,0},
        {WIDTH, 0},
        {WIDTH, HEIGHT},
        {0, HEIGHT}
    }
    sim = make_simulation(enclosure)
    Display.initialize("Demo", {WIDTH, HEIGHT}, false)
    rawset(_G, "FONT", Display.font_load("fonts/Gudea-Regular.ttf", 14))
    Display.draw_loop () -> sim\step()

main()
