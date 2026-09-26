----
-- Overworld variant "ringwood": a layout built from lanarts' shape tools.
-- A rock mesa rises at the centre of the valley with a small summit glade, reached by one path.
-- A ring of glades circles it, linked by a loop road, and outer glades hang off the
-- ring on spokes. Glades are the same ellipse regions the classic generator uses, each with its
-- own point count and tilt, so none are quite round.
----

SourceMap = require "core.SourceMap"
{:Region, :skewed_ellipse_points} = require "maps.GenerateUtils"
{:TILES, :paint, :layout_area, :connect_walkable, :create, :protect_matching} = require "map_descs.OverworldCommon"

glade = (rng, cx, cy, w, h) ->
    r = Region.create(math.floor(cx - w / 2), math.floor(cy - h / 2), w, h, rng\random(6, 13), rng\randomf(0, math.pi))
    r.conf = {n_statues: 1}
    return r

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    cx, cy = math.floor((x1 + x2) / 2), math.floor((y1 + y2) / 2)
    half = math.min(x2 - x1, y2 - y1) / 2

    -- The mesa, and its summit
    mesa = rng\random(30, 38)
    SourceMap.polygon_apply {:map, operator: paint(TILES.stone),
        points: skewed_ellipse_points(rng, {cx, cy}, {mesa, mesa}, 18)}   -- centred on its xy
    summit = glade(rng, cx, cy, 8, 8)
    summit\apply {:map, operator: paint(TILES.grass_dry)}
    -- The summit glade stays clear of vaults
    protect_matching(map, {cx - 5, cy - 5, cx + 6, cy + 6}, {})

    -- The ring of glades around it, at evenly spread angles with some wobble
    n = rng\random(7, 11)
    ring_radius = half * 0.58
    start = rng\randomf(0, 2 * math.pi)
    ring = for i=1,n
        a = start + (i - 1) * 2 * math.pi / n + rng\randomf(-0.2, 0.2)
        r = ring_radius + rng\randomf(-3, 3)
        glade(rng, cx + math.cos(a) * r, cy + math.sin(a) * r, rng\random(15, 22), rng\random(14, 20))
    for i, g in ipairs ring
        g\apply {:map, operator: paint(if i % 3 == 0 then TILES.grass_lush else TILES.grass)}
    -- The loop road: each glade to the next
    for i=1,n
        ring[i]\line_connect {:map, target: ring[i % n + 1], line_width: 3, operator: paint(TILES.dirt)}

    -- One path climbs the mesa to the summit
    ring[1]\line_connect {:map, target: summit, line_width: 2, operator: paint(TILES.dirt)}

    -- Outer glades, each on a spoke from the nearest ring glade
    outer = for i=1,rng\random(4, 7)
        a = rng\randomf(0, 2 * math.pi)
        r = half * rng\randomf(0.82, 0.9)
        glade(rng, cx + math.cos(a) * r, cy + math.sin(a) * r, rng\random(8, 13), rng\random(8, 13))
    for g in *outer
        g\apply {:map, operator: paint(TILES.grass_lush)}
        nearest = ring[1]
        for r in *ring
            if g\square_distance(r) < g\square_distance(nearest)
                nearest = r
        g\line_connect {:map, target: nearest, line_width: 2, operator: paint(TILES.grass)}

    if not connect_walkable(map, area, 0.1)
        return false
    map.regions = [g for g in *ring]
    return true

return create(layout)
