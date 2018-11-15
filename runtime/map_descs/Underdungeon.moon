{:MapNode, :MapDesc} = require "maps.MapDesc"

{:Tile, :spread_region_delta_func} = require "maps.GenerateUtils"
NewMaps = require "maps.NewMaps"
Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
Vaults = require "maps.Vaults"
{:place_vault_in} = require "maps.VaultUtils"
SourceMap = require "core.SourceMap"
OldMaps = require "maps.OldMaps"

{:MapCompilerContext, :make_on_player_interact} = require "maps.MapCompilerContext"
Places = require "maps.Places"

-- Generation constants and data
{ :FLAG_INNER_PERIMETER, :FLAG_NO_ENEMY_SPAWN, :FLAG_OVERWORLD, :FLAG_DOOR_CANDIDATE } = Vaults

create_dungeon_scheme = (tileset) -> nilprotect {
    floor1: Tile.create(tileset.floor, false, true, {}, {FLAG_OVERWORLD})
    floor2: Tile.create(tileset.floor_alt, false, true, {}, {FLAG_OVERWORLD})
    wall1: Tile.create(tileset.wall, true, false, {}, {FLAG_OVERWORLD})
    wall2: Tile.create(tileset.wall_alt, true, false, {}, {FLAG_OVERWORLD})
}

DUNGEON_TILESET = create_dungeon_scheme(Tilesets.underdungeon)

underdungeon_items_and_enemies = (region_set) ->
    {:map, :regions} = region_set

    for region in *regions
        area = region\bbox()
        for i=1,OldMaps.adjusted_item_amount(10) do
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
            item = ItemUtils.item_generate ItemGroups.basic_items
            MapUtils.spawn_item(map, item.type, item.amount, sqr)

        OldMaps.generate_from_enemy_entries(map, OldMaps.hard_enemies, 10, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
        OldMaps.generate_from_enemy_entries(map, OldMaps.fast_enemies, 10, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})

place_doors_and_statues = (region_set) ->
    {:map, :regions} = region_set

    for region in *regions
        area = region\bbox()
        conf = region.conf
        for xy in *SourceMap.rectangle_match {:map, selector: {matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}, matches_all: {FLAG_DOOR_CANDIDATE}}}
            MapUtils.spawn_door(map, xy)
        for i=1,conf.n_statues
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT}, remove: SourceMap.FLAG_SEETHROUGH})
            MapUtils.spawn_decoration(map, OldMaps.statue, sqr, random(0,17))

underdungeon_features = (region_set, back_links, forward_links) ->
    -----------------------------
    -- Purple Dragon lair            --
    place_purple_dragon_lair = () ->
        name = "Purple Dragon Lair"
        create_compiler_context = () ->
            cc = MapCompilerContext.create()
            cc\register(name, Places.DragonLair)
            append region_set.map.post_game_map, (game_map) ->
                cc\register("root", () -> game_map)
            return cc
        cc = create_compiler_context()
        return place_vault_in region_set, Vaults.graghs_lair_entrance {
            tileset: Tilesets.hell
            dungeon_placer: (map, xy) ->
                -- Make portal
                portal = MapUtils.spawn_portal(map, xy, "spr_gates.volcano_portal")
                other_portal = cc\add_pending_portal name, (feature, compiler) ->
                    MapUtils.random_portal(compiler.map, nil, "spr_gates.volcano_exit")
                other_portal\connect {feature: portal, label: "root"}
                portal.on_player_interact = make_on_player_interact(cc, other_portal)
                return portal
            door_placer: (map, xy) ->
                MapUtils.spawn_door(map, xy, nil, Vaults._door_key2, "Dandelite Key")
            enemy_placer: (map, xy) ->
                enemy = OldMaps.enemy_generate({{enemy: "Fire Bat", chance: 100}})
                MapUtils.spawn_enemy(map, enemy, xy)
        }
    if not place_purple_dragon_lair()
        return false

    append region_set.map.post_maps, () ->
        underdungeon_items_and_enemies(region_set)

    for link in *table.tconcat(back_links, forward_links)
        xy = MapUtils.random_square(region_set.map, nil, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
        if not xy
            return false
        link(region_set.map, xy)
    return true

return MapDesc.create {
    map_label: "Underdungeon"
    size: {200, 200}
    default_content: Tilesets.underdungeon.wall
    default_flags: {SourceMap.FLAG_SOLID}
    arc_chance: 0.05
    children: {
        MapNode.create {
            place: () =>
                size = @rng\random_choice {{200, 180}, {180, 200}}
                number_regions = @rng\random(5, 7)
                connect_line_width = () -> @rng\random(2, 6)
                room_radius = () -> @rng\random(5,10)
                template = nilprotect {
                    default_wall:  Tile.create(Tilesets.underdungeon.wall, true, true, {SourceMap.FLAG_SOLID})
                    subtemplates: {nilprotect {
                        is_overworld: true
                        :size
                        :number_regions
                        floor1: DUNGEON_TILESET.floor1
                        floor2: DUNGEON_TILESET.floor2
                        wall1: DUNGEON_TILESET.wall1
                        wall2: DUNGEON_TILESET.wall2
                        rect_room_num_range: {4,8} -- disable
                        rect_room_size_range: {14,20}
                        rvo_iterations: 100
                        :connect_line_width
                        :room_radius
                        region_delta_func: spread_region_delta_func
                        -- Dungeon objects/features
                        n_statues: 4
                    }}
                    outer_conf: nilprotect {
                        floor1: DUNGEON_TILESET.floor1
                        floor2: DUNGEON_TILESET.floor2
                        :connect_line_width
                    }
                    shell: 25
                }
                -- TODO, handle links to overworld...
                if not NewMaps.map_try_create(@map, @map.rng, template)
                    return false

                NewMaps.generate_door_candidates(@map, @map.rng, @map.regions)
                append @map.post_maps, () ->
                    return place_doors_and_statues(@region_set)
                if not underdungeon_features(@region_set, @desc.back_links, @desc.forward_links)
                    return false
                return true
        }
    }
}
