----
-- Overworld variant "ruins": an overgrown ruined town.
-- A clearing in the forest holds a rough grid of dirt streets, unevenly spaced and jogging a
-- little between crossings. The blocks between them hold the shells of buildings: rock walls with
-- doorways and collapsed stretches around floors of dirt and dry grass, with an empty yard here
-- and there. The forest is taking the town back: stands of trees grow through blocks and
-- streets, and whole streets have gone to grass.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :WALKABLE, :layout_area, :forest_rim, :connect_walkable, :grid_regions, :create} = require "map_descs.OverworldCommon"

STREET_WIDTH = 2
MIN_BLOCK = 11
MAX_BLOCK = 17
OVERGROWN_STREET_CHANCE = 0.2   -- a street segment gone to grass
COLLAPSE_CHANCE = 0.07          -- per wall square, a collapsed stretch starts here
YARD_CHANCE = 0.2               -- a block with no building left standing

-- Street positions across [lo, hi): unevenly spaced lines, each a street's first square
street_lines = (rng, lo, hi) ->
    lines = {}
    at = lo + rng\random(0, 4)
    while at + STREET_WIDTH <= hi
        append lines, at
        at += STREET_WIDTH + rng\random(MIN_BLOCK, MAX_BLOCK + 1)
    return lines

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    seed = () -> rng\random(0, 2^30)
    open = (x, y) -> map\square_query({x, y}, WALKABLE)
    set = (x, y, tile) -> map\square_apply({x, y}, paint(tile))
    rect = (a, b, c, d, tile) ->
        SourceMap.rectangle_apply {:map, area: {a, b, c, d}, fill_operator: paint(tile, {matches_none: {SourceMap.FLAG_SOLID}}), create_subgroup: false}

    -- The clearing: a lumpy ellipse around the town centre
    cx, cy = (x1 + x2) / 2 + rng\random(-4, 5), (y1 + y2) / 2 + rng\random(-4, 5)
    rx, ry = rng\random(38, 43), rng\random(34, 39)
    edge = SourceMap.noise_grid {:area, seed: seed(), scale: 14, octaves: 3}
    ground = SourceMap.noise_grid {:area, seed: seed(), scale: 8, octaves: 2}
    for y=y1,y2-1
        for x=x1,x2-1
            gx, gy = x - x1 + 1, y - y1 + 1
            d = math.sqrt(((x - cx) / rx)^2 + ((y - cy) / ry)^2)
            if d + 0.25 * edge[gy][gx] < 1
                g = ground[gy][gx]
                set(x, y, if g < -0.25 then TILES.grass_dry else TILES.grass)

    -- The street grid over the town's bounding box. Streets only pave open ground, so they fade
    -- out where the clearing ends. Each segment between crossings may jog a square aside.
    bx1, by1 = math.floor(cx - rx), math.floor(cy - ry)
    bx2, by2 = math.floor(cx + rx), math.floor(cy + ry)
    xs, ys = street_lines(rng, bx1, bx2), street_lines(rng, by1, by2)
    street = (a, b, c, d) ->
        tile = if rng\randomf() < OVERGROWN_STREET_CHANCE then TILES.grass_lush else TILES.dirt
        rect(a, b, c, d, tile)
    ybounds = {by1, unpack(ys)}
    append ybounds, by2
    for sx in *xs
        for j=1,#ybounds-1
            jog = rng\random(-1, 2)
            street(sx + jog, ybounds[j], sx + jog + STREET_WIDTH, ybounds[j + 1] + STREET_WIDTH)
    xbounds = {bx1, unpack(xs)}
    append xbounds, bx2
    for sy in *ys
        for i=1,#xbounds-1
            jog = rng\random(-1, 2)
            street(xbounds[i], sy + jog, xbounds[i + 1] + STREET_WIDTH, sy + jog + STREET_WIDTH)

    -- A ruined building: a floor inside rock walls, with a doorway and collapsed stretches
    building = (a, b, c, d) ->
        floor = if rng\random(0, 2) == 0 then TILES.dirt else TILES.grass_dry
        rect(a + 1, b + 1, c - 1, d - 1, floor)
        -- The walls in order around the perimeter, so collapses run along them
        walls = {}
        for x=a,c-1 do append walls, {x, b}
        for y=b+1,d-1 do append walls, {c - 1, y}
        for x=c-2,a,-1 do append walls, {x, d - 1}
        for y=d-2,b+1,-1 do append walls, {a, y}
        gap = {}
        -- The doorway, two squares somewhere along a side (not a corner)
        door = rng\random(1, #walls + 1)
        gap[door], gap[door % #walls + 1] = true, true
        i = 1
        while i <= #walls
            if rng\randomf() < COLLAPSE_CHANCE
                for k=i,math.min(#walls, i + rng\random(1, 4))
                    gap[k] = true
                i += 4
            i += 1
        for k, {x, y} in ipairs walls
            if open(x, y)
                set(x, y, if gap[k] then floor else TILES.stone)
        -- Rubble fallen inside
        for _=1,rng\random(0, 3)
            x, y = rng\random(a + 1, c - 1), rng\random(b + 1, d - 1)
            if open(x, y)
                set(x, y, TILES.stone)

    -- Each block holds a building or two, set back from the street, or is left as a yard
    for i=1,#xbounds-1
        for j=1,#ybounds-1
            a = xbounds[i] + (if i > 1 then STREET_WIDTH + 1 else 0)
            b = ybounds[j] + (if j > 1 then STREET_WIDTH + 1 else 0)
            c, d = xbounds[i + 1] - 1, ybounds[j + 1] - 1
            if c - a < 5 or d - b < 5 or rng\randomf() < YARD_CHANCE
                continue
            -- Wide blocks split into two houses side by side
            if c - a >= 14
                mid = math.floor((a + c) / 2) + rng\random(-1, 2)
                building(a + rng\random(0, 2), b + rng\random(0, 2), mid - 1, d - rng\random(0, 2))
                building(mid + 1, b + rng\random(0, 2), c - rng\random(0, 2), d - rng\random(0, 2))
            elseif d - b >= 14
                mid = math.floor((b + d) / 2) + rng\random(-1, 2)
                building(a + rng\random(0, 2), b + rng\random(0, 2), c - rng\random(0, 2), mid - 1)
                building(a + rng\random(0, 2), mid + 1, c - rng\random(0, 2), d - rng\random(0, 2))
            else
                building(a + rng\random(0, 2), b + rng\random(0, 2), c - rng\random(0, 2), d - rng\random(0, 2))

    -- The forest grows back: stands of trees over ground, streets and floors, the thickest of
    -- them swallowing walls too, and lush grass creeping over the streets around them
    SourceMap.noise_apply {:map, :area, seed: seed(), scale: 9, octaves: 3, bands: {
        {max: 0.1}
        paint(TILES.grass_lush, {max: 0.3, matches_content: TILES.dirt.id})
        {max: 0.3}
        paint(TILES.trees, {max: 0.55, matches_none: {SourceMap.FLAG_SOLID}})
        paint(TILES.trees, {max: 1})
    }}
    -- Saplings scattered through the town
    SourceMap.noise_apply {:map, :area, seed: seed(), scale: 2.5, octaves: 2, bands: {
        {max: 0.7}
        paint(TILES.trees, {max: 1, matches_none: {SourceMap.FLAG_SOLID}})
    }}

    forest_rim(map, rng, area, 0.3, -0.45)
    -- Fill pinholes: open squares walled in on nearly every side become forest
    SourceMap.automaton_apply {:map, :rng, :area, rules: {
        paint(TILES.trees, {matches_none: {SourceMap.FLAG_SOLID}, neighbours: {matches_all: {SourceMap.FLAG_SOLID}}, min_neighbours: 7})
    }}
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    return true

return create(layout, "crossroads")
