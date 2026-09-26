----
-- Overworld variant "ravine": a gorge splits the valley.
-- A deep rock ravine meanders from one side of the valley to the other, its centreline a sum of
-- sine waves. Sheer stone walls drop to a narrow floor of dirt and dry grass. The wooded uplands
-- either side hold glades joined by trails, and a few ramps cut down through the walls (one of
-- them crossing straight over) so both sides reach the ravine floor and each other.
----

SourceMap = require "core.SourceMap"
{:Region} = require "maps.GenerateUtils"
{:TILES, :paint, :layout_area, :forest_rim, :connect_walkable, :grid_regions, :create, :protect_matching} = require "map_descs.OverworldCommon"

-- Trail walkers push through forest and open ground, never through the ravine walls
NOT_ROCK = {matches_all: {SourceMap.FLAG_SEETHROUGH}}
-- How far either side of the centreline the nearest centreline point is searched for
SEARCH = 14

-- A smooth meander: a sum of sine waves with random wavelengths and phases
wave = (rng, amplitudes, wavelengths) ->
    parts = for i, a in ipairs amplitudes
        {a * rng\randomf(0.75, 1), 2 * math.pi / (wavelengths[i] * rng\randomf(0.8, 1.25)), rng\randomf(0, 2 * math.pi)}
    return (t) ->
        s = 0
        for {a, f, p} in *parts
            s += a * math.sin(f * t + p)
        return s

-- A minimum spanning tree over points, as index pairs
spanning_tree = (points) ->
    dist = (a, b) -> (a[1] - b[1])^2 + (a[2] - b[2])^2
    joined, edges = {true}, {}
    for _=2,#points
        best = nil
        for i=1,#points
            if joined[i]
                for j=1,#points
                    if not joined[j] and (not best or dist(points[i], points[j]) < best[3])
                        best = {i, j, dist(points[i], points[j])}
        joined[best[2]] = true
        append edges, {best[1], best[2]}
    return edges

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area

    -- Work in (u, v): u runs along the ravine, v across it. A vertical ravine swaps the axes
    vertical = rng\random(2) == 0
    to_xy = (u, v) -> if vertical then {math.floor(v), math.floor(u)} else {math.floor(u), math.floor(v)}
    u1, u2, v1, v2 = x1, x2, y1, y2
    if vertical
        u1, u2, v1, v2 = y1, y2, x1, x2

    -- The ravine's course, and how wide its floor and walls are along it
    mid = (v1 + v2) / 2 + rng\randomf(-8, 8)
    meander = wave(rng, {16, 5}, {120, 36})
    centre = (u) -> mid + meander(u)
    -- Near the valley's edges the ravine narrows and peters out into the woods, rather than
    -- stopping at the straight edge of the layout area
    u_start, u_end = u1 + rng\random(0, 5), u2 - rng\random(1, 6)
    taper = (u) -> math.max(0, math.min(1, (u - u_start) / 10, (u_end - u) / 10))
    floor_wave = wave(rng, {0.7, 0.3}, {34, 11})
    floor_half = (u) -> (1.7 + floor_wave(u)) * taper(u)^0.5
    wall_wave = wave(rng, {1.2, 0.6}, {45, 14})
    wall = (u) -> (4.2 + wall_wave(u)) * taper(u)^0.7
    outer = (u) -> floor_half(u) + wall(u)

    -- Cut the ravine: each square finds its distance to the nearest point of the centreline
    line = {}
    for u=u1-SEARCH,u2+SEARCH
        line[u] = centre(u)
    for u=u1,u2-1
        for v=v1,v2-1
            best_d, best_u = math.huge, u
            for w=u-SEARCH,u+SEARCH
                d = (w - u)^2 + (line[w] - v)^2
                if d < best_d
                    best_d, best_u = d, w
            d = math.sqrt(best_d)
            if taper(best_u) <= 0
                continue
            if d <= floor_half(best_u)
                map\square_apply(to_xy(u, v), paint(TILES.dirt))
            elseif d <= outer(best_u) + rng\randomf(-0.4, 0.6)
                map\square_apply(to_xy(u, v), paint(TILES.stone))
    -- Dry grass in patches along the floor
    SourceMap.noise_apply {:map, :area, seed: rng\random(0, 2^30), scale: 8, octaves: 2, bands: {
        paint(TILES.grass_dry, {max: 0, matches_content: TILES.dirt.id})
    }}

    -- Ramps down through the walls: one crossing cuts both walls, the others descend from one
    -- side only. heads[side] collects the upland end of each, for the trails to reach
    heads = {[-1]: {}, [1]: {}}
    ramp = (u, side) ->
        du = rng\random(4, 9) * (if rng\random(2) == 0 then 1 else -1)
        top_u = math.max(u1 + 2, math.min(u2 - 3, u + du))
        top_v = centre(top_u) + side * (outer(top_u) + 3)
        top_v = math.max(v1 + 2, math.min(v2 - 3, top_v))
        SourceMap.line_apply {:map, from_xy: to_xy(u, centre(u)), to_xy: to_xy(top_u, top_v), line_width: 2, operator: paint(TILES.dirt)}
        append heads[side], to_xy(top_u, top_v)
    slots = {1, 2, 3, 4}
    for i=#slots,2,-1
        j = rng\random(1, i + 1)
        slots[i], slots[j] = slots[j], slots[i]
    slot_u = (slot) -> math.floor(u1 + 12 + (slot - 1 + rng\randomf(0.2, 0.8)) * (u2 - u1 - 24) / 4)
    crossing = slot_u(slots[1])
    ramp(crossing, -1)
    ramp(crossing, 1)
    ramp(slot_u(slots[2]), -1)
    ramp(slot_u(slots[3]), 1)
    if rng\random(2) == 0
        ramp(slot_u(slots[4]), if rng\random(2) == 0 then -1 else 1)

    -- Glades in the uplands, clear of the ravine and of each other
    glades = {[-1]: {}, [1]: {}}
    placed = {}
    n_glades = rng\random(11, 15)
    for attempt=1,600
        if #placed >= n_glades
            break
        u, v = rng\random(u1 + 9, u2 - 9), rng\random(v1 + 9, v2 - 9)
        w, h = rng\random(12, 19), rng\random(12, 19)
        r = math.max(w, h) / 2
        across = v - centre(u)
        if math.abs(across) - outer(u) < r + 2
            continue
        ok = true
        for {pu, pv} in *placed
            if (pu - u)^2 + (pv - v)^2 < 17^2
                ok = false
                break
        if not ok
            continue
        append placed, {u, v}
        {x, y} = to_xy(u, v)
        g = Region.create(math.floor(x - w / 2), math.floor(y - h / 2), w, h, rng\random(7, 13), rng\randomf(0, math.pi))
        g\apply {:map, operator: paint((if rng\random(3) == 0 then TILES.grass_lush else TILES.grass), {matches_content: TILES.trees.id})}
        append glades[if across < 0 then -1 else 1], {x, y}
    if #placed < 8
        return false

    -- Trails: on each side, a spanning tree over its glades and ramp heads
    for side in *{-1, 1}
        nodes = [g for g in *glades[side]]
        for h in *heads[side]
            append nodes, h
        for {a, b} in *spanning_tree(nodes)
            SourceMap.random_walk_apply {
                :map, :rng, :area, from_xy: nodes[a], toward_xy: nodes[b], toward_bias: 0.7, steps: 2000, width: 2
                passable: NOT_ROCK, operator: paint(TILES.dirt, {matches_content: TILES.trees.id})
            }

    -- The valley fades into the forest at its edges
    forest_rim(map, rng, area, 0.2, -0.5)
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    -- No vault is built across the gorge: its rock walls are protected
    protect_matching(map, area, {matches_content: TILES.stone.id})
    return true

return create(layout, "rootways")
