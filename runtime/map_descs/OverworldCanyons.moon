----
-- Overworld variant "canyons": rock country cut by narrow canyons.
-- The inverse of the forest variants: the valley starts as solid rock. A few junction basins are
-- hollowed out, then random walkers of varying width (2-4) carve winding canyons between them,
-- plus some blind side canyons. The floors are dirt and dry grass, a few hardy trees cling to
-- the canyon walls, and the rock gives way to forest at the rim.
----

SourceMap = require "core.SourceMap"
{:skewed_ellipse_points} = require "maps.GenerateUtils"
{:TILES, :paint, :layout_area, :connect_walkable, :grid_regions, :create} = require "map_descs.OverworldCommon"

ROCK = {matches_content: TILES.stone.id}

-- Junction basins spread over the valley, at least min_dist apart
place_basins = (rng, area, n, min_dist) ->
    {x1, y1, x2, y2} = area
    basins = {}
    for attempt=1,400
        if #basins >= n
            break
        x, y = rng\random(x1 + 14, x2 - 14), rng\random(y1 + 14, y2 - 14)
        ok = true
        for {bx, by} in *basins
            if (bx - x)^2 + (by - y)^2 < min_dist^2
                ok = false
                break
        if ok
            append basins, {x, y}
    return basins

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

-- Carves a canyon from start towards target, in stretches of random width (2-4) so it narrows
-- and widens along its length. Gives up after max_steps
carve_canyon = (map, rng, area, start, target, max_steps, bias = 0.6) ->
    at, steps = start, 0
    while steps < max_steps
        at = SourceMap.random_walk_apply {
            :map, :rng, :area, from_xy: at, toward_xy: target, toward_bias: bias
            steps: 25, width: rng\random(2, 5), passable: {}, operator: paint(TILES.dirt)
        }
        steps += 25
        if at[1] == target[1] and at[2] == target[2]
            break

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area

    SourceMap.rectangle_apply {:map, area: {x1, y1, x2, y2}, fill_operator: paint(TILES.stone), create_subgroup: false}

    -- Junction basins: small open hollows where canyons meet
    basins = place_basins(rng, area, rng\random(6, 9), 25)
    if #basins < 4
        return false
    for {bx, by} in *basins
        SourceMap.polygon_apply {:map, operator: paint(TILES.dirt),
            points: skewed_ellipse_points(rng, {bx, by}, {rng\random(10, 16), rng\random(10, 16)}, 12)}

    -- Canyons join the basins (a spanning tree plus a loop or two)...
    routes = spanning_tree(basins)
    for _=1,rng\random(3, 5)
        a, b = rng\random(1, #basins + 1), rng\random(1, #basins + 1)
        if a ~= b
            append routes, {a, b}
    for {a, b} in *routes
        carve_canyon(map, rng, area, basins[a], basins[b], 600)
    -- ...and blind side canyons wander off from them into the rock
    for _=1,rng\random(10, 15)
        start = basins[rng\random(1, #basins + 1)]
        target = {rng\random(x1 + 6, x2 - 6), rng\random(y1 + 6, y2 - 6)}
        carve_canyon(map, rng, area, start, target, rng\random(120, 220), 0.5)

    -- Knock down lone rock pillars left standing in the canyons
    SourceMap.automaton_apply {:map, :rng, :area, rules: {
        paint(TILES.dirt, {matches_content: TILES.stone.id, neighbours: ROCK, max_neighbours: 2})
    }}
    -- Dry grass in patches over the canyon floors
    SourceMap.noise_apply {:map, :area, seed: rng\random(0, 2^30), scale: 9, octaves: 2, bands: {
        paint(TILES.grass_dry, {max: 0.05, matches_content: TILES.dirt.id})
    }}
    -- Occasional hardy trees, clinging to the canyon walls
    SourceMap.automaton_apply {:map, :rng, :area, rules: {
        paint(TILES.trees, {matches_none: {SourceMap.FLAG_SOLID}, neighbours: ROCK, min_neighbours: 3, chance: 0.05})
    }}

    -- The rock country fades into forest at its edges
    -- (broad noise, so the woods come in as a few deep bays rather than speckle)
    SourceMap.noise_apply {:map, :area, seed: rng\random(0, 2^30), scale: 18, octaves: 3, falloff: 0.4, bands: {
        paint(TILES.trees, {max: -0.4})
    }}
    SourceMap.automaton_apply {:map, :rng, :area, iterations: 2, outside_counts: true, rules: {
        paint(TILES.trees, {matches_content: TILES.stone.id, neighbours: {matches_content: TILES.trees.id}, min_neighbours: 4})
    }}
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    return true

return create(layout)
