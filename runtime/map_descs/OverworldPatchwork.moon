----
-- Overworld variant "patchwork": organic meadows between hedgerows.
-- Seed points are scattered over the middle of the valley and every square joins its nearest
-- seed (a Voronoi diagram). Each cell becomes a meadow of one grass, or now and then stays
-- woodland; squares near the border between two cells grow into a hedgerow of trees, with a gate
-- or two between neighbouring meadows. Squares far from every seed stay forest, and a ragged rim
-- blends the meadows into the woods.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :layout_area, :forest_rim, :connect_walkable, :grid_regions, :create} = require "map_descs.OverworldCommon"

MIN_SEED_DIST = 13     -- seeds closer than this are rejected, so cells are field-sized
SPREAD = 33            -- seeds lie within this radius of the valley's centre
REACH = 14             -- squares further than this from every seed stay forest
HEDGE = 2.0            -- hedge where (second-nearest - nearest seed distance) is below this
WOODLAND_CHANCE = 0.12

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    cx, cy = (x1 + x2) / 2, (y1 + y2) / 2

    -- Seeds by rejection sampling: random points in a disc, kept if far enough from the others
    seeds = {}
    n_target = rng\random(12, 21)
    for attempt=1,600
        if #seeds >= n_target
            break
        -- The first seed sits near the centre; the rest spread uniformly over the disc
        a, r = rng\randomf(0, 2 * math.pi), if #seeds == 0 then rng\randomf(0, 4) else SPREAD * math.sqrt(rng\randomf(0, 1))
        sx, sy = cx + math.cos(a) * r, cy + math.sin(a) * r
        ok = true
        for s in *seeds
            if (s.x - sx)^2 + (s.y - sy)^2 < MIN_SEED_DIST^2
                ok = false
                break
        if ok
            crops = {TILES.grass, TILES.grass_lush, TILES.grass_dry}
            tile = if rng\randomf() < WOODLAND_CHANCE then false else crops[rng\random(1, #crops + 1)]
            append seeds, {x: sx, y: sy, :tile}
    -- The centre cell is always a meadow, so the heart of the valley is open
    seeds[1].tile or= TILES.grass

    borders = {}   -- hedge squares between two meadows, by pair of seeds
    -- Nearest and second-nearest seed of every square
    for y=y1,y2-1
        for x=x1,x2-1
            i1, d1, i2, d2 = nil, math.huge, nil, math.huge
            for i, s in ipairs seeds
                d = math.sqrt((s.x - x)^2 + (s.y - y)^2)
                if d < d1
                    i2, d2 = i1, d1
                    i1, d1 = i, d
                elseif d < d2
                    i2, d2 = i, d
            if d1 > REACH
                continue
            tile = seeds[i1].tile
            if not tile
                continue                               -- woodland cell: leave the trees
            if d2 - d1 < HEDGE
                -- Remember hedge squares between two meadows as gate candidates
                if seeds[i2].tile and d2 <= REACH
                    key = math.min(i1, i2) * 100 + math.max(i1, i2)
                    borders[key] or= {}
                    append borders[key], {x, y}
                continue                               -- hedgerow: leave the trees
            map\square_apply({x, y}, paint(tile))

    -- Gates: one (two on long borders) 3-wide opening per hedgerow between meadows
    -- (keys sorted so the order, and so the rng draws, don't depend on table iteration)
    keys = [key for key in pairs borders]
    table.sort keys
    for key in *keys
        squares = borders[key]
        for _=1,(if #squares > 30 then 2 else 1)
            {gx, gy} = squares[rng\random(1, #squares + 1)]
            SourceMap.rectangle_apply {:map, area: {gx - 1, gy - 1, gx + 2, gy + 2}, fill_operator: paint(TILES.dirt), create_subgroup: false}

    forest_rim(map, rng, area, 0.5, -0.4)
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    return true

return create(layout, "patchwork")
