-- Shape.moon
-- Introduces a Shape type
b2 = require 'b2'
Display = require "core.Display"
GenerateUtils = require "maps.GenerateUtils"
WIDTH, HEIGHT = 640, 480

B2Utils = require "maps.B2Utils"

--shape_distance = () ->

transform_polygon = (t, shape) ->
    return for {x, y} in shape
        vec = b2.b2Mul(t, b2.Vec2(x, y))
        {vec.x, vec.y}

-- A renderable shape. Defined using a collection of polygons.
Shape = newtype {
    -- polygons: Expressed as lists of points of a polygon:
    -- xy: Relative to root area center
    init: (polygons, outer_polygon = false) =>
        @polygons = polygons
        @outer_polygon = outer_polygon
        @x = 0
        @y = 0

    -- Update with the body after the simulation, applying the transform
    b2Update: (body) =>
        t = body\getTransform()
        {:x, :y} = t.p
        @x, @y = x, y
        t.p\Set(0,0)
        @polygons = for shape in *@polygons
            transform_polygon(t, shape)
        @outer_polygon = transform_polygon(t, @outer_polygon)

    -- For B2Shape:
    b2Body: (world, density = 0.0) => 
        -- TODO 
        body = world\CreateBody with b2.BodyDef()
            .type = b2.dynamicBody
            .position\Set(@x, @y)
        fixtures = for shape in *@polygons
            body\CreateFixture with b2.PolygonShape()
                \Set(shape),
                density
        outer_fixture = if @outer_polygon
            body\CreateFixture with b2.PolygonShape()
                \Set(outer_polygon),
                density
        else nil
        return body, outer_fixture, fixtures
}

B2ShapeSet = newtype {
    init: (world, shapes) =>
        @shapes = shapes
        @b2shapes = for shape in *shapes
            body, outer_fixture, fixture = shape\b2Body(world, 1.0)
            {:body, :outer_fixture, :fixture}
    set_velocities: () =>
        for {:body} in *@b2shapes
            pos = body\GetPosition()
            -- -- Check bounds:
            -- if pos.x < 1 or pos.x > WIDTH - 1 or pos.y < 1 or pos.y > HEIGHT - 1
            --     -- If out of bounds, push into center
            --     pos = b2.Vec2(math.random() * WIDTH, math.random() * HEIGHT)
            --     body\SetTransform(pos, body\GetAngle())
            dx, dy = (WIDTH/2 - pos.x), (HEIGHT/2 - pos.y) 
            body\SetLinearVelocity(b2.Vec2(dx, dy))
}

VIS_W, VIS_H = 800, 600
spread_shapes = (shapes, visualize = true) ->
    world = b2.World(b2.Vec2(0.0,0.0)) -- No gravity
    b2shapes = B2ShapeSet.create(world, shapes)
    --b2shapes\set_velocities()
    --world\Step(1, 10, 10)
    if visualize
        drawer = b2.GLDrawer()
        world\SetDebugDraw(drawer)
        Display.initialize("Demo", {VIS_W, VIS_H}, false)
        Display.set_drawing_region({-VIS_W/2, -VIS_H/2, VIS_W/2, VIS_H/2})
        drawer\SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
        require("core.GameState").game_loop () ->
            world\DrawDebugData()

-- return {:Shape, :spread_shapes}

make_polygon = (w, h, points) ->
    return GenerateUtils.skewed_ellipse_points(-w/2, -h/2, w, h, points)

polygons = for i=8,10 do make_polygon(i * 10, i * 10, i)
shapes = for poly in *polygons do Shape.create {poly}
spread_shapes(shapes)
