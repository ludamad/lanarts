----
-- Overworld variant "craters": an impact field.
-- A big crater near the middle and a handful of smaller ones around it, each a lopsided basin
-- of dry grass and dirt behind a ragged rock rim. Every rim is breached in a place or two so the
-- basin can be entered. Between the craters is scrubland strewn with rock debris and the odd
-- tree, and away from the craters the scrub gives way to forest.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :layout_area, :forest_rim, :connect_walkable, :grid_regions, :create, :protect} = require "map_descs.OverworldCommon"
{:from_bbox} = require "maps.MapRegion"

-- How far the scrubland reaches past the nearest crater's edge, in squares (plus noise)
SCRUB_REACH = 5
DEBRIS_CHANCE = 0.025

-- A crater: a tilted ellipse whose edge wobbles with the angle, and a rim whose thickness does too
make_crater = (rng, cx, cy, radius) ->
    stretch = rng\randomf(0.8, 1.2)
    breaches = for _=1,rng\random(1, 3)
        rng\randomf(0, 2 * math.pi)
    return {
        :cx, :cy, :radius, :breaches
        rx: radius * stretch, ry: radius / stretch, tilt: rng\randomf(0, math.pi)
        -- Wobble of the edge and of the rim's thickness, as phases of a few sine terms
        p1: rng\randomf(0, 2 * math.pi), p2: rng\randomf(0, 2 * math.pi), p3: rng\randomf(0, 2 * math.pi)
    }

-- Distance in squares outside the crater's floor (negative inside), and the angle around it
crater_distance = (c, x, y) ->
    dx, dy = x + 0.5 - c.cx, y + 0.5 - c.cy
    cos, sin = math.cos(c.tilt), math.sin(c.tilt)
    u, v = dx * cos + dy * sin, -dx * sin + dy * cos
    a = math.atan2(v, u)
    r = math.sqrt((u / c.rx)^2 + (v / c.ry)^2)
    r /= 1 + 0.08 * math.sin(3 * a + c.p1) + 0.05 * math.sin(5 * a + c.p2)
    return (r - 1) * c.radius, a

-- Is the angle within a breach of the rim? Breaches are about 3-4 squares across whatever the size
in_breach = (c, a) ->
    half = 2.2 / c.radius
    for b in *c.breaches
        diff = math.abs((a - b + math.pi) % (2 * math.pi) - math.pi)
        if diff < half
            return true
    return false

-- Craters spread over the valley without overlapping much, the biggest near the middle
place_craters = (rng, area) ->
    {x1, y1, x2, y2} = area
    mx, my = (x1 + x2) / 2, (y1 + y2) / 2
    craters = {make_crater(rng, mx + rng\random(-6, 7), my + rng\random(-6, 7), rng\random(13, 17))}
    target = rng\random(4, 8)   -- 4-7 craters
    for attempt=1,300
        if #craters >= target
            break
        radius = rng\random(5, 10)
        pad = radius + 8
        x, y = rng\random(x1 + pad, x2 - pad), rng\random(y1 + pad, y2 - pad)
        ok = true
        for c in *craters
            -- Rims may touch, basins may not
            if (c.cx - x)^2 + (c.cy - y)^2 < ((c.radius + radius) * 1.05 + 2)^2
                ok = false
                break
        -- Keep the field compact: close enough to some crater to share its scrubland
        near = false
        for c in *craters
            if (c.cx - x)^2 + (c.cy - y)^2 < ((c.radius + radius) + 20)^2
                near = true
        if ok and near
            append craters, make_crater(rng, x, y, radius)
    return craters

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area
    seed = () -> rng\random(0, 2^30)

    craters = place_craters(rng, area)
    if #craters < 4
        return false
    reach = SourceMap.noise_grid {:area, seed: seed(), scale: 12, octaves: 3}
    soil = SourceMap.noise_grid {:area, seed: seed(), scale: 9, octaves: 2}
    copses = SourceMap.noise_grid {:area, seed: seed(), scale: 3.5, octaves: 2}

    -- Each square belongs to the crater whose floor it is nearest (or inside); craters are drawn
    -- after the scrub and the forest margin, so the forest never cuts through a rim
    craters_pass, scrub_pass = {}, {}
    for y=y1,y2-1
        for x=x1,x2-1
            gx, gy = x - x1 + 1, y - y1 + 1
            best, best_a, crater = math.huge, 0, craters[1]
            for c in *craters
                d, a = crater_distance(c, x, y)
                if d < best
                    best, best_a, crater = d, a, c
            s = soil[gy][gx]
            if best < 0
                -- The basin floor: dry grass, bare dirt on its lower slopes, a rocky peak in big ones
                tile = if crater.radius >= 13 and best < -crater.radius * 0.88
                    TILES.stone
                elseif best > -1.5 or s < -0.2
                    TILES.dirt
                else
                    TILES.grass_dry
                append craters_pass, {x, y, tile}
            elseif best < 2.2 + 0.7 * math.sin(2 * best_a + crater.p3) + rng\randomf(0, 0.8)
                -- The rim, broken where it was breached
                append craters_pass, {x, y, if in_breach(crater, best_a) then TILES.dirt else TILES.stone}
            elseif best < SCRUB_REACH + 7 * (reach[gy][gx] + 0.3)
                -- Scrubland between the craters, with ejected rock close to the rims
                debris = if best < 4 then 0.12 else DEBRIS_CHANCE
                tile = if rng\randomf() < debris
                    TILES.stone
                elseif copses[gy][gx] > 0.62
                    TILES.trees
                elseif s < -0.45
                    TILES.dirt
                elseif s < 0
                    TILES.grass_dry
                elseif s < 0.4
                    TILES.grass
                else
                    TILES.grass_lush
                append scrub_pass, {x, y, tile}

    for {x, y, tile} in *scrub_pass
        map\square_apply({x, y}, paint(tile))
    forest_rim(map, rng, area, 0.3, -0.45)
    for {x, y, tile} in *craters_pass
        map\square_apply({x, y}, paint(tile))
    -- No vault is built into a crater's basin or rim
    protect(map, [{x, y} for {x, y} in *craters_pass])
    -- Fill pinholes: open squares walled in on nearly every side become forest
    SourceMap.automaton_apply {:map, :rng, :area, rules: {
        paint(TILES.trees, {matches_none: {SourceMap.FLAG_SOLID}, neighbours: {matches_all: {SourceMap.FLAG_SOLID}}, min_neighbours: 7})
    }}
    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions(area, 2, 2, 2)
    -- Dungeon entrances are dug near the craters
    map.dungeon_regions = for c in *craters
        r = c.radius + 10
        from_bbox(math.max(x1, math.floor(c.cx - r)), math.max(y1, math.floor(c.cy - r)), math.min(x2, math.floor(c.cx + r)), math.min(y2, math.floor(c.cy + r)))
    return true

return create(layout, "craters")
