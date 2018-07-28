-- Helpers for using Box2D to make levels with

b2 = require 'b2'
GenerateUtils = require "maps.GenerateUtils"
Keys = require "core.Keyboard"
DebugUtils = require "maps.DebugUtils"
B2Utils = require "maps.B2Utils"

-- Used for shape simulations and transformation utilities
-- Useful class for using simulations to create meaningful shapes
B2World = newtype {
    init: () =>
        @world = b2.World(b2.Vec2(0.0,0.0)) -- No gravity
        @dynamic_bodies = {}
        @static_bodies = {}
        @all_bodies = {}
    add_body: (shape, density = 1.0) =>
        -- Density 0.0 implies a static box2d object
        body = B2Utils.create_body @world, shape, density
        if density == 0.0
            append @static_bodies, body
        else
            append @dynamic_bodies, body
        append @all_bodies, body
        return body
    unique_within: (o1, o2, p1, p2) =>
        for o in *@all_bodies
            if o == o1 or o == o2
                continue
            if B2Utils.ray_cast(o.fixtures, p1, p2)
                return false
        return true
    has_overlaps: () =>
        for {:body} in *@all_bodies
            if body\OverlapsOtherBody()
                return true
        return false
    visualize: (title) =>
        if not DebugUtils.is_debug_visualization()
            return
        @world\SetDebugDraw with b2.GLDrawer()
            \SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
        w,h = DebugUtils.visualization_size()
        require("core.Display").set_world_region {-w/2, -h/2, w/2, h/2}
        require("core.GameState").game_loop () ->
            font = font_cached_load "fonts/Gudea-Regular.ttf", 14
            @world\DrawDebugData()
            if Keys.key_pressed "N"
                return true
            -- Hack for dealing with box2d clobbering:
            require("core.Display").reset_blend_func()
            font\draw({color: COL_WHITE, origin: require("core.Display").CENTER}, {100, 100}, title)
            return false
}

-- Performs a connectivity graph algorithm on a B2World
B2ShapeConnector = newtype {
    init: (b2world) =>
        -- The simulation world:
        @world = b2world.world
        -- All bodies you do not want tunnels to overlap
        @all_bodies = {}
        -- Bodies you want to connect, and their connection lists
        @bodies = {}
        @tunnels = {}
        -- We start with one connection set per body
        -- We merge connection sets as they gain contacts between them
        -- Our goal is one final connection set
        @connection_sets = {}
        @_init(b2world.dynamic_bodies, b2world.static_bodies)
    _init: (dynamic_bodies, static_bodies) =>
        for {:body, :fixtures} in *dynamic_bodies
            -- Have initially empty connection list for each
            o = {:body, :fixtures, connections: {}}
            append @all_bodies, o
            append @connection_sets, {o} -- Each their own partition at first
            append @bodies, o
        for o in *static_bodies
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
        p1, p2, dist = B2Utils.body_distance(body_a, body_b)
        if not p1 or not p2
            return false

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
        append @tunnels, {:index_a, :index_b, polygon: shape}
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

-- Low-level interface
-- Returns a list of transforms for each dynamic body in b2world
-- NOTE: These transforms should be immediately used
_spread_shapes = (args) ->
    -- Arguments
    b2world = assert args.b2world
    n_iterations = args.n_iterations or 100 -- Default 100 iterations
    n_subiterations = args.n_subiterations or 10 -- Default 100 iterations
    -- Do shapes form clumps once they become near?
    clump_once_near = args.clump_once_near or false
    mode = args.mode or 'towards_fixed_shapes'

    b2world\visualize("Starting point")
    -- Iteration loop
    for i=1,n_iterations
        if mode == 'towards_fixed_shapes'
            B2Utils.set_velocities_to_fixed_set(b2world.dynamic_bodies, b2world.static_bodies, clump_once_near)
        elseif mode == 'towards_center'
            B2Utils.set_velocities_to_point(b2world.dynamic_bodies, 0, 0)
        else
            error("Unrecognized mode '#{mode}'!")
        b2world.world\Step(0.1, n_subiterations, n_subiterations)
        if i % 10 == 0
            b2world\visualize("Iteration #{i}")
    for i, {:body} in ipairs b2world.dynamic_bodies
        args.sync(i, body)

-- Standardizes the conversion from map regions and fixed polygons to a B2World
_b2_world_from_args = (args) ->
    b2world = B2World.create()
    for region in *args.regions
        b2world\add_body(region.polygons)
    for polygon in *(args.fixed_polygons or {})
        -- Use density 0.0 to represent a static body
        -- These will not generate transforms in _spread_shapes
        b2world\add_body({polygon}, 0.0)
    return b2world

spread_map_regions = (args) ->
    regions = args.regions
    _spread_shapes {
        b2world: _b2_world_from_args(args)
        n_iterations: args.n_iterations
        n_subiterations: args.n_subiterations
        clump_once_near: args.clump_once_near
        mode: args.mode
        sync: (i, body) ->
            r = regions[i]
            t = body\GetTransform()
            -- Note: transform polygons in-place, as they can be shared between MapRegion's
            for p in *r.polygons
                B2Utils.transform_polygon(t, p)
    }

connect_map_regions = (args) ->
    assert args.rng, "Need 'rng' object"
    b2world = _b2_world_from_args(args)
    -- Perform connection pass
    connector = B2ShapeConnector.create(b2world)
    for i=1,args.n_connections or #args.regions
        if not connector\do_random_connection(args.rng)
            break
        b2world\visualize("Random connection #{i}")
    -- Tunnels have 'index_a', 'index_b', 'polygon'
    -- We add them to the region with index 'index_a'
    for tunnel in *connector.tunnels
        append args.regions[tunnel.index_a].tunnels, tunnel

visualize_map_regions = (args) ->
    if not DebugUtils.is_debug_visualization()
        return
    b2world = _b2_world_from_args(args)
    b2world\visualize(args.title)

return {:connect_map_regions, :spread_map_regions, :visualize_map_regions,
        has_overlaps: (args) ->
            b2world = _b2_world_from_args(args)
            return b2world\has_overlaps()
        create_b2world: _b2_world_from_args}
