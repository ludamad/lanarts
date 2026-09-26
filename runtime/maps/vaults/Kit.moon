----
-- The vault kit: vaults written as ASCII layouts over one shared set of legend characters, drawn in
-- a palette of tiles. The vault files in this folder (listed in Library.moon) each return a list of
-- definitions:
--   name      unique id, eg "farmland_barn"
--   title     display name; blurb: one line on what it is
--   kind      "landmark", "treasure", "encounter" or "store" (overworld features), or "entrance"
--   themes    the overworld themes (features) or dungeons (entrances) it belongs to; "any" for all
--   palette   a PALETTES name, a Tilesets name, or a table of tile refs; when absent, the palette
--             the placer passes (an entrance's dungeon), else the overworld's
--   base      the tile key under spawns and decorations ("floor" unless set)
--   legend    extra or overriding characters, as specs (see STANDARD)
--   enemies   optional enemy names for 'e', in place of the overworld's animals; brutes: the same
--             for 'E', in place of OldMaps.easy_enemies
--   layouts   a list of ASCII layouts; each placement picks one
--
-- A tile ref is a palette key (floor, floor_alt, path, ground, ground_alt, wall, wall_alt, trees,
-- rock) or "<tileset>.<key>" naming a tile in tiles/Tilesets.lua.
----

SourceMap = require "core.SourceMap"
Display = require "core.Display"
MapUtils = require "maps.MapUtils"
Tilesets = require "tiles.Tilesets"
Vaults = require "maps.Vaults"
{:place_vault_in} = require "maps.VaultUtils"

{:FLAG_HAS_VAULT, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN} = Vaults
{:FLAG_SOLID, :FLAG_SEETHROUGH, :FLAG_HAS_OBJECT} = SourceMap
HAS_CONTENT = FLAG_HAS_VAULT + FLAG_HAS_OBJECT

-- Specs for the characters every vault shares. A spec has:
--   keep    leave the square as it is
--   need    "walkable" or "solid": what the square must be before the vault goes down
--   tile    tile ref laid down (the vault's base tile when absent)
--   solid   blocks movement; seethrough: a solid square that doesn't block sight
--   spawn   door, portal, player, gold, item, exp, enemy, brute or store; chance: its odds (default 1)
--   decor   statue, anvil, tombstone, skull or shrub (all but skull block movement)
STANDARD = {
    '.': {keep: true}                                   -- anything; left untouched
    '+': {keep: true, need: "walkable"}                 -- open ground the vault is entered from
    'o': {tile: "floor"}
    ',': {tile: "floor_alt"}
    ':': {tile: "path"}
    'g': {tile: "ground"}
    '"': {tile: "ground_alt"}
    '#': {tile: "wall", solid: true}
    'W': {tile: "wall_alt", solid: true}
    'R': {tile: "rock", solid: true}
    'T': {tile: "trees", solid: true, seethrough: true}
    '1': {tile: "wall", solid: true, need: "solid"}     -- wall set into existing rock or forest
    '2': {tile: "wall_alt", solid: true, need: "solid"}
    'd': {tile: "floor_alt", spawn: "door"}
    'D': {spawn: "portal"}                              -- the dungeon's portal (entrances)
    'p': {spawn: "player"}                              -- where players may start (entrances)
    '*': {spawn: "gold"}
    'i': {spawn: "item"}
    'X': {spawn: "exp"}
    'e': {spawn: "enemy"}
    'E': {spawn: "brute"}
    '$': {spawn: "store"}
    's': {decor: "statue"}
    'a': {decor: "anvil"}
    't': {decor: "tombstone"}
    'k': {decor: "skull"}
    'h': {decor: "shrub"}
}

-- Palette keys every palette falls back to: the overworld's own tiles
DEFAULT_PALETTE = {
    floor: "pebble.floor", floor_alt: "grass.dirt", path: "grass.dirt"
    ground: "grass.floor", ground_alt: "grass.floor_alt1", ground_dry: "grass.floor_alt2"
    wall: "grass.wall_alt", wall_alt: "grass.wall_alt", trees: "grass.wall", rock: "grass.wall_alt"
}

-- Named palettes; any Tilesets name also works as a palette of that tileset's tiles
PALETTES = {
    meadow: {floor: "grass.floor_alt1", floor_alt: "grass.floor_alt2"}
    stone: {floor: "snake.floor", floor_alt: "snake.floor_alt", wall: "snake.wall", wall_alt: "brick.wall_alt"}
    farm: {floor: "brick.floor_alt", floor_alt: "grass.floor_alt2", wall: "brick.wall", wall_alt: "grass.wall"}
    village: {floor: "stonework.floor", floor_alt: "brick.floor_alt", wall: "brick.wall", wall_alt: "stonework.wall"}
    druid: {floor: "moss.floor", floor_alt: "grass.floor_alt1", wall: "moss.wall_alt", wall_alt: "moss.wall"}
    impact: {floor: "volcanic.floor_alt", floor_alt: "quarry.floor", wall: "quarry.wall", wall_alt: "cobalt.wall_alt"}
    shrine: {floor: "marble.floor", floor_alt: "marble.floor_alt", wall: "marble.wall", wall_alt: "marble.wall_alt"}
    burrow: {floor: "lair.floor", floor_alt: "grass.dirt", wall: "grass.wall", wall_alt: "lair.wall"}
}

tileset_palette = (name) -> {
    floor: "#{name}.floor", floor_alt: "#{name}.floor_alt", wall: "#{name}.wall", wall_alt: "#{name}.wall_alt"
}

-- The palette (a table of tile refs) a vault is drawn with
palette_of = (def, args = {}) ->
    p = def.palette or args.palette
    named = if type(p) == "table" then p elseif PALETTES[p] then PALETTES[p] elseif p then tileset_palette(p) else {}
    return table.merge DEFAULT_PALETTE, named

-- Resolves a tile ref to "<tileset>.<key>"
resolve_ref = (ref, palette) ->
    for i=1,3
        if not palette[ref]
            break
        ref = palette[ref]
    assert ref\find("%."), "Unknown tile ref '#{ref}'"
    return ref

tile_of = (ref) ->
    tileset, key = ref\match("^(.-)%.(.*)$")
    tile = Tilesets[tileset] and rawget(Tilesets[tileset], key)
    assert tile, "No tile #{ref}"
    return tile

-- The legend spec for a character of def (the vault's own legend first)
spec_of = (def, c) ->
    spec = def.legend and def.legend[c]
    if spec and spec.like
        spec = table.merge STANDARD[spec.like], spec
    return spec or STANDARD[c]

-- The rows of a layout, padded to one width
layout_rows = (layout) ->
    rows = [row for row in layout\gmatch("[^\n]+") when row\match("%S")]
    width = 0
    for row in *rows
        width = math.max(width, #row)
    return [row .. string.rep(".", width - #row) for row in *rows]

-------------------------------------------------------------------------------
-- Placement
-------------------------------------------------------------------------------

ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
OldMaps = require "maps.OldMaps"
Region1 = require "maps.Region1"

DECOR = {
    statue: {sprite: OldMaps.statue, frames: 18, solid: true}
    anvil: {sprite: Vaults._anvil, solid: true, seethrough: true}
    tombstone: {sprite: Display.image_load("features/sprites/tombstone.png"), solid: true, seethrough: true}
    skull: {sprite: Vaults._warning_skull}
    shrub: {sprite: Display.image_load("features/sprites/shrub.png"), solid: true, seethrough: true}
}

-- The placer for each spawn; args can override any of them with <spawn>_placer (door_placer,
-- dungeon_placer for the portal, ...)
spawners = (def, args) -> {
    door: args.door_placer or (map, xy) -> MapUtils.spawn_door(map, xy)
    portal: args.dungeon_placer or args.item_placer or (map, xy) ->
        item = ItemUtils.item_generate ItemGroups.basic_items
        MapUtils.spawn_item(map, item.type, item.amount, xy)
    player: (map, xy) ->
        if args.player_spawn_area
            {x, y} = xy
            append map.player_candidate_squares, {x*32+16, y*32+16}
    gold: args.gold_placer or (map, xy) -> MapUtils.spawn_item(map, "Gold", map.rng\random(2, 11), xy)
    item: args.item_placer or (map, xy) ->
        item = ItemUtils.item_generate ItemGroups.basic_items
        MapUtils.spawn_item(map, item.type, item.amount, xy)
    exp: (map, xy) -> MapUtils.spawn_item(map, "Scroll of Experience", 1, xy)
    enemy: args.enemy_placer or (map, xy) ->
        enemy = if def.enemies then map.rng\random_choice(def.enemies) else OldMaps.enemy_generate(OldMaps.medium_animals)
        MapUtils.spawn_enemy(map, enemy, xy)
    brute: args.brute_placer or (map, xy) ->
        enemy = if def.brutes then map.rng\random_choice(def.brutes) else OldMaps.enemy_generate(OldMaps.easy_enemies)
        MapUtils.spawn_enemy(map, enemy, xy)
    store: args.store_placer or (map, xy) -> Region1.generate_store(map, xy)
}

-- A SourceMap legend entry for a spec
legend_entry = (spec, def, palette, placers) ->
    if spec.keep
        return {remove: {}, matches_none: (if spec.need == "walkable" then {FLAG_HAS_VAULT, FLAG_SOLID} else nil)}
    decor = spec.decor and DECOR[spec.decor]
    solid = spec.solid or (decor and decor.solid)
    seethrough = if spec.solid then spec.seethrough else (not decor or not decor.solid or decor.seethrough)
    add = {FLAG_HAS_VAULT}
    remove = {}
    append (if solid then add else remove), FLAG_SOLID
    append (if seethrough then add else remove), FLAG_SEETHROUGH
    if not solid
        append add, FLAG_NO_ENEMY_SPAWN
        append add, FLAG_NO_ITEM_SPAWN
    if decor or spec.spawn == "door" or spec.spawn == "portal" or spec.spawn == "store"
        append add, FLAG_HAS_OBJECT
    on_placement = nil
    if decor
        on_placement = (map, xy) ->
            frame = if decor.frames then map.rng\random(0, decor.frames) else 0
            MapUtils.spawn_decoration(map, decor.sprite, xy, frame, decor.solid or false)
    elseif spec.spawn
        on_placement = assert placers[spec.spawn], "Unknown spawn '#{spec.spawn}'"
        if spec.chance
            spawn = on_placement
            on_placement = (map, xy) ->
                if map.rng\randomf() < spec.chance
                    spawn(map, xy)
    return {
        :add, :remove, :on_placement
        content: tile_of(resolve_ref(spec.tile or def.base or "floor", palette))
        matches_none: {HAS_CONTENT}
        matches_all: (if spec.need == "solid" then FLAG_SOLID else nil)
    }

-- The area template spec (legend + data) for layout i of def
template = (def, args, i) ->
    palette = palette_of(def, args)
    placers = spawners(def, args)
    rows = layout_rows(def.layouts[i])
    legend = {}
    for row in *rows
        for c in row\gmatch(".")
            if not legend[c]
                spec = spec_of(def, c)
                assert spec, "Vault #{def.name}: unknown character '#{c}'"
                legend[c] = legend_entry(spec, def, palette, placers)
    return {:legend, data: table.concat(rows, "\n")}

M = {:STANDARD, :PALETTES, :DEFAULT_PALETTE, :palette_of, :resolve_ref, :spec_of, :layout_rows, :template}

-- Called with (def, layout index) after each placement; engine.StartMapGenCheck hooks it
M.on_placed = do_nothing

-- LANARTS_VAULT=name or name:layout forces that vault (and layout) wherever its kind is placed
forced = os.getenv("LANARTS_VAULT")
if forced and forced ~= ""
    M.force_name, layout = forced\match("^([^:]+):?(%d*)$")
    M.force_layout = tonumber(layout)

-- Places def somewhere in region_set: a random layout, then another if that one won't fit.
-- Returns success.
M.place = (region_set, def, args = {}) ->
    {:rng} = region_set.map
    n = #def.layouts
    order = if M.force_layout and def.name == M.force_name
        {M.force_layout}
    else
        first = rng\random(1, n + 1)
        if n > 1 then {first, (first + rng\random(0, n - 1)) % n + 1} else {first}
    for i in *order
        if place_vault_in(region_set, template(def, args, i), 60)
            M.on_placed(def, i)
            return true
    return false

return M
