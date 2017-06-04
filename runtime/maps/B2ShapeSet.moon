-- Shape.moon
-- Introduces a Shape type
b2 = require 'b2'
GenerateUtils = require "maps.GenerateUtils"
Keys = require "core.Keyboard"
WIDTH, HEIGHT = 640, 480
B2Utils = require "maps.B2Utils"

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
    translate: (x, y) =>
        for shape in *@polygons
            for point in *shape
                point[1] += x
                point[2] += y
        return nil
    bbox: () =>
        x1,y1 = math.huge, math.huge
        x2,y2 = -math.huge, -math.huge
        for shape in *@polygons
            for {x, y} in *shape
                x1, y1 = math.min(x1, x), math.min(y1, y)
                x2, y2 = math.max(x2, x), math.max(y2, y)
        return {x1, y1, x2, y2}
    b2Body: (world, density = 0.0) => 
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
    return B2Utils.shape_set_distance(t1, fixs1, t2, fixs2)

scale_polygon = (points, scale_x, scale_y) ->
    cx,cy = 0,0
    for {x, y} in *points
        cx, cy = cx + x, cy + y
    cx,cy = cx/#points, cy/#points
    return for {x, y} in *points
        x = (x - cx)*scale_x + cx
        y = (y - cy)*scale_y + cy
        {x, y}

-- Used for shape simulations and transformation utilities
-- Useful class for using simulations to create meaningful shapes
B2ShapeSet = newtype {
    init: (world, shapes, fixed_shapes) =>
        @shapes = assert shapes
        @fixed_shapes = assert fixed_shapes
        @b2shapes = for shape in *shapes
            body, outer_fixture, fixtures = shape\b2Body(world, 1.0)
            {:body, :outer_fixture, :fixtures, :shape}
        @b2fixed_shapes = for shape in *fixed_shapes
            body, outer_fixture, fixtures = shape\b2Body(world, 0.0)
            {:body, :outer_fixture, :fixtures, :shape}
    drift_shapes: (scale_x, scale_y) =>
        for shape in *@shapes
            shape.x *= scale_x
            shape.y *= scale_y
        return nil
    drift_bodies: (scale_x, scale_y) =>
        for {:body} in *@b2shapes
            p = body\GetPosition()
            p.x *= scale_x
            p.y *= scale_y
        return nil
    update_shapes: () =>
        for {:body, :shape} in *@b2shapes
            -- Offset to the center:
            shape.x = body\GetPosition().x
            shape.y = body\GetPosition().y
            shape.polygons = for polygon in *shape.polygons 
                for {px, py} in *polygon
                    {:x, :y} = body\GetWorldPoint(b2.Vec2(px, py))
                    {x - shape.x, y - shape.y}
        return nil
    to_shape: (tunnels = {}) =>
        -- Collect all the polygons into our new shape
        polygons = {}
        for shape_sets in *{@b2shapes, tunnels}
            for {:body, :shape} in *shape_sets
                -- Offset to the center:
                for polygon in *shape.polygons
                    append polygons, for {px, py} in *polygon
                        {:x, :y} = body\GetWorldPoint(b2.Vec2(px, py))
                        {x, y}
        -- New shape situated at 0,0
        return Shape.create(polygons)
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
            p1, p2, dist = body_distance(o, @b2fixed_shapes)
            if clump_once_near and #@b2shapes > 1
                if dist < 100
                    p1, p2, dist = body_distance(o, @b2shapes)
            dx, dy = (p2.x - p1.x), (p2.y - p1.y)
            o.body\SetLinearVelocity(b2.Vec2(dx, dy))
        return nil
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
        return nil
    set_velocities_to_zero: () =>
        for {:body} in *@b2shapes
            body\SetLinearVelocity(b2.Vec2(0, 0))
        return nil
}

-- Performs a connectivity graph algorithm on a B2ShapeSet
B2ShapeConnector = newtype {
    init: (world, b2shape_set) =>
        -- The simulation world:
        @world = world
        -- All bodies you do not want tunnels to overlap 
        @all_bodies = {}
        -- Bodies you want to connect, and their connection lists
        @bodies = {}
        @tunnels = {}
        -- We start with one connection set per body
        -- We merge connection sets as they gain contacts between them
        -- Our goal is one final connection set
        @connection_sets = {}
        @_init(b2shape_set.b2shapes, b2shape_set.b2fixed_shapes)
    _init: (b2shapes, b2fixed_shapes) =>
        for {:body, :shape, :fixtures} in *b2shapes
            -- Have initially empty connection list for each
            o = {:body, :shape, :fixtures, connections: {}}
            append @all_bodies, o
            append @connection_sets, {o} -- Each their own partition at first
            append @bodies, o
        for o in *b2fixed_shapes
            append @all_bodies, o
        return nil

    _get_set_index_of: (body) =>
        for i=1,#@connection_sets
            if table.contains @connection_sets[i], body
                return i
        error("Body should be contained in a connection set!")

    _record_connection: (body_a, body_b) =>
        -- Append to connection info
        append body_a.connections, body_b
        append body_b.connections, body_a

        -- Merge connection sets
        -- TODO can optimize slightly
        ind_a = @_get_set_index_of(body_a)
        ind_b = @_get_set_index_of(body_b)
        -- If not the same, merge:
        if ind_a ~= ind_b
            C = @connection_sets
            -- Copy all of ind_b into ind_a
            for body in *C[ind_b]
                append C[ind_a], body
            -- Extract last
            last = C[#C]
            -- Overwrite ind_b with last
            C[ind_b] = last
            -- Remove last
            C[#C] = nil
            --print("There are now #{#C} connection sets.")
    _try_random_connection: (rng) =>
        -- Constants
        TUNNEL_WIDTH = 2
        DIST_THRESHOLD = 0.0
        -- 
        if #@bodies < 2
            error("Cannot connect a set of just one body!")
        index_a = rng\random(1,#@bodies)
        index_b = rng\random(index_a + 1,#@bodies+1)
        --print(index_a, index_b)
        body_a, body_b = @bodies[index_a], @bodies[index_b]
        if table.contains(body_a.connections, body_b)
            return false

        -- Find closest points:
        p1, p2, dist = body_distance(body_a, body_b)
        x1, y1 = p1.x, p1.y
        x2, y2 = p2.x, p2.y
        local dx, dy 
        if dist <= DIST_THRESHOLD
            -- If the points coincide or are close, 
            -- use the center point of body_a for deciding 
            -- penetration direction
            -- TODO use fixture data for this, too
            bp1 = body_a.body\GetPosition()
            --bp2 = body_b.body\GetPosition()
            dx, dy = p2.x - bp1.x, p2.y - bp1.y
        else
            dx, dy = p2.x - p1.x, p2.y - p1.y
        -- Create unit vector:
        mag = math.sqrt(dx*dx+dy*dy)
        dx, dy = dx / mag * TUNNEL_WIDTH, dy / mag * TUNNEL_WIDTH
        -- Back into objects 2 TUNNEL_WIDTH's
        x1 -= dx * TUNNEL_WIDTH * 2
        y1 -= dy * TUNNEL_WIDTH * 2
        x2 += dx * TUNNEL_WIDTH * 2
        y2 += dy * TUNNEL_WIDTH * 2
        -- Check if a tunnel created here is OK using raycasting
        for b in *@all_bodies
            -- Its OK for our bodies to be touched
            -- in the ray cast
            if b == body_a or b == body_b
                continue
            if B2Utils.ray_cast(b.fixtures, p1, p2)
                return false
        -- Create tunnel
        -- TODO allow width customization
        shape = {
            {x1 - dy, y1 + dx}
            {x1 + dy, y1 - dx}
            {x2 + dy, y2 - dx}
            {x2 - dy, y2 + dx}
        }
        tunnel_body = @world\CreateBody with b2.BodyDef()
            .position\Set(0, 0)
        tunnel_body\CreateFixture with b2.PolygonShape()
            \Set(shape),
            0.0
        append @tunnels, {body: tunnel_body, shape: Shape.create({shape})}
        @_record_connection(body_a, body_b)
        return true

    -- Continuously try until a random connection is made
    do_random_connection: (rng) =>
        MAX_TRIALS = 100
        for trial=1,MAX_TRIALS
            if @_try_random_connection(rng)
                return true
        return false

    do_ensure_connected: (rng) =>
        return nil
}

VIS_W, VIS_H = 800, 600
_FIRST_VISUALIZE = true
visualize_world = (world) -> (title) ->
    if _FIRST_VISUALIZE
        require("core.Display").initialize("Demo", {VIS_W, VIS_H}, false)
        require("core.Display").set_world_region({-VIS_W/2, -VIS_H/2, VIS_W/2, VIS_H/2})
        _FIRST_VISUALIZE = false
    require("core.GameState").game_loop () ->
        font = font_cached_load "fonts/Gudea-Regular.ttf", 14
        world\DrawDebugData()
        if Keys.key_pressed "N"
            return true
        -- Hack for dealing with box2d clobbering:
        require("core.Display").reset_blend_func()
        font\draw({color: COL_WHITE, origin: require("core.Display").CENTER}, {100, 100}, title)
        return false

spread_shapes = (args) ->
    -- Arguments
    shapes = assert args.shapes
    fixed_shapes = assert args.fixed_shapes
    n_iterations = args.n_iterations or 100 -- Default 100 iterations
    n_subiterations = args.n_subiterations or 10 -- Default 100 iterations
    scale = args.scale or {1,1}
    -- Do shapes from clumps once they become near?
    clump_once_near = args.clump_once_near or false
    return_compound_shape = args.return_compound_shape or false
    mode = args.mode or 'towards_fixed_shapes'
    connect_after = args.connect_after or false
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
        world\Step(0.1, n_subiterations, n_subiterations)
        if i % 10 == 0
            visualize("Iteration #{i}")
    -- Perform connection pass
    tunnels = {}
    if connect_after
        rng = assert args.rng, "Need 'rng' object for connect_after=True!"
        connector = B2ShapeConnector.create(world, b2shapes)
        for i=1,100
            if not connector\do_random_connection(rng)
                break
            visualize("Random connection #{i}")
        tunnels = connector.tunnels
    -- Return a compound shape
    compound_shape = if return_compound_shape then b2shapes\to_shape(tunnels) else nil
    b2shapes\update_shapes()
    return compound_shape

-- return {:Shape, :spread_shapes}

sample_shape = () ->
    make_shapes = (n) ->
        polygons = for i=4,3 + n do make_polygon(i * 10, i * 10, i * math.random() + 3)
        return for poly in *polygons 
            shape = Shape.create {poly}
            shape.x, shape.y = rng\randomf(-100, 100), rng\randomf(-100, 100)
            shape
    return spread_shapes {
        :rng
        shapes: make_shapes(4)
        fixed_shapes: {Shape.create({make_polygon(8, 8)})}
        n_iterations: 50
        --visualize: true
        mode: 'towards_fixed_shapes'
        clump_once_near: true
        connect_after: true
        return_compound_shape: true
        --scale: {1.1, 1.1}
    }

__visualize = () ->
    SourceMap = require "core.SourceMap"
    shape = sample_shape()
    bbox = shape\bbox()
    w, h = bbox[3] - bbox[1], bbox[4] - bbox[2]
    -- Make the top y=0 and left x=0
    shape\translate(-bbox[1], -bbox[2])
    assert w > 0 and h > 0, "w, h should be > 0, #{w}, #{h}"
    tw, th = 8,8
    require("core.Display").initialize("Demo", {800, 600} , false)
    require("core.Display").set_world_region({0, 0, w * tw, h * th})
    map = SourceMap.map_create {
        :rng
        label: "Test"
        size: {w, h}
        flags: SourceMap.FLAG_SOLID
    }

    timer = timer_create()
    for i=1,#shape.polygons
        polygon = shape.polygons[i]
        SourceMap.polygon_apply {
            :map
            area: {2,2, w-2, h-2}
            operator: {
                remove: SourceMap.FLAG_SOLID
                add: SourceMap.FLAG_SEETHROUGH
                content: 1
            }
            points: scale_polygon(polygon, 1.1, 1.1)
        }
    print "Time spent rasterizing polygons: ", timer\get_milliseconds()

    require("core.GameState").game_loop () ->
        row = {}
        for y=1,h
            SourceMap.get_row_content(map, row, 1, w, y)
            for x=1,w
                color = COLORS[(row[x] % #COLORS) + 1]
                if color == COL_BLACK and row[x] ~= 0
                    color = COL_GOLD
                require("core.Display").draw_rectangle(color, {(x-1)*tw, (y-1)*th, x*tw, y*th})
        if Keys.key_pressed 'R'
            return true
        return nil

return {:sample_shape}
