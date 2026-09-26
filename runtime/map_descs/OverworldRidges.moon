----
-- Overworld variant "ridges": long parallel ridges running across the valley.
-- A banded field, sin(y * frequency + noise), draws rock ridges running roughly east to west; the
-- noise makes them wander. Forest clothes the slopes either side, grass fills the valleys between,
-- and a few passes break through every ridge so the valleys join up north to south.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :layout_area, :forest_rim, :connect_walkable, :grid_regions, :create} = require "map_descs.OverworldCommon"

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    wobble = SourceMap.noise_grid {:area, seed: rng\random(0, 2^30), scale: 34, octaves: 2}
    lushness = SourceMap.noise_grid {:area, seed: rng\random(0, 2^30), scale: 12, octaves: 2}
    frequency = 2 * math.pi / rng\random(20, 26)         -- one ridge every 20-25 squares
    phase = rng\randomf(0, 2 * math.pi)

    for y=y1,y2-1
        for x=x1,x2-1
            band = math.sin((y - y1) * frequency + phase + wobble[y - y1 + 1][x - x1 + 1] * 3.2)
            tile = if band > 0.72
                TILES.stone                               -- ridge crest
            elseif band > 0.25
                TILES.trees                               -- wooded slopes
            elseif lushness[y - y1 + 1][x - x1 + 1] > 0.3
                TILES.grass_lush
            elseif lushness[y - y1 + 1][x - x1 + 1] < -0.4
                TILES.grass_dry
            else
                TILES.grass
            map\square_apply({x, y}, paint(tile))

    -- Passes: a few places along the valley floors where a path climbs straight over the ridge
    for i=1,rng\random(7, 11)
        px = rng\random(x1 + 8, x2 - 8)
        py = rng\random(y1 + 6, y2 - 6)
        SourceMap.random_walk_apply {
            :map, :rng, :area, from_xy: {px, py}, toward_xy: {px + rng\random(-4, 5), py + (if rng\random(2) == 0 then -24 else 24)}
            toward_bias: 0.8, steps: 60, width: 2
            operator: paint(TILES.dirt, {matches_all: {SourceMap.FLAG_SOLID}})
        }

    forest_rim(map, rng, area, 0.5, -0.4)
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    return true

return create(layout, "classic")
