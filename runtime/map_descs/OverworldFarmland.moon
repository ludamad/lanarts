----
-- Overworld variant "farmland": a patchwork of fields.
-- The valley is split again and again into plots (a binary space partition). The first splits
-- become dirt lanes, later ones hedgerows of trees with gaps for gates. Plots are sown with
-- different grasses, planted as orchards, or left as woodland.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :layout_area, :connect_walkable, :create} = require "map_descs.OverworldCommon"
{:from_bbox} = require "maps.MapRegion"

LANE_DEPTH = 2         -- splits this shallow become lanes rather than hedges
MIN_PLOT = 12

rect = (map, x1, y1, x2, y2, tile) ->
    SourceMap.rectangle_apply {:map, area: {x1, y1, x2, y2}, fill_operator: paint(tile), create_subgroup: false}

-- Splits {x1, y1, x2, y2} into plots, calling divider(x1, y1, x2, y2, depth) for each split line
partition = (rng, area, depth, plots, divider) ->
    {x1, y1, x2, y2} = area
    w, h = x2 - x1, y2 - y1
    can_x, can_y = w >= MIN_PLOT * 2 + 2, h >= MIN_PLOT * 2 + 2
    -- Field-sized plots sometimes stay whole, so field sizes vary
    if (not can_x and not can_y) or (math.max(w, h) <= 30 and rng\random(0, 3) == 0)
        append plots, area
        return
    along_x = if can_x and can_y then w > h or (w == h and rng\random(0, 2) == 0) else can_x
    if along_x
        cut = rng\random(x1 + MIN_PLOT, x2 - MIN_PLOT - 1)
        partition(rng, {x1, y1, cut, y2}, depth + 1, plots, divider)
        partition(rng, {cut + 2, y1, x2, y2}, depth + 1, plots, divider)
        divider(cut, y1, cut + 2, y2, depth)
    else
        cut = rng\random(y1 + MIN_PLOT, y2 - MIN_PLOT - 1)
        partition(rng, {x1, y1, x2, cut}, depth + 1, plots, divider)
        partition(rng, {x1, cut + 2, x2, y2}, depth + 1, plots, divider)
        divider(x1, cut, x2, cut + 2, depth)

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    -- Farm the middle of the valley, with forest beyond
    margin = math.floor((x2 - x1) * 0.2)
    inner = {x1 + margin, y1 + margin, x2 - margin, y2 - margin}

    plots, lanes, hedges = {}, {}, {}
    partition rng, inner, 0, plots, (a, b, c, d, depth) ->
        append (if depth < LANE_DEPTH then lanes else hedges), {a, b, c, d}

    crops = {TILES.grass, TILES.grass_lush, TILES.grass_dry}
    regions = {}
    for plot in *plots
        {px1, py1, px2, py2} = plot
        kind = rng\random(0, 10)
        if kind < 3
            continue                                       -- woodland: leave the trees
        crop = crops[rng\random(1, #crops + 1)]
        rect(map, px1, py1, px2, py2, crop)
        if kind == 3
            -- Orchard: trees in rows
            for y=py1+1,py2-2,3
                for x=px1+1,px2-2,3
                    map\square_apply({x, y}, paint(TILES.trees))
        r = from_bbox(px1, py1, px2, py2)
        r.conf = {n_statues: 0}
        append regions, r
    -- Hedgerows between plots, with two gates in each
    for {a, b, c, d} in *hedges
        rect(map, a, b, c, d, TILES.trees)
        for _=1,2
            if c - a > d - b
                gx = rng\random(a, math.max(a + 1, c - 2))
                rect(map, gx, b, math.min(c, gx + 2), d, TILES.dirt)
            else
                gy = rng\random(b, math.max(b + 1, d - 2))
                rect(map, a, gy, c, math.min(d, gy + 2), TILES.dirt)
    -- Lanes run the full length of the first splits
    for {a, b, c, d} in *lanes
        rect(map, a, b, c, d, TILES.dirt)

    if not connect_walkable(map, area, 0.1, rng)
        return false
    -- A statue or two at crossroads-sized plots
    for i=1,math.min(4, #regions)
        regions[i].conf.n_statues = 1
    map.regions = regions
    return true

return create(layout)
