----
-- Overworld variant "spiral": a road coiling in to a central clearing.
-- An Archimedean spiral is traced as short wobbly segments, two to three turns from the outer
-- valley in to a clearing at the centre, with forest standing between the turns. Glades bulge off
-- the road at intervals, and a few shortcut paths cut across between neighbouring turns so the
-- valley is not one long corridor.
----

SourceMap = require "core.SourceMap"
{:Region, :skewed_ellipse_points} = require "maps.GenerateUtils"
{:from_bbox} = require "maps.MapRegion"
{:TILES, :paint, :WALKABLE, :layout_area, :connect_walkable, :grid_regions, :create, :protect_matching} = require "map_descs.OverworldCommon"

STEP = 3          -- rough arc length of each traced segment, in squares
OUTER_RADIUS = 52
INNER_RADIUS = 9

glade = (rng, cx, cy, w, h) ->
    Region.create(math.floor(cx - w / 2), math.floor(cy - h / 2), w, h, rng\random(6, 13), rng\randomf(0, math.pi))

line = (map, a, b, width, tile) ->
    SourceMap.line_apply {:map, from_xy: {math.floor(a[1]), math.floor(a[2])}, to_xy: {math.floor(b[1]), math.floor(b[2])},
        line_width: width, operator: paint(tile)}

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    cx, cy = (x1 + x2) / 2 + rng\random(-3, 4), (y1 + y2) / 2 + rng\random(-3, 4)

    -- Trace the spiral from the outside in: radius shrinks linearly with the angle turned, with a
    -- slow wave and a little per-point jitter so the road wanders
    turns = rng\randomf(2.2, 2.8)
    total = turns * 2 * math.pi
    spin = if rng\chance(0.5) then 1 else -1
    a0 = rng\randomf(0, 2 * math.pi)
    squash = rng\randomf(0.88, 1)   -- slightly oval
    wave_k, wave_phase = rng\random(3, 6), rng\randomf(0, 2 * math.pi)
    radius_at = (t) -> OUTER_RADIUS - (OUTER_RADIUS - INNER_RADIUS) * t / total
    point_at = (t, dr = 0) ->
        r = radius_at(t) + math.sin(t * wave_k / 2 + wave_phase) * 1.5 + dr
        a = a0 + spin * t
        return {cx + math.cos(a) * r, cy + math.sin(a) * r * squash}
    path = {}
    t = 0
    while t < total
        {px, py} = point_at(t, rng\randomf(-0.8, 0.8))
        append path, {px, py, :t}
        t += STEP / math.max(radius_at(t), 1)
    append path, {cx, cy, t: total}

    -- The outer valley where the road begins, and the clearing at the centre
    {sx, sy} = point_at(0)
    SourceMap.polygon_apply {:map, operator: paint(TILES.grass),
        points: skewed_ellipse_points(rng, {math.floor(sx), math.floor(sy)}, {rng\random(12, 16), rng\random(12, 16)}, 12)}
    SourceMap.polygon_apply {:map, operator: paint(TILES.grass_lush),
        points: skewed_ellipse_points(rng, {math.floor(cx), math.floor(cy)}, {rng\random(20, 25), rng\random(20, 25)}, 14)}
    -- The clearing the road leads to stays clear of vaults
    protect_matching(map, {math.floor(cx) - 10, math.floor(cy) - 10, math.floor(cx) + 11, math.floor(cy) + 11}, {})

    -- Glades bulge off the road at intervals, on its inner or outer side
    glades = {}
    t = rng\randomf(1.2, 2)
    while t < total - 2.5
        side = if rng\chance(0.5) then 1 else -1
        {gx, gy} = point_at(t, side * rng\randomf(4, 6))
        g = glade(rng, gx, gy, rng\random(9, 13), rng\random(9, 13))
        g\apply {:map, operator: paint(if rng\chance(0.3) then TILES.grass_lush else TILES.grass)}
        append glades, g
        t += rng\randomf(1.1, 1.7)

    -- The road: a grass verge with the dirt track on top
    for i=1,#path - 1
        line(map, path[i], path[i + 1], 5, TILES.grass)
    for i=1,#path - 1
        line(map, path[i], path[i + 1], 3, TILES.dirt)

    -- Shortcuts: straight radial paths from one turn to the next one in
    n_shortcuts = rng\random(2, 4)
    for i=1,n_shortcuts
        -- Spread over the turns that have another turn inside them
        t = (total - 2 * math.pi - 0.5) * (i - rng\randomf(0.2, 0.8)) / n_shortcuts
        line(map, point_at(t), point_at(t + 2 * math.pi), 2, TILES.dirt)

    -- Tidy the verges: the short segments leave a comb of spikes and notches along the road edge
    SourceMap.automaton_apply {:map, :rng, :area, rules: {
        paint(TILES.trees, {matches_content: TILES.grass.id, neighbours: WALKABLE, max_neighbours: 2})
        paint(TILES.grass, {matches_content: TILES.trees.id, neighbours: WALKABLE, min_neighbours: 6})
    }}

    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    -- Dungeons are dug near the glades along the road
    map.dungeon_regions = for g in *glades
        {gx, gy} = {g\center()}
        from_bbox(math.max(x1, gx - 14), math.max(y1, gy - 14), math.min(x2, gx + 14), math.min(y2, gy + 14))
    return true

return create(layout, "spiral")
