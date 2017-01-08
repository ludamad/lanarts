----
-- Generates the game maps, starting with high-level details (places that will be in the game)
-- and then generating actual tiles.
----
import map_place_object, ellipse_points, 
    LEVEL_PADDING, Region, RVORegionPlacer, 
    random_rect_in_rect, random_ellipse_in_ellipse, 
    ring_region_delta_func, default_region_delta_func, spread_region_delta_func,
    center_region_delta_func, 
    towards_region_delta_func, 
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree,
    Tile, tile_operator from require "maps.GenerateUtils"

NewMaps = require "maps.NewMaps"

TileSets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"
import make_tunnel_oper, make_rectangle_criteria, make_rectangle_oper
    from MapUtils

MapSequence = require "maps.MapSequence"
Vaults = require "maps.Vaults"
World = require "core.World"
SourceMap = require "core.SourceMap"
Map = require "core.Map"
OldMaps = require "maps.OldMaps"
Region1 = require "maps.Region1"

-- Generation constants and data
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE, 
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

DEFAULT_DUNGEON_VISION_RADIUS = 7

M = nilprotect {}

M.create_dungeon_scheme = (tileset) -> {
    :tileset
    floor1: Tile.create(tileset.floor, false, true, {}, {FLAG_OVERWORLD})
    floor2: Tile.create(tileset.floor_alt, false, true, {}, {FLAG_OVERWORLD}) 
    wall1: Tile.create(tileset.wall, true, false, {}, {FLAG_OVERWORLD})
    wall2: Tile.create(tileset.wall_alt, true, false, {}, {FLAG_OVERWORLD})
}

M.dungeon_defaults = (rng) -> {
    rvo_iterations: 20
    connect_line_width: () -> 2 + (if rng\random(5) == 4 then 1 else 0)
    region_delta_func: default_region_delta_func
    n_statues: 0
}

M.place_feature = (map, template, region_filter = ()->true) ->
   -- Function to try a single placement, returns success:
   attempt_placement = (template) ->
       orient = map.rng\random_choice {
           SourceMap.ORIENT_DEFAULT, SourceMap.ORIENT_FLIP_X, SourceMap.ORIENT_FLIP_Y,
           SourceMap.ORIENT_TURN_90, SourceMap.ORIENT_TURN_180, SourceMap.ORIENT_TURN_270
       }
       for r in *map.regions
           if not region_filter(r)
               continue
           {w, h} = template.size
           -- Account for rotation in size:
           if orient == SourceMap.ORIENT_TURN_90 or orient == SourceMap.ORIENT_TURN_180
               w, h = h, w
           {x1, y1, x2, y2} = r\bbox()
           -- Expand just outside the bounds of the region:
           x1, y1, x2, y2 = (x1 - w), (y1 - h), (x2 + w), (y2 + h)
           -- Ensure we are within the bounds of the world map:
           x1, y1, x2, y2 = math.max(x1, 0), math.max(y1, 0), math.min(x2, map.size[1] - w), math.min(y2, map.size[2] - h)
           top_left_xy = MapUtils.random_square(map, {x1, y1, x2, y2})
           apply_args = {:map, :top_left_xy, orientation: orient }
           if template\matches(apply_args)
               template\apply(apply_args)
               return true
       return false
   -- Function to try placement n times, returns success:
   attempt_placement_n_times = (template, n) ->
       for i=1,n
           if attempt_placement(template)
               return true
       return false
   -- Try to create the template object using our placement routines:
   if attempt_placement_n_times(template, 100)
       -- Exit, as we have handled the first overworld component
       return true
   return false

M._runed_door_sprite = tosprite("spr_doors.runed_door")
M.make_dungeon_template = (data) -> table.merge {
    outer_conf: data.subtemplates[1]
    shell: 40
    default_wall: Tile.create(data.tileset.wall, true, true, {})
    -- Public members:
    post_pone: (f) =>
        append @_post_poned, f
    -- Private members:
    _post_poned: {}
    _vision_radius: DEFAULT_DUNGEON_VISION_RADIUS
    _player_spawn_points: nil
    _base_vault_config: (args = {}) =>
        enemy_placer = (map, xy) ->
            enemy = OldMaps.enemy_generate(@_enemy_entries())
            MapUtils.spawn_enemy(map, enemy, xy)
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy, nil, args.door_sprite, args.door_key)
        store_placer = (map, xy) ->
            Region1.generate_store(map, xy)
        item_placer = (map, xy) ->
            item = ItemUtils.item_generate ItemGroups.enchanted_items, false, 1 --Randart power level
            MapUtils.spawn_item(map, item.type, item.amount, xy)
        return  {:enemy_placer, :door_placer, :store_placer, :item_placer, tileset: @tileset}
    _default_vault_config: (args = {}) =>
        return @_base_vault_config(args)
    _spawn_enemies: (map) =>
        area = {0,0,map.size[1],map.size[2]}
        OldMaps.generate_from_enemy_entries(map, @_enemy_entries(), @_n_enemies(), area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
        return true
    _spawn_items: (map) =>
        area = {0,0,map.size[1],map.size[2]}
        for group in *@_item_groups()
            for i=1,group[2] do
                sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
                if not sqr
                    break
                map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
                item = ItemUtils.item_generate group[1], false, 1, (group[3] or 2) --Randart power level and chance
                MapUtils.spawn_item(map, item.type, item.amount, sqr) 
        return true
    _recalculate_perimeter: (map) =>
        -- Detect the perimeter, important for the winding-tunnel algorithm.
        SourceMap.perimeter_apply {:map,
            candidate_selector: {matches_all: SourceMap.FLAG_SOLID}, inner_selector: {matches_none: SourceMap.FLAG_SOLID}
            operator: {add: SourceMap.FLAG_PERIMETER}
        }
        SourceMap.perimeter_apply {:map
            candidate_selector: {matches_none: {SourceMap.FLAG_SOLID}}, 
            inner_selector: {matches_all: {SourceMap.FLAG_PERIMETER, SourceMap.FLAG_SOLID}}
            operator: {add: FLAG_INNER_PERIMETER}
        }
    _spawn_statues: (map) =>
        @_recalculate_perimeter(map)
        for region in *map.regions
            for i=1,region.conf.n_statues
                area = region\bbox()
                sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
                if not sqr
                    break
                map\square_apply(sqr, {add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT}, remove: SourceMap.FLAG_SEETHROUGH})
                MapUtils.spawn_decoration(map, OldMaps.statue, sqr, random(0,17))
        return true
    _spawn_healing_squares: (map) =>
        conf = data.subtemplates[1]
        for i=1,conf.n_healing_squares or 0
            region = random_choice(map.regions)
            area = region\bbox()
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}, remove: SourceMap.FLAG_SEETHROUGH})
            MapUtils.spawn_healing_square(map, sqr)
        return true
    _create_encounter_rooms: (map) =>
        default_config =  @_default_vault_config {door_sprite: M._runed_door_sprite, door_key: "dummykey"}
        for i =1,@_n_encounter_vaults()
            conf = table.merge(default_config, {rng: map.rng})
            vault = SourceMap.area_template_create(@_encounter_vault() conf)
            if not M.place_feature(map, vault)
                return nil
        return true
    on_create_game_map: (game_map) =>
        for f in *@_post_poned
            f(game_map)
        Map.set_vision_radius(game_map, @_vision_radius)
        if @_player_spawn_points
            World.players_spawn(game_map, @_player_spawn_points)
        @connector.post_connect(game_map)
}, data

M.make_connector = (types) ->
    placed_portals = {}
    on_generate = types.on_generate
    on_finish = types.on_finish
    types.on_generate = nil
    types.on_finish = nil
    for k,type in pairs(types)
        pretty(k, type)
        assert(type.n_portals and type.sprite and type.connect, "Malformed connector type!")
        placed_portals[k] = {} -- list
    return {
        :placed_portals
        n_portals: (type) ->
            {:n_portals} = types[type]
            return n_portals
        spawn_portal: (type, map, xy) ->
            {:sprite} = types[type]
            portal = MapUtils.spawn_portal(map, xy, sprite)
            append placed_portals[type], portal
            return portal
        random_portal: (type, map, area) ->
            {:sprite} = types[type]
            portal = MapUtils.random_portal(map, area, sprite)
            if portal
                append placed_portals[type], portal
            return portal
        post_generate: (map) ->
            if on_generate
                on_generate(map)
        post_connect: (game_map) ->
            for k, type in pairs(types)
                assert(placed_portals[k], "No portals for " .. k .. "!")
                type.connect(placed_portals[k])
            if on_finish
                on_finish(game_map)
    }

M.make_linear_dungeon = (args) ->
    {:MapSeq, :offset} = args
    assert(type(offset) == "number", "offset must be number!") 
    assert(MapSeq, "Needs MapSeq")
    offset or=1
    assert(args.on_generate and args.sprite_up and args.sprite_down and args.portals_up and args.portals_down)
    local make_connector
    make_dungeon = (floor) ->
        dungeon = NewMaps.map_create (rng) -> 
            args.dungeon_template(rng, floor, make_connector(floor))
        MapSeq\slot_resolve(floor + offset, dungeon)
        return dungeon
    make_connector = (floor) -> M.make_connector {
        on_generate: (map) -> 
            args.on_generate(map, floor)
        on_finish: (game_map) ->
            if args.on_finish
                args.on_finish(game_map, floor)
        up: {
            n_portals: args.portals_up(floor) 
            sprite: args.sprite_up(floor)
            connect: (portals) ->
                for i=1,#portals
                    MapSeq\backward_portal_resolve(floor + offset, portals[i], i)
        }
        down: {
            n_portals: args.portals_down(floor) 
            sprite: args.sprite_down(floor)
            connect: (portals) ->
                for i=1,#portals
                    c = MapSeq\forward_portal_add(floor + offset, portals[i], i, () -> make_dungeon(floor + 1))
                    c()
        }
    }
    return make_dungeon(1)

return M
