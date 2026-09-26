----
-- Overworld variant "crossroads": four roads meeting at a village green.
-- A lush green sits at the heart of the valley, ringed by standing stones or shading a small
-- grove. Four long dirt roads wander out from it towards the four edges, each a chain of jittered
-- waypoints, and peter out into the forest before reaching the edge. Hamlet clearings sit along
-- the roads at irregular intervals, some right on the road, some down short side lanes.
----

SourceMap = require "core.SourceMap"
{:Region, :skewed_ellipse_points} = require "maps.GenerateUtils"
{:from_bbox} = require "maps.MapRegion"
{:TILES, :paint, :layout_area, :connect_walkable, :grid_regions, :create, :protect_matching} = require "map_descs.OverworldCommon"

ROAD_SEGMENTS = 5

glade = (rng, cx, cy, w, h) ->
    Region.create(math.floor(cx - w / 2), math.floor(cy - h / 2), w, h, rng\random(6, 13), rng\randomf(0, math.pi))

line = (map, a, b, width, tile) ->
    SourceMap.line_apply {:map, from_xy: {math.floor(a[1]), math.floor(a[2])}, to_xy: {math.floor(b[1]), math.floor(b[2])},
        line_width: width, operator: paint(tile)}

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    cx, cy = (x1 + x2) / 2 + rng\random(-4, 5), (y1 + y2) / 2 + rng\random(-4, 5)
    clamp = (v, lo, hi) -> math.max(lo, math.min(hi, v))
    inside = (x, y) -> {clamp(x, x1 + 4, x2 - 5), clamp(y, y1 + 4, y2 - 5)}

    -- The village green
    green_w, green_h = rng\random(30, 36), rng\random(28, 34)
    SourceMap.polygon_apply {:map, operator: paint(TILES.grass_lush),
        points: skewed_ellipse_points(rng, {math.floor(cx), math.floor(cy)}, {green_w, green_h}, 16)}
    -- No vault or dungeon entrance is built on the green
    protect_matching(map, {math.floor(cx - green_w / 2), math.floor(cy - green_h / 2), math.ceil(cx + green_w / 2), math.ceil(cy + green_h / 2)}, {})

    -- Four roads, roughly at right angles, each a chain of jittered waypoints
    hamlets = {}
    start = rng\randomf(0, math.pi / 2)
    roads = {}
    for i=0,3
        angle = start + i * math.pi / 2 + rng\randomf(-0.2, 0.2)
        dx, dy = math.cos(angle), math.sin(angle)
        len = rng\random(47, 55)
        points = {{cx, cy}}
        for s=1,ROAD_SEGMENTS
            d = len * s / ROAD_SEGMENTS
            -- Sideways jitter, gentler near the green so the roads meet cleanly
            j = if s == 1 then rng\randomf(-1.5, 1.5) else rng\randomf(-5, 5)
            append points, inside(cx + dx * d - dy * j, cy + dy * d + dx * j)
        append roads, {:points, :dx, :dy}

        -- Hamlets at irregular distances along the road
        d = rng\random(19, 24)
        while d < len - 6
            t = d / len * ROAD_SEGMENTS
            k = math.min(ROAD_SEGMENTS, math.floor(t) + 1)
            f = t - (k - 1)
            a, b = points[k], points[k + 1] or points[k]
            px, py = a[1] + (b[1] - a[1]) * f, a[2] + (b[2] - a[2]) * f
            side = if rng\chance(0.5) then 1 else -1
            -- Some sit on the road, some down a short side lane
            off = if rng\chance(0.5) then rng\random(3, 6) else rng\random(12, 18)
            hx, hy = unpack inside(px - dy * off * side, py + dx * off * side)
            w, h = rng\random(14, 20), rng\random(12, 18)
            append hamlets, {glade: glade(rng, hx, hy, w, h), road_xy: {px, py}, lane: off > 8}
            d += rng\random(12, 19)

    -- Hamlets: grass glades, a few dry
    for h in *hamlets
        h.glade\apply {:map, operator: paint(if rng\chance(0.3) then TILES.grass_dry else TILES.grass)}

    -- Road verges then the dirt road on top; the last stretch narrows to a grassy track
    for {:points} in *roads
        for s=1,ROAD_SEGMENTS
            last = s == ROAD_SEGMENTS
            line(map, points[s], points[s + 1], (if last then 2 else 6), TILES.grass)
        for s=1,ROAD_SEGMENTS - 1
            line(map, points[s], points[s + 1], 3, TILES.dirt)
    for h in *hamlets
        if h.lane
            line(map, h.road_xy, {h.glade\center()}, 2, TILES.dirt)

    -- The heart of the green: a ring of standing stones, or a small grove
    if rng\chance(0.6)
        n = rng\random(7, 10)
        r = rng\randomf(6, 8)
        a0 = rng\randomf(0, 2 * math.pi)
        for i=1,n
            a = a0 + i * 2 * math.pi / n
            map\square_apply({math.floor(cx + math.cos(a) * r), math.floor(cy + math.sin(a) * r)}, paint(TILES.stone))
    else
        SourceMap.polygon_apply {:map, operator: paint(TILES.trees),
            points: skewed_ellipse_points(rng, {math.floor(cx), math.floor(cy)}, {rng\random(5, 8), rng\random(5, 8)}, 8)}

    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    -- Dungeons are dug near the hamlets
    map.dungeon_regions = for h in *hamlets
        {hx, hy} = {h.glade\center()}
        from_bbox(math.max(x1, hx - 14), math.max(y1, hy - 14), math.min(x2, hx + 14), math.min(y2, hy + 14))
    return true

return create(layout, "crossroads")
