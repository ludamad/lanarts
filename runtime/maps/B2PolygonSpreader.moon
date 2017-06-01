b2 = require 'b2'
Display = require "core.Display"
GenerateUtils = require "maps.GenerateUtils"
WIDTH, HEIGHT = 640, 480

B2Utils = require "maps.B2Utils"

Area = newtype {
    -- shapes: Expressed as lists of points of a polygon:
    -- xy: Relative to root area center
    init: (@parent_area, @shapes, @outer_shape = false) =>
        @subareas = {}
        @x = 0
        @y = 0
    b2Body: (world, density = 0.0) => 
        -- TODO 
        body = world\CreateBody with b2.BodyDef()
            .type = b2.dynamicBody
            .position\Set(@x, @y)
        fixtures = for shape in *@shapes
            body\CreateFixture with b2.PolygonShape()
                \Set(shape),
                density
        outer_shape = if @outer_shape
            body\CreateFixture with b2.PolygonShape()
                \Set(outer_shape),
                density
        else nil
        return body, outer_fixture, fixtures
}


B2AreaSpreader = newtype {
    init: (@subareas) =>
        @area_to_polygons = {}
        @world = b2.World(b2.Vec2(0.0,0.0)) -- No gravity
        -- Initialize the body+fixtures / area association
        @area_body = {}
        @_make_bodies()

    _make_bodies: () =>
        for area in *@subareas
            body, outer_fixture, fixtures = area\b2Body(@world, 1.0)
            @area_body[area] = {body, outer_fixture, fixtures}

    visualize: () =>
        math.randomseed(os.time())
        Display.initialize("Demo", {WIDTH, HEIGHT}, false)
        Display.set_drawing_region({-
        Display.draw_loop () -> 
            @step(world)
            @draw(world)
    step: (world) =>
        for i=1,60
            for body in *polygons
                pos = body\GetPosition()
                if pos.x < 1 or pos.x > WIDTH - 1 or pos.y < 1 or pos.y > HEIGHT - 1
                    position = {math.random() * WIDTH, math.random() * HEIGHT}
                    body\SetTransform(b2.Vec2(position[1], position[2]), body\GetAngle())
                dx, dy = WIDTH/2 - pos.x, HEIGHT/2 - pos.y
                mag = math.sqrt(dx*dx + dy*dy) 
                dx = dx / mag --* body\GetMass()
                dy = dy / mag --* body\GetMass()
                -- Move towards centre
                body\SetLinearVelocity(b2.Vec2(dx, dy))
            world\Step(TIME_STEP, ITERS, ITERS)
    draw: (world) =>
        font = font_cached_load("fonts/Gudea-Regular.ttf", 14))
        world\DrawDebugData()
        for body in *polygons
            pos = body\GetPosition()
            min_body, min_d = min_dist(body)
            color = COL_WHITE
            text = "#{min_d}"
            Display.reset_opengl_state()
            font\draw({color: color, origin: Display.CENTER}, {pos.x, pos.y}, text)
}

-- Centered around a 0x0 point:
make_packed_polygons = () ->
    areas = for i=1,10
        points = GenerateUtils.skewed_ellipse_points(-size[1]/2,-size[2]/2,size[1],size[2], math.floor(math.random()*8 + 4))
        Area.create(false, {points})
    


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
ITERS = 1
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
            _, _, dist = B2Utils.body_distance(b, body)
            if dist < min_d
                min_d = dist
                min_body = b
        return min_body, min_d
    return {
        step: () =>
            for i=1,60
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
                    dx = dx / mag --* body\GetMass()
                    dy = dy / mag --* body\GetMass()
                    -- TODO test for lingering objects to teleport them
                    body\SetLinearVelocity(b2.Vec2(dx, dy))
                world\Step(TIME_STEP, ITERS, ITERS)
            world\DrawDebugData()
            for body in *polygons
                pos = body\GetPosition()
                min_body, min_d = min_dist(body)
                color = COL_WHITE
                text = "#{min_d}"
                Display.reset_opengl_state()
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

                -- Hack for dealing with box2d clobbering:
                Display.set_drawing_region({0, 0, WIDTH, HEIGHT})

main()
