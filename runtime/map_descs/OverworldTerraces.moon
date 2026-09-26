----
-- Overworld variant "terraces": stepped highlands.
-- A noise field is cut into height levels: forest in the lowlands, then lush, plain and dry
-- terraces climbing towards the heights. Where a terrace rises above its neighbour the edge is a
-- rock cliff, broken here and there by ramps. Terraces that still can't be reached get a path.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :layout_area, :forest_rim, :connect_walkable, :grid_regions, :create} = require "map_descs.OverworldCommon"

-- Height levels, lowest first; level 1 stays forest
LEVEL_TILES = {false, TILES.grass_lush, TILES.grass, TILES.grass_dry, TILES.dirt}
RAMP_CHANCE = 0.07

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    heights = SourceMap.noise_grid {:area, seed: rng\random(0, 2^30), scale: 30, octaves: 3, persistence: 0.45, falloff: 0.2}
    n = #LEVEL_TILES
    level = (x, y) ->
        v = heights[y - y1 + 1][x - x1 + 1]
        -- Below -0.42 is lowland forest; the rest climbs through the terraces
        return math.max(1, math.min(n, math.floor((v + 0.42) / 1.15 * (n - 1)) + 2))

    for y=y1,y2-1
        for x=x1,x2-1
            l = level(x, y)
            if LEVEL_TILES[l]
                -- A cliff where this terrace stands above a walkable neighbour
                cliff = false
                for {dx, dy} in *{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
                    nx, ny = x + dx, y + dy
                    if nx >= x1 and ny >= y1 and nx < x2 and ny < y2
                        nl = level(nx, ny)
                        if nl > 1 and nl < l
                            cliff = true
                if cliff and rng\randomf() >= RAMP_CHANCE
                    map\square_apply({x, y}, paint(TILES.stone))
                else
                    map\square_apply({x, y}, paint(LEVEL_TILES[l]))

    forest_rim(map, rng, area, 0.5, -0.35)
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    return true

return create(layout)
