{:B2World} = require "maps.B2World"
B2Utils = require "maps.B2Utils"

-- Standardizes the conversion from map regions and fixed polygons to a B2World
b2world_from_args = (args) ->
    b2world = B2World.create()
    for region in *args.regions
        b2world\add_body(region.polygons)
    for polygon in *(args.fixed_polygons or {})
        -- Use density 0.0 to represent a static body
        -- These will not generate transforms in _spread_shapes
        b2world\add_body({polygon}, 0.0)
    return b2world

-- as_b2world = (args, f) ->
--     b2world = b2world_from_args(args)
--     f(b2world)
--     for i,body in ipairs b2world.dynamic_bodies
--         r = args.regions[i]
--         t = body\GetTransform()
--         -- Note: transform polygons in-place, as they can be shared between MapRegion's
--         for p in *r.polygons
--             B2Utils.transform_polygon(t, p)

_try_tunnel = (b2world, index_a, index_b) ->
    -- Constants
    TUNNEL_WIDTH = 2
    DIST_THRESHOLD = 0.0
    bodies = b2world.dynamic_bodies

    if #bodies < 2
        error("Cannot connect a set of just one body!")
    body_a, body_b = bodies[index_a], bodies[index_b]

    -- Find closest points:
    p1, p2, dist = B2Utils.body_distance(body_a, body_b)
    if not p1 or not p2
        return nil

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
    for b in *bodies
        -- Its OK for our bodies to be touched
        -- in the ray cast
        if b == body_a or b == body_b
            continue
        if B2Utils.ray_cast(b.fixtures, p1, p2)
            return nil
    -- Create tunnel
    -- TODO allow width customization
    -- shape = {
    --     {x1 - dy, y1 + dx}
    --     {x1 + dy, y1 - dx}
    --     {x2 + dy, y2 - dx}
    --     {x2 - dy, y2 + dx}
    -- }
    -- tunnel_body = b2world\CreateBody with b2.BodyDef()
    --     .position\Set(0, 0)
    -- tunnel_body\CreateFixture with b2.PolygonShape()
    --     \Set(shape),
    --     0.0
    return {}, {x1, y1}, {x2, y2}

_edges_fully_connected = (edges) ->
    visited = [false for i=1,#edges]
    nodes = {1}
    while #nodes > 0
        next_nodes = {}
        for node in *nodes
            visited[node] = true
            table.insert_all next_nodes, edges[node]
        nodes = next_nodes
    for v in *visited
        if not v
            return false
    return true

_try_connect_map_regions = (b2world, create_graph) ->
    tunnel_bodies = {}
    edges = [{} for i=1,#b2world.dynamic_bodies]
    pos_pairs = {}
    create_edge = (index_a, index_b) ->
        index_a, index_b = math.min(index_a, index_b), math.max(index_a, index_b)
        if table.contains(edges[index_a], index_b)
            return false
        body, p1, p2 = _try_tunnel(b2world, index_a, index_b)
        if body
            assert p1 and p2
            append pos_pairs, {p1, p2}
            -- append tunnel_bodies, body
            append edges[index_a], index_b
            return true
        return false
    if not create_graph(create_edge)
        return nil
    if not _edges_fully_connected(edges)
        for body in *tunnel_bodies
            b2world\remove_body(body)
        return nil
    return pos_pairs

connect_map_regions = (args, max_tries=100) ->
    -- Connect map regions, adding to their 'tunnels'
    b2world = b2world_from_args(args)
    for i=1,max_tries
        -- Adds to MapRegion 'tunnels' parameter
        pos_pairs = _try_connect_map_regions(b2world, args.create_graph)
        if pos_pairs
            return pos_pairs
    return nil

return nilprotect {:b2world_from_args, :connect_map_regions}
