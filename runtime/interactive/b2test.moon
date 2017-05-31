b2 = require 'b2'
Display = require "core.Display"
GenerateUtils = require "maps.GenerateUtils"
WIDTH, HEIGHT = 640, 480

math.randomseed(os.time())

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
        table.insert(points, {x1 - 10, y1 - 10})
        table.insert(points, {x2 + 10, y1 - 10})
        table.insert(points, {x2 + 10, y2 + 10})
        table.insert(points, {x1 - 10, y2 + 10})
        make_polygon world, bd, points

make_dynamic_shape = (world, size, xy) ->
    -- Define the dynamic body. We set its position and call the body factory.
    {x, y} = xy
    points = GenerateUtils.skewed_ellipse_points(0,0,size[1],size[2], math.floor(math.random()*8 + 4))
    body = make_polygon world,
        with b2.BodyDef()
            .type = b2.dynamicBody
            .position\Set(x, y),
        points,
        1.0
    return body

create_polygons = (world) ->
    -- Return list:
    return for i=1,60
        size = {math.random() * WIDTH / 5  + 15, math.random() *HEIGHT / 5 + 15}
        position = {math.random() * WIDTH, math.random() * HEIGHT}
        make_dynamic_shape(world, size, position)

TIME_STEP = 1.0 / 60
ITERS = 6
make_simulation = (outer_area) ->
    -- Create drawing engine
    drawer = b2.GLDrawer()
    world = b2.World(b2.Vec2(0.0,0.0)) -- No gravity
    world\SetDebugDraw(drawer)
    drawer\SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
    -- Populate simulation
    enclosure_parts = make_enclosure(world, outer_area)
    polygons = create_polygons(world)
    return {
        step: () =>
            for body in *polygons
                pos = body\GetPosition()
                if pos.x < 10 or pos.x > WIDTH - 10 or pos.y < 10 or pos.y > HEIGHT - 10
                    position = {math.random() * WIDTH, math.random() * HEIGHT}
                    body\SetTransform(b2.Vec2(position[1], position[2]), body\GetAngle())
                dx, dy = WIDTH/2 - pos.x, HEIGHT/2 - pos.y
                --angle = math.atan2(dy, dx)
                --tx, ty = math.cos(angle) * 100 + WIDTH/2, math.sin(angle) * 100 + HEIGHT/2
                --dx, dy = tx - pos.x, ty - pos.y
                mag = math.sqrt(dx*dx + dy*dy) 
                if mag > 20
                    dx = dx / mag * 16 --* body\GetMass()
                    dy = dy / mag * 16 --* body\GetMass()
                    -- TODO test for lingering objects to teleport them
                    body\SetLinearVelocity(b2.Vec2(dx, dy))
            world\Step(TIME_STEP, ITERS, ITERS)
            world\DrawDebugData()
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

    Display.draw_loop () -> sim\step()

main()
