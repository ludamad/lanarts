-- Shape.moon
-- Introduces a Shape type
b2 = require 'b2'
Display = require "core.Display"
GenerateUtils = require "maps.GenerateUtils"
Keys = require "core.Keyboard"
WIDTH, HEIGHT = 640, 480

B2Utils = require "maps.B2Utils"

rng = require("mtwist").create(os.time())
make_polygon = (w, h, points) ->
    return GenerateUtils.skewed_ellipse_points(rng, {0,0}, {w, h}, points)
--shape_distance = () ->
transform_polygon = (t, polygon) ->
    return for {x, y} in *polygon
        vec = b2.b2Mul(t, b2.Vec2(x, y))
        {vec.x, vec.y}

-- A SourceMap renderable shape. Defined using a collection of polygons.
Shape = newtype {
    -- polygons: Expressed as lists of points of a polygon:
    -- xy: Relative to root area center
    init: (polygons, outer_polygon = false) =>
        @polygons = polygons
        @outer_polygon = outer_polygon
        @x = 0
        @y = 0
    -- Update with the body after the simulation, applying the transform
    b2Update: (body, shift_polygons = false) =>
        t = body\GetTransform()
        @apply_transform(t, shift_polygons)
    apply_transform: (t, shift_polygons = false) =>
        {:x, :y} = t.p
        if shift_polygons
            @x, @y = 0, 0
        else
            @x, @y = x, y
            t.p\Set(0,0)
        @polygons = for shape in *@polygons
            transform_polygon(t, shape)
        if @outer_polygon
            @outer_polygon = transform_polygon(t, @outer_polygon)
        t.p\Set(x, y) -- Restore

    b2Body: (world, density = 0.0) => 
        -- TODO 
        body = world\CreateBody with b2.BodyDef()
            .type = b2.dynamicBody if density > 0.0
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

-- Used for shape simulations and transformation utilities
-- Useful class for using simulations to create meaningful shapes
B2ShapeSet = newtype {
    init: (world, shapes, fixed_shapes) =>
        @shapes = assert shapes
        @fixed_shapes = assert fixed_shapes
        @b2shapes = for shape in *shapes
            body, outer_fixture, fixtures = shape\b2Body(world, 1.0)
            {:body, :outer_fixture, :fixtures}
        @b2fixed_shapes = for shape in *fixed_shapes
            body, outer_fixture, fixtures = shape\b2Body(world, 0.0)
            {:body, :outer_fixture, :fixtures}
    update_shapes: (shift_polygons = false) =>
        for i=1,#@shapes
            shape = @shapes[i]
            {:body} = @b2shapes[i]
            shape\b2Update(body, shift_polygons)
    drift_shapes: (scale_x, scale_y) =>
        for shape in *@shapes
            shape.x *= scale_x
            shape.y *= scale_y
    drift_bodies: (scale_x, scale_y) =>
        for {:body} in *@b2shapes
            p = body\GetPosition()
            p.x *= scale_x
            p.y *= scale_y
    to_shape: () =>
        old_shapes = table.deep_clone(@shapes) 
        -- Shift the polygons so everything is defined in terms of the origin
        @update_shapes(true)
        -- Collect all the polygons into our new shape
        polygons = {}
        for shape in *@shapes
            for polygon in *shape.polygons
                append polygons, polygon
        @shapes = old_shapes
        -- New shape situated at 0,0
        return Shape.create(polygons)
    _distance: (o1, others) =>
        mp1,mp2,mdist = nil,nil, math.huge
        for o2 in *others
            if o1 == o2
                continue
            p1,p2,dist = @distance(o1, o2)
            if dist < mdist
                mp1,mp2,mdist=p1,p2,dist
        return mp1,mp2,mdist
    distance: (o1, o2) =>
        if o2.body == nil -- Is it possibly an object list?
            return @_distance(o1, o2)
        t1, fixs1 = o1.body\GetTransform(), o1.fixtures
        t2, fixs2 = o2.body\GetTransform(), o2.fixtures
        return B2Utils.shape_set_distance(t1, fixs1, t2, fixs2)
    unique_within: (o1, o2, p1, p2) =>
        for o in *@b2shapes
            if o == o1 or o == o2
                continue
            if B2Utils.ray_cast(o.fixtures, p1, p2)
                return false
        return true
    set_velocities_to_fixed_set: (clump_once_near = false) =>
        for o in *@b2shapes
            pos = o.body\GetPosition()
            p1, p2, dist = @distance(o, @b2fixed_shapes)
            if clump_once_near and #@b2shapes > 1
                if dist < 100
                    p1, p2, dist = @distance(o, @b2shapes)
            dx, dy = (p2.x - p1.x), (p2.y - p1.y)
            o.body\SetLinearVelocity(b2.Vec2(dx, dy))

    has_overlaps: () =>
        for {:body} in *@b2shapes
            if body\OverlapsOtherBody()
                return true
        return false
    set_velocities_to_point: (x, y) =>
        for {:body} in *@b2shapes
            pos = body\GetPosition()
            dx, dy = (x - pos.x), (y - pos.y)
            body\SetLinearVelocity(b2.Vec2(dx, dy))
    set_velocities_to_zero: () =>
        for {:body} in *@b2shapes
            body\SetLinearVelocity(b2.Vec2(0, 0))
}

VIS_W, VIS_H = 800, 600
visualize_spread_shapes = (args) ->
    -- Arguments
    shapes = assert args.shapes
    fixed_shapes = assert args.fixed_shapes
    -- unused 'n_iterations' 
    -- Do shapes from clumps once they become near in towards_fixed_shapes mode?
    clump_once_near = args.clump_once_near or false
    mode = args.mode or 'towards_fixed_shapes'

    -- Helpers
    local world, b2shapes
    new_world = () ->
        world = b2.World(b2.Vec2(0.0,0.0)) -- No gravity
        b2shapes = B2ShapeSet.create(world, shapes, fixed_shapes)
        world\SetDebugDraw with b2.GLDrawer()
            \SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
    connect = (shape1, shape2) ->

    step_world = () ->
        if mode == 'towards_fixed_shapes'
            b2shapes\set_velocities_to_fixed_set(clump_once_near)
        elseif mode == 'towards_center'
            b2shapes\set_velocities_to_point(0, 0)
        else
            error("Unrecognized mode '#{mode}'!")
        world\Step(0.1, 10, 10)
    Display.initialize("Demo", {VIS_W, VIS_H}, false)
    Display.set_world_region({-VIS_W/2, -VIS_H/2, VIS_W/2, VIS_H/2})
    while true
        new_world()
        require("core.GameState").game_loop () ->
            world\DrawDebugData()
            if Keys.key_pressed "B"
                step_world()
            if Keys.key_pressed "N"
                for i=1,10
                    step_world()
            if Keys.key_pressed "M"
                for i=1,100
                    step_world()
            if Keys.key_pressed "C"
                print 'C'
                for {:body} in *b2shapes.b2shapes
                    --contacts = body\GetContactList()
                    pretty(body, body\OverlapsOtherBody())
                    --while contacts
                    --    print body, contacts
                    --    contacts = contacts\GetNext()
            for k in *{'M', 'R', 'J', 'Q'} 
                if Keys.key_pressed(k)
                    return true
        if Keys.key_pressed "M"
            b2shapes\update_shapes()
        elseif Keys.key_pressed "J"
            b2shapes\update_shapes()
            b2shapes\drift_shapes(1.1,1.1)
        else
            break
    return b2shapes\to_shape()

visualize_world = (world) -> (title) ->
    require("core.GameState").game_loop () ->
        font = font_cached_load "fonts/Gudea-Regular.ttf", 14
        world\DrawDebugData()
        if Keys.key_pressed "N"
            return true
        -- Hack for dealing with box2d clobbering:
        Display.reset_blend_func()
        font\draw({color: COL_WHITE, origin: Display.CENTER}, {100, 100}, title)
        return false
spread_shapes = (args) ->
    -- Arguments
    shapes = assert args.shapes
    fixed_shapes = assert args.fixed_shapes
    n_iterations = args.n_iterations or 100 -- Default 100 iterations
    scale = args.scale or {1,1}
    -- Attempt to jiggle until there is no overlap
    jiggle_iterations = args.jiggle_iterations or 0
    -- Do shapes from clumps once they become near?
    clump_once_near = args.clump_once_near or false
    mode = args.mode or 'towards_fixed_shapes'
    -- State
    world = b2.World(b2.Vec2(0.0,0.0)) -- No gravity
    visualize = if args.visualize then visualize_world(world) else do_nothing
    b2shapes = B2ShapeSet.create(world, shapes, fixed_shapes)
    if args.visualize
        world\SetDebugDraw with b2.GLDrawer()
            \SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
    visualize("Starting point")
    -- Iteration loop
    for i=1,n_iterations
        if mode == 'towards_fixed_shapes'
            b2shapes\set_velocities_to_fixed_set(clump_once_near)
        elseif mode == 'towards_center'
            b2shapes\set_velocities_to_point(0, 0)
        else
            error("Unrecognized mode '#{mode}'!")
        world\Step(0.1, 1000, 1000)
        visualize("Iteration #{i}")
    for i=1,jiggle_iterations
        if not b2shapes\has_overlaps()
            break
        b2shapes\drift_bodies(1.1, 1.1)
        if mode == 'towards_fixed_shapes'
            b2shapes\set_velocities_to_fixed_set(clump_once_near)
        elseif mode == 'towards_center'
            b2shapes\set_velocities_to_point(0, 0)
        else
            error("Unrecognized mode '#{mode}'!")
        world\Step(0.1, 1000, 1000)
        visualize("Jiggle #{i}")
    b2shapes\update_shapes()
    b2shapes\drift_shapes(scale[1], scale[2])
    -- Return a compound shape
    return b2shapes\to_shape()

-- return {:Shape, :spread_shapes}


Display.initialize("Demo", {VIS_W, VIS_H}, false)
Display.set_world_region({-VIS_W/2, -VIS_H/2, VIS_W/2, VIS_H/2})
while true
    make_shapes = (n) ->
        polygons = for i=4,3 + n do make_polygon(i * 10, i * 10, i * math.random() + 3)
        return for poly in *polygons 
            shape = Shape.create {poly}
            shape.x, shape.y = rng\randomf(-100, 100), rng\randomf(-100, 100)
            shape

    shapes = {}
    for i=1,1
        compound_shape = spread_shapes {
            shapes: make_shapes(4)
            visualize: true
            fixed_shapes: {Shape.create({make_polygon(8, 8)})}
            mode: 'towards_fixed_shapes'
            clump_once_near: true
            --scale: {1.1, 1.1}
        }
        compound_shape.x, compound_shape.y = rng\randomf(-200, 200), rng\randomf(-200, 200)
        append shapes, compound_shape
    spread_shapes {
        shapes: {shape}
        visualize: true
        fixed_shapes: {Shape.create({make_polygon(8, 8)})}
        mode: 'towards_center'
    }
    if not Keys.key_pressed "R"
        break
