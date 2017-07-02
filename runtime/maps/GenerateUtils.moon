RVOWorld = require "core.RVOWorld"
SourceMap = require "core.SourceMap"

LEVEL_PADDING = {10, 10}
MAX_TRIES = 1000

----
-- Polygon based regions

ellipse_points = (x, y, w, h, n_points = 16, start_angle = 0) ->
    points = {}
    angle,step = start_angle,(1/n_points* 2 * math.pi)
    cx, cy = x+w/2, y+h/2
    for i=1,n_points
        append points, {(math.sin(angle) + 1)/2 * w + x, (math.cos(angle)+1)/2 * h + y}
        angle += step
    return points

-- TODO migrate to this, ellipse centered on x,y
ellipse_points_0 = (x, y, w, h, n_points = 16, start_angle = 0) ->
    points = {}
    angle,step = start_angle,(1/n_points* 2 * math.pi)
    cx, cy = x+w/2, y+h/2
    for i=1,n_points
        append points, {math.sin(angle)/2 * w + x, math.cos(angle)/2 * h + y}
        angle += step
    return points


skewed_ellipse_points = (rng, xy, wh, n_points = 16, start_angle = 0) ->
    {x, y} = xy
    {w, h} = wh
    while true
        points = {}
        angle,step = start_angle,(1/n_points* 2 * math.pi)
        cx, cy = x+w/2, y+h/2
        for i=1,n_points
            append points, {math.sin(angle) * w / 2 + x, math.cos(angle) * h / 2 + y}
            angle += rng\randomf(0, step) + step / 2
        -- Reject all that have too high angle:
        if angle < 1.9 * math.pi
            return points

-- Adapter for the new MapRegion class for the old Region API
-- TODO normalize this mess. Low priority while content builds and churns
MapRegionAdapter = newtype {
    init: (@region) =>
        bbox = require("maps.MapRegion").map_region_bbox(@region)
        @x,@y = bbox[1],bbox[2]
        @ox,@oy = @x,@y
        @w,@h = bbox[3]-bbox[1],bbox[4]-bbox[2]
        @_points = false
        @subregions = {}
    add: (subregion) => error("Not emulated")
    get: {
        points: () => 
            @_points or= require("maps.GeometryUtils").convex_hull @region.polygons
            return @_points
    }
    apply: (args) =>
        args.points = @points
        SourceMap.polygon_apply(args)
    bbox: () => {@x, @y, @x+@w, @y+@h}
    ortho_dist: (o) =>
        cx,cy = @center()
        ocx, ocy = o\center()
        dx, dy = cx - ocx, cy - ocy
        return math.max math.abs(dx),math.abs(dy)

    square_distance: (o) =>
        cx,cy = @center()
        ocx, ocy = o\center()
        dx, dy = cx - ocx, cy - ocy
        return dx*dx + dy*dy

    ellipse_intersect: (x,y,w,h) =>
        cx, cy = @center()
        cxo,cyo = x+w/2,y+h/2
        dx,dy = cxo-cx, cyo-cy
        -- Local radius depends on angle:
        ang = math.atan2(dy, dx)
        r1 = math.sqrt(  (@w/2*math.cos(ang))^2 + (@h/2*math.sin(ang))^2  )
        r2 = math.sqrt(  (w/2*math.cos(-ang))^2 + (h/2*math.sin(-ang))^2  )
        -- Condense into unit coordinates:
        return (math.sqrt(dx*dx+dy*dy) < (r1+r2))

    rect_intersect: (x,y,w,h) =>
        if @x > x+w or x > @x+@w
            return false
        if @y > y+h or y > @y+@h
            return false
        return true
 
    center: () =>
        return math.floor(@x+@w/2), math.floor(@y+@h/2)
    -- Create a line between the two regions
    line_connect: (args) =>
        args.from_xy = {@center()}
        args.to_xy = {args.target\center()}
        SourceMap.line_apply(args)
    line_match: (args) =>
        args.from_xy = {@center()}
        args.to_xy = {args.target\center()}
        SourceMap.line_match(args)
    _arc_adjust_params: (args) =>
        cx, cy = @center()
        ocx, ocy = args.target\center()
        w, h = math.abs(cx - ocx) - 1, math.abs(cy - ocy) - 1
        if w < 2 or h < 2 -- or w > 35 or h > 35
            return false
        -- Set up the ellipse section for our connection:
        args.width, args.height = w*2, h*2
        args.x, args.y = math.floor((cx+ocx)/2), math.floor((cy+ocy)/2)
        a1 = math.atan2((args.y - cy) / h , (args.x - cx)/w)
        a2 = math.atan2((args.y - ocy) / h, (args.x - ocx)/w)
        args.angle1, args.angle2 = a1 + math.pi/2, (a2 - a1)
        return true
    arc_connect: (args) =>
        if not @_arc_adjust_params(args)
            return @line_connect(args)
        SourceMap.arc_apply(args)
    arc_match: (args) =>
        if not @_arc_adjust_params(args)
            return @line_match(args)
        SourceMap.arc_match(args)
}

Region = newtype {
    init: (@x, @y, @w, @h, @n_points = 16, @angle = 0) =>
        @_points = false
        @subregions = {}
    add: (subregion) => append @subregions, subregion
    get: {
        points: () => 
            @_points or= ellipse_points(@x, @y, @w, @h, @n_points, @angle)
            return @_points
    }
    apply: (args) =>
        args.points = @points
        SourceMap.polygon_apply(args)
    bbox: () => {@x, @y, @x+@w, @y+@h}
    ortho_dist: (o) =>
        cx,cy = @center()
        ocx, ocy = o\center()
        dx, dy = cx - ocx, cy - ocy
        return math.max math.abs(dx),math.abs(dy)

    square_distance: (o) =>
        cx,cy = @center()
        ocx, ocy = o\center()
        dx, dy = cx - ocx, cy - ocy
        return dx*dx + dy*dy

    ellipse_intersect: (x,y,w,h) =>
        cx, cy = @center()
        cxo,cyo = x+w/2,y+h/2
        dx,dy = cxo-cx, cyo-cy
        -- Local radius depends on angle:
        ang = math.atan2(dy, dx)
        r1 = math.sqrt(  (@w/2*math.cos(ang))^2 + (@h/2*math.sin(ang))^2  )
        r2 = math.sqrt(  (w/2*math.cos(-ang))^2 + (h/2*math.sin(-ang))^2  )
        -- Condense into unit coordinates:
        return (math.sqrt(dx*dx+dy*dy) < (r1+r2))

    rect_intersect: (x,y,w,h) =>
        if @x > x+w or x > @x+@w
            return false
        if @y > y+h or y > @y+@h
            return false
        return true
 
    center: () =>
        return math.floor(@x+@w/2), math.floor(@y+@h/2)
    -- Create a line between the two regions
    line_connect: (args) =>
        args.from_xy = {@center()}
        args.to_xy = {args.target\center()}
        SourceMap.line_apply(args)
    line_match: (args) =>
        args.from_xy = {@center()}
        args.to_xy = {args.target\center()}
        SourceMap.line_match(args)
    _arc_adjust_params: (args) =>
        cx, cy = @center()
        ocx, ocy = args.target\center()
        w, h = math.abs(cx - ocx) - 1, math.abs(cy - ocy) - 1
        if w < 2 or h < 2 -- or w > 35 or h > 35
            return false
        -- Set up the ellipse section for our connection:
        args.width, args.height = w*2, h*2
        args.x, args.y = math.floor((cx+ocx)/2), math.floor((cy+ocy)/2)
        a1 = math.atan2((args.y - cy) / h , (args.x - cx)/w)
        a2 = math.atan2((args.y - ocy) / h, (args.x - ocx)/w)
        args.angle1, args.angle2 = a1 + math.pi/2, (a2 - a1)
        return true
    arc_connect: (args) =>
        if not @_arc_adjust_params(args)
            return @line_connect(args)
        SourceMap.arc_apply(args)
    arc_match: (args) =>
        if not @_arc_adjust_params(args)
            return @line_match(args)
        SourceMap.arc_match(args)
}

_edge_list_append_if_unique = (edges, p1, p2) ->
    for {op1, op2} in *edges
        if op1 == p1 and op2 == p2 or op2 == p1 and op1 == p2
            return
    append edges, {p1, p2}

-- Returns a list of edges
subregion_minimum_spanning_tree = (R, acceptable_distf = nil) ->
    -- R: The list of regions
    -- C: The connected set
    C = {false for p in *R}
    C[1] = true -- Start with the first region in the 'connected set'
    edge_list = {}
    while true
        -- Find the next edge to add:
        min_sqr_dist = math.huge
        min_i, min_j = nil, nil
        sub_i, sub_j = nil, nil
        for i=1,#R do if C[i] 
            for j=1,#R do if not C[j]
                SI,SJ = R[i].subregions,R[j].subregions
                for si in *SI 
                    for sj in *SJ
                        sqr_dist = si\square_distance(sj)
                        if acceptable_distf and sqr_dist < acceptable_distf(si, sj)^2
                            _edge_list_append_if_unique(edge_list, si, sj)
                        elseif sqr_dist < min_sqr_dist
                            min_sqr_dist = sqr_dist
                            min_i, min_j = i, j
                            sub_i, sub_j = sj, si
        -- All should be connected by this point
        if min_i == nil
            break
        C[min_j] = true
        append edge_list, {sub_i, sub_j}

    return edge_list

-- Returns a list of edges
region_minimum_spanning_tree = (R) ->
    -- R: The list of regions
    -- C: The connected set
    C = {false for p in *R}
    C[1] = true -- Start with the first region in the 'connected set'
    edge_list = {}
    while true
        -- Find the next edge to add:
        min_sqr_dist = math.huge
        min_i, min_j = nil, nil
        for i=1,#R do if C[i] 
            for j=1,#R do if not C[j]
                sqr_dist = R[i]\square_distance(R[j])
                if sqr_dist < min_sqr_dist
                    min_sqr_dist = sqr_dist
                    min_i, min_j = i, j
        -- All should be connected by this point
        if min_i == nil
            break
        C[min_j] = true
        append edge_list, {R[min_i], R[min_j]}

    return edge_list

-- A scheme based on circles dynamically placed in a room
RVORegionPlacer = newtype {
    init: (boundary = nil) =>
        @rvo = RVOWorld.create(boundary)
        @regions = {}

    add: (region, velocity_func) =>
        append @regions, region
        {:x, :y, :w, :h} = region
        region.max_speed = rawget(region, "max_speed") or 1
        r = math.max(w,h) -- Be conservative with the radius
        region.id = @rvo\add_instance(x+w/2, y+h/2, math.ceil(r), region.max_speed)
        region.velocity_func = velocity_func
    step: () =>
        for region in *@regions
            cx, cy = region\center()
            vx, vy = region\velocity_func()
            r = math.min(region.w,region.h) / 2-- Be conservative with the radius
            @rvo\update_instance(region.id, cx, cy, math.ceil(r), region.max_speed, vx, vy)
        @rvo\step()
        for region in *@regions
            vx,vy = @rvo\get_velocity(region.id)
            region.x, region.y = region.x + vx, region.y + vy
    steps: (n) => 
        for i=1,n do @step()
        @finish()
    finish: () =>
        for region in *@regions
            region.x, region.y = math.round(region.x), math.round(region.y)
}

random_rect_in_rect = (rng, w,h, xo,yo,wo,ho) ->
    return rng\random(xo,xo+wo-w), rng\random(yo,yo+ho-h), w, h

random_ellipse_in_ellipse = (rng, w,h, xo, yo, wo, ho) ->
    -- Make a random position in the circular room boundary:
    dist = rng\randomf(0, 1)
    ang = rng\randomf(0, 2*math.pi)
    cxo, cyo = xo+wo/2, yo+ho/2
    x = math.cos(ang) * dist * (wo/2 - w/2) + cxo - w/2
    y = math.sin(ang) * dist * (ho/2 - h/2) + cyo - h/2
    -- assert x >= xo - 0.1
    -- assert y >= yo - 0.1
    -- assert x + w <= (xo + wo)+0.1
    -- assert y + h <= (yo + ho)+0.1
    return x, y, w, h

region_intersects = (x,y,w,h, R) ->
    for r in *R.regions
        if r\ellipse_intersect(x,y,w,h)
            return true
    return false

random_region_add = (rng, w, h, n_points, velocity_func, angle, R, bbox, ignore_intersect = false) ->
    assert(w and h, "Must have size")
    for tries=1,MAX_TRIES
        {PW,PH} = LEVEL_PADDING
        {x1, y1, x2, y2} = bbox
        x,y = random_ellipse_in_ellipse(rng, w,h, x1, y1, x2-x1, y2-y1)
        if ignore_intersect or not region_intersects(x,y,w,h, R)
            r = Region.create(x,y,w,h, n_points, angle)
            R\add(r, velocity_func)
            return r
    return nil

Tile = newtype {
    init: (@id, @solid, @seethrough, add = {}, remove = {}) =>
        @add_flags = add
        @remove_flags = remove
        append (if @solid then @add_flags else @remove_flags), SourceMap.FLAG_SOLID
        append (if @seethrough then @add_flags else @remove_flags), SourceMap.FLAG_SEETHROUGH
}

tile_operator = (tile, data = {}) ->
    assert not data.content
    data.content = tile.id
    data.add or= {}
    data.remove or= {}
    if type(data.add) ~= "table" 
        data.add = {data.add}
    if type(data.remove) ~= "table" 
        data.remove = {data.remove}
    for flag in *tile.add_flags do append(data.add, flag)
    for flag in *tile.remove_flags do append(data.remove, flag)
    return data

spread_region_delta_func = (map, rng, outer) ->
    center_x, center_y = outer\center()
    return () => rng\randomf(-2,2), rng\randomf(-2,2)
        -- math.sign_of(@x - center_x)*2, math.sign_of(@y - center_y)*2

towards_region_delta_func = (map, rng, region) ->
    return () => math.sign_of(region.x - @x)*100, math.sign_of(region.y - @y)*100

center_region_delta_func = (map, rng, outer) ->
    center_x, center_y = outer\center()
    return () => math.sign_of(center_x - @x)*100, math.sign_of(center_y - @y)*100

default_region_delta_func = (map, rng, outer) ->
    center_x, center_y = outer\center()
    local vfunc 
    type = rng\random(0, 2) -- Only first two for now
    if type == 0
        return () => math.sign_of(@x - center_x)*2, math.sign_of(@y - center_y)*2
    elseif type == 1
        return () => math.sign_of(center_x - @x)*2, math.sign_of(center_y - @y)*2
    else --Unused
        return () => 0,0

ring_region_delta_func = (map, rng, outer) ->
    angle = rng\randomf(0, 2*math.pi)
    rx, ry = outer\center()
    rx, ry = rx-5, ry-5
    ring_n = rng\random(1,4)
    rx /= ring_n
    ry /= ring_n
    to_x, to_y = math.cos(angle)*rx + outer.w/2, math.sin(angle)*ry + outer.h/2
    return () => math.sign_of(to_x - @x)*10, math.sign_of(to_y - @y)*10

return {
    :LEVEL_PADDING, :ellipse_points, :Region, :RVORegionPlacer, :MapRegionAdapter
    :subregion_minimum_spanning_tree, :region_minimum_spanning_tree, 
    :spread_region_delta_func
    :center_region_delta_func
    :ellipse_points
    :ellipse_points_0
    :skewed_ellipse_points
    :towards_region_delta_func
    :random_rect_in_rect, :random_ellipse_in_ellipse, :Tile, :tile_operator
    :region_intersects, :random_region_add 
    :default_region_delta_func
    :ring_region_delta_func
}
