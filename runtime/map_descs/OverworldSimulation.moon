----
-- Overworld variant "settled woods": the valley is grown by simulation.
-- It starts as primeval forest. Settlers found camps and, year by year, woodcutters chop outward
-- from them, trade trails are worn between neighbouring camps, clearings widen where they're
-- worked and the forest reclaims ground left alone. Two camps quarry into rock, and the dungeon
-- entrances are dug near the settlements.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :WALKABLE, :layout_area, :connect_walkable, :grid_regions, :create} = require "map_descs.OverworldCommon"
{:from_bbox} = require "maps.MapRegion"

YEARS = 14

SOLID = SourceMap.FLAG_SOLID
-- Walkers can push through forest and open ground but not rock (trees are see-through, rock isn't)
NOT_ROCK = {matches_all: {SourceMap.FLAG_SEETHROUGH}}
CHOP = paint(TILES.grass, {matches_content: TILES.trees.id})
TRAIL = paint(TILES.dirt, {matches_none: {}})

-- Camps spread over the valley, at least min_dist apart
found_camps = (rng, area, n, min_dist) ->
    {x1, y1, x2, y2} = area
    camps = {}
    for attempt=1,400
        if #camps >= n
            break
        x, y = rng\random(x1 + 12, x2 - 12), rng\random(y1 + 12, y2 - 12)
        ok = true
        for {cx, cy} in *camps
            if (cx - x)^2 + (cy - y)^2 < min_dist^2
                ok = false
                break
        if ok
            append camps, {x, y}
    return camps

-- Trade routes: a minimum spanning tree over the camps, plus one extra link for a loop
trade_routes = (camps) ->
    dist = (a, b) -> (a[1] - b[1])^2 + (a[2] - b[2])^2
    joined, routes = {true}, {}
    for _=2,#camps
        best = nil
        for i=1,#camps
            if joined[i]
                for j=1,#camps
                    if not joined[j] and (not best or dist(camps[i], camps[j]) < best[3])
                        best = {i, j, dist(camps[i], camps[j])}
        joined[best[2]] = true
        append routes, {best[1], best[2]}
    if #camps >= 4
        append routes, {1, #camps}
    return routes

walk = (map, rng, area, start, fields) ->
    fields.map, fields.rng, fields.area, fields.from_xy = map, rng, area, start
    fields.passable or= NOT_ROCK
    return SourceMap.random_walk_apply(fields)

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area

    camps = found_camps(rng, area, rng\random(6, 9), 26)
    if #camps < 5
        return false
    -- Each camp begins as a small clearing
    for camp in *camps
        walk(map, rng, area, camp, {steps: 60, width: 3, operator: CHOP, toward_xy: camp, toward_bias: 0.35})

    routes = trade_routes(camps)
    -- Quarries: two camps cut into rock a short way out
    for i=1,2
        {cx, cy} = camps[i]
        face = {cx + rng\random(-8, 9), cy + rng\random(-8, 9)}
        walk(map, rng, area, face, {steps: 45, width: 2, operator: paint(TILES.stone, {matches_content: TILES.trees.id}), passable: {}})

    for year=1,YEARS
        -- Woodcutters head out from every camp, a little further each year
        for camp in *camps
            for _=1,2
                reach = 6 + year
                target = {camp[1] + rng\random(-reach, reach + 1), camp[2] + rng\random(-reach, reach + 1)}
                walk(map, rng, area, camp, {steps: 25 + 2 * year, width: 1, operator: CHOP, toward_xy: target, toward_bias: 0.55})
        -- One trade route is worn in every other year, until all are
        if year % 2 == 0 and #routes > 0
            {a, b} = table.remove(routes, 1)
            walk(map, rng, area, camps[a], {steps: 2000, width: 2, operator: TRAIL, toward_xy: camps[b], toward_bias: 0.7})
        -- Worked ground spreads: trees hemmed in by clearing get felled
        SourceMap.automaton_apply {:map, :rng, :area, rules: {
            paint(TILES.grass, {matches_content: TILES.trees.id, neighbours: WALKABLE, min_neighbours: 5, chance: 0.3})
        }}
        -- The forest reclaims clearings nobody works (trails stay trodden)
        SourceMap.automaton_apply {:map, :rng, :area, rules: {
            paint(TILES.trees, {matches_content: TILES.grass.id, neighbours: {matches_all: {SOLID}}, min_neighbours: 6, chance: 0.25})
        }}
    -- Any trade routes left are finished off
    for {a, b} in *routes
        walk(map, rng, area, camps[a], {steps: 2000, width: 2, operator: TRAIL, toward_xy: camps[b], toward_bias: 0.7})

    -- Settled ground: long-worked meadows grow lush, lone trees in the open are cleared
    SourceMap.automaton_apply {:map, :rng, :area, rules: {
        paint(TILES.grass_lush, {matches_content: TILES.grass.id, neighbours: WALKABLE, min_neighbours: 8, chance: 0.5})
        paint(TILES.grass, {matches_content: TILES.trees.id, neighbours: {matches_all: {SOLID}}, max_neighbours: 1})
    }}

    if not connect_walkable(map, area, 0.1)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    -- The dungeons were dug near the settlements
    map.dungeon_regions = for {cx, cy} in *camps
        from_bbox(math.max(x1, cx - 16), math.max(y1, cy - 16), math.min(x2, cx + 16), math.min(y2, cy + 16))
    return true

return create(layout)
