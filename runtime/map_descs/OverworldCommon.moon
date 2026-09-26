----
-- Shared by every overworld variant (see World.moon): tiles, the features, vaults, items and
-- animals every overworld gets, and create(), which wraps a variant's layout into a MapDesc.
----

{:Tile, :tile_operator} = require "maps.GenerateUtils"

NewMaps = require "maps.NewMaps"
Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"

Vaults = require "maps.Vaults"
VaultKit = require "maps.vaults.Kit"
VaultLibrary = require "maps.vaults.Library"
{:place_vault_in} = require "maps.VaultUtils"
SourceMap = require "core.SourceMap"
OldMaps = require "maps.OldMaps"
{:from_bbox} = require "maps.MapRegion"

{:MapDesc, :MapNode} = require "maps.MapDesc"

{   :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_NO_ENEMY_SPAWN
} = Vaults

create_overworld_scheme = (tileset) -> nilprotect {
    floor1: Tile.create(tileset.floor, false, true, {FLAG_OVERWORLD})
    floor2: Tile.create(tileset.floor_alt, false, true, {FLAG_OVERWORLD})
    wall1: Tile.create(tileset.wall, true, true, {FLAG_OVERWORLD})
    wall2: Tile.create(tileset.wall_alt, true, false)
}

OVERWORLD_TILESET = create_overworld_scheme(Tilesets.grass)

place_doors_and_statues = (region_set) ->
    {:map} = region_set

    -- Never on vault squares: a plain door there would open the way past a locked vault door
    for xy in *SourceMap.rectangle_match {:map, selector: {matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT}, matches_all: {FLAG_DOOR_CANDIDATE}}}
        MapUtils.spawn_door(map, xy)
        map\square_apply(xy, {add: SourceMap.FLAG_HAS_OBJECT})
    -- Use the generated regions (map.regions), they carry the subtemplate conf
    for region in *map.regions
        area = region\bbox()
        for i=1,region.conf.n_statues
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT}, remove: SourceMap.FLAG_SEETHROUGH})
            MapUtils.spawn_decoration(map, OldMaps.statue, sqr, random(0,17))
    return true

overworld_items_and_enemies = (region_set) ->
    {:map, :regions} = region_set

    for region in *regions
        area = region\bbox()
        for i=1,OldMaps.adjusted_item_amount(10) do
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, Vaults.FLAG_NO_ITEM_SPAWN}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
            item = ItemUtils.item_generate ItemGroups.basic_items
            MapUtils.spawn_item(map, item.type, item.amount, sqr)

        -- Only a few wandering animals; the vaults hold most of the overworld's enemies
        OldMaps.generate_from_enemy_entries(map, OldMaps.medium_animals, OldMaps.adjusted_enemy_amount(2), area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
    return true

-----------------------------
-- The overworld's vaults, by kind: a trader, treasure, guarded spots and landmarks
FEATURE_SLOTS = {"store", "treasure", "treasure", "treasure", "encounter", "encounter", "landmark", "landmark", "landmark", "landmark", "landmark"}

-- Places the overworld's vaults, drawn from the theme's own (see maps/vaults) and the shared ones.
-- A vault that won't fit gives way to another of its kind; the slot stays empty if none will.
overworld_features = (region_set, theme = "classic") ->
    {:rng} = region_set.map
    used = {}
    for kind in *FEATURE_SLOTS
        for attempt=1,3
            def = VaultLibrary.pick(rng, kind, theme, used)
            if not def
                break
            used[def.name] = true
            if VaultKit.place(region_set, def)
                break
    return true


-- The overworld's surfaces, for variants that paint the map directly
TILES = nilprotect {
    grass: Tile.create(Tilesets.grass.floor, false, true, {FLAG_OVERWORLD})
    grass_lush: Tile.create(Tilesets.grass.floor_alt1, false, true, {FLAG_OVERWORLD})
    grass_dry: Tile.create(Tilesets.grass.floor_alt2, false, true, {FLAG_OVERWORLD})
    dirt: Tile.create(Tilesets.grass.dirt, false, true, {FLAG_OVERWORLD})
    trees: Tile.create(Tilesets.grass.wall, true, true, {FLAG_OVERWORLD})
    stone: Tile.create(Tilesets.grass.wall_alt, true, false)
}

-- An operator (with any extra fields, eg a selector or noise band) that lays down a tile
paint = (tile, fields = {}) -> tile_operator(tile, fields)

WALKABLE = {matches_none: {SourceMap.FLAG_SOLID}}

-- Walkable squares in the classic overworld: about 2400 on average, from about 1750 (measured over
-- 16 seeds with engine.StartMapGenCheck --dump). Variants stay between the classic minimum and twice
-- the classic average, so no overworld is drastically roomier than another.
WALKABLE_RANGE = {1700, 4800}

-- Hands the outskirts of area back to forest along a ragged line, so a layout fades into the
-- woods instead of stopping at the straight edge of its area. Higher strength eats further in.
forest_rim = (map, rng, area, strength = 0.5, cut = -0.4) ->
    SourceMap.noise_apply {:map, :area, seed: rng\random(0, 2^30), scale: 10, octaves: 3, falloff: strength, bands: {
        paint(TILES.trees, {max: cut})
    }}

-- The area a variant lays out, inside the map's padding
layout_area = (node) ->
    {x1, y1, x2, y2} = node.region_set.regions[1]\bbox()
    return {x1, y1, x2, y2}

-- Marks squares as a layout's showpiece: no vault (the shared features or a dungeon entrance) is
-- placed with its footprint over them (see VaultUtils). Doors, statues, items and animals can
-- still go there.
protect = (map, squares) ->
    map.protected or= {}
    for {x, y} in *squares
        map.protected[y * 4096 + x] = true

-- Protects every square of area ({x1, y1, x2, y2}, exclusive end) matching selector
protect_matching = (map, area, selector) ->
    {x1, y1, x2, y2} = area
    protect(map, SourceMap.rectangle_match {:map, area: {x1, y1, x2, y2}, :selector})

-- The walkable squares of area grouped into 4-connected components, largest first
walkable_components = (map, area) ->
    {x1, y1, x2, y2} = area
    seen = {}
    key = (x, y) -> y * 4096 + x
    components = {}
    for y=y1,y2-1
        for x=x1,x2-1
            if not seen[key(x, y)] and map\square_query({x, y}, WALKABLE)
                component = {}
                stack = {{x, y}}
                seen[key(x, y)] = true
                while #stack > 0
                    xy = table.remove(stack)
                    append component, xy
                    for {dx, dy} in *{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
                        nx, ny = xy[1] + dx, xy[2] + dy
                        if nx >= x1 and ny >= y1 and nx < x2 and ny < y2 and not seen[key(nx, ny)]
                            if map\square_query({nx, ny}, WALKABLE)
                                seen[key(nx, ny)] = true
                                append stack, {nx, ny}
                append components, component
    table.sort components, (a, b) -> #a > #b
    return components

-- After vaults, statues and the like are placed they can cut the ground into pockets. Carves a
-- winding path from every pocket to the main area, through forest or rock only (never a vault or
-- an object). Returns false if some pocket couldn't be reached.
reconnect_walkable = (map, rng, area) ->
    for attempt=1,3
        components = walkable_components(map, area)
        if #components <= 1
            return true
        main = components[1]
        for i=2,#components
            -- Walk from the pocket's side that faces the main area to the nearest main square
            pocket = components[i]
            start, best, best_d = nil, nil, math.huge
            for k=1,#pocket,math.max(1, math.floor(#pocket / 40))
                for j=1,#main,math.max(1, math.floor(#main / 400))
                    a, b = pocket[k], main[j]
                    d = (a[1] - b[1])^2 + (a[2] - b[2])^2
                    if d < best_d
                        start, best, best_d = a, b, d
            SourceMap.random_walk_apply {
                -- A modest pull towards the target, so the path wanders like a trail
                :map, :rng, :area, from_xy: start, toward_xy: best, toward_bias: 0.62, steps: 6000, width: 2
                passable: {matches_none: {Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_HAS_OBJECT}}
                operator: paint(TILES.grass, {matches_all: {SourceMap.FLAG_SOLID}, matches_none: {Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_HAS_OBJECT}})
            }
    return #walkable_components(map, area) <= 1

-- Makes every part of the overworld reachable: stretches of ground with at least keep_size squares
-- are joined to the largest by winding paths (see reconnect_walkable, which needs rng), smaller
-- specks become forest. Returns false if too little ground is left.
connect_walkable = (map, area, min_fraction = 0.1, rng = nil, keep_size = 40) ->
    {x1, y1, x2, y2} = area
    components = walkable_components(map, area)
    if #components == 0
        return false
    for i=2,#components
        if not rng or #components[i] < keep_size
            for xy in *components[i]
                map\square_apply(xy, paint(TILES.trees))
    if rng and not reconnect_walkable(map, rng, area)
        return false
    walkable = #SourceMap.rectangle_match {:map, area: {x1, y1, x2, y2}, selector: WALKABLE}
    return walkable >= min_fraction * (x2 - x1) * (y2 - y1)

-- Regions for statues: the layout area split into a grid of cells
grid_regions = (area, cols, rows, n_statues) ->
    {x1, y1, x2, y2} = area
    w, h = math.floor((x2 - x1) / cols), math.floor((y2 - y1) / rows)
    regions = {}
    for j=0,rows-1
        for i=0,cols-1
            r = from_bbox(x1 + i * w, y1 + j * h, x1 + (i + 1) * w, y1 + (j + 1) * h)
            r.conf = {:n_statues}
            append regions, r
    return regions

-- A MapDesc for an overworld whose layout() paints @map and sets @map.regions (each with a bbox()
-- and conf.n_statues). The layout may return false to retry; every variant then gets the same
-- doors, statues, items and animals as the classic overworld, and the vaults of theme (see
-- maps/vaults/Library.moon).
-- A layout can also set @map.dungeon_regions to steer where World.moon digs the dungeon
-- entrances; items and animals still spread over the whole map.
create = (layout, theme) ->
    return MapDesc.create {
        -- The label is load-bearing: C++ keys the overworld's music and monster wandering off it
        map_label: "Plain Valley"
        size: {120, 120}
        default_content: Tilesets.grass.wall
        default_flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
        arc_chance: 0.05
        children: {
            MapNode.create {
                place: () =>
                    if not layout(@)
                        return false
                    {x1, y1, x2, y2} = layout_area(@)
                    walkable = #SourceMap.rectangle_match {map: @map, area: {x1, y1, x2, y2}, selector: WALKABLE}
                    if walkable < WALKABLE_RANGE[1] or walkable > WALKABLE_RANGE[2]
                        return false
                    if not overworld_features(@region_set, theme)
                        return false
                    whole_map = @region_set.regions
                    if @map.dungeon_regions
                        @region_set.regions = @map.dungeon_regions
                    append @map.post_maps, () ->
                        @region_set.regions = whole_map
                        if not place_doors_and_statues(@region_set)
                            return false
                        if not overworld_items_and_enemies(@region_set)
                            return false
                        return reconnect_walkable(@map, @rng, layout_area(@))
                    return true
            }
        }
    }

return {
    :OVERWORLD_TILESET, :TILES, :paint, :WALKABLE, :WALKABLE_RANGE, :forest_rim, :protect, :protect_matching, :layout_area, :walkable_components, :connect_walkable
    :reconnect_walkable, :grid_regions
    :place_doors_and_statues, :overworld_items_and_enemies, :overworld_features, :create
}
