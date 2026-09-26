----
-- Overworld variant "rootways": meadows that run out from the heart of the valley in branching,
-- root-like runs. A few roots head outward from a central clearing; each run wanders towards its
-- heading (a biased random walk) and then forks into two shorter runs splaying apart, generation
-- after generation. Small glades open at the forks, and an automaton pass widens the runs so they
-- can be walked.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :WALKABLE, :layout_area, :forest_rim, :connect_walkable, :grid_regions, :create, :protect_matching} = require "map_descs.OverworldCommon"
{:skewed_ellipse_points} = require "maps.GenerateUtils"

GENERATIONS = 5
SPLAY = 0.55            -- radians between a run's heading and each fork's

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    cx, cy = math.floor((x1 + x2) / 2), math.floor((y1 + y2) / 2)
    inside = (x, y) -> math.max(x1 + 4, math.min(x2 - 5, math.floor(x))), math.max(y1 + 4, math.min(y2 - 5, math.floor(y)))

    glade = (x, y, size) ->
        SourceMap.polygon_apply {:map, operator: paint(TILES.grass_lush),
            points: skewed_ellipse_points(rng, {x, y}, {size, size}, 10)}

    -- Grows one run from (x, y) along heading, then forks
    grow = (x, y, heading, length, generation) ->
        tx, ty = inside(x + math.cos(heading) * length, y + math.sin(heading) * length)
        {ex, ey} = SourceMap.random_walk_apply {
            :map, :rng, :area, from_xy: {x, y}, toward_xy: {tx, ty}, toward_bias: 0.7
            steps: math.floor(length * 3), width: if generation <= 4 then 2 else 1
            operator: paint(if generation == 1 then TILES.dirt else TILES.grass)
        }
        if generation >= GENERATIONS
            return
        if rng\random(0, 2) == 0
            glade(ex, ey, rng\random(6, 11))
        for side in *{-1, 1}
            grow(ex, ey, heading + side * SPLAY + rng\randomf(-0.2, 0.2), length * 0.72, generation + 1)

    glade(cx, cy, 12)
    -- The heart the roots grow from stays clear of vaults
    protect_matching(map, {cx - 7, cy - 7, cx + 8, cy + 8}, {})
    n_roots = rng\random(3, 5)
    start = rng\randomf(0, 2 * math.pi)
    for i=1,n_roots
        heading = start + (i - 1) * 2 * math.pi / n_roots + rng\randomf(-0.3, 0.3)
        grow(cx, cy, heading, rng\random(18, 24), 1)

    -- Widen the runs just enough to walk comfortably
    SourceMap.automaton_apply {:map, :rng, :area, iterations: 2, rules: {
        paint(TILES.grass, {matches_content: TILES.trees.id, neighbours: WALKABLE, min_neighbours: 3, chance: 0.85})
    }}

    forest_rim(map, rng, area, 0.45, -0.6)
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    return true

return create(layout)
