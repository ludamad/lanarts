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
NewDungeons = require("maps.NewDungeons")

{:MapDesc, :MapNode} = require("maps.MapDesc")
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


M = nilprotect {} -- Module

graghs_lair_plan = (rng) -> {
    rvo_iterations: 20
    connect_line_width: () -> 2 + (if rng\random(5) == 4 then 1 else 0)
    region_delta_func: default_region_delta_func
    tileset: Tilesets.lair
    floor1: Tile.create(Tilesets.lair.floor, false, true, {}, {FLAG_OVERWORLD})
    floor2: Tile.create(Tilesets.lair.floor_alt, false, true, {}, {FLAG_OVERWORLD})
    wall1: Tile.create(Tilesets.lair.wall, true, false, {}, {FLAG_OVERWORLD})
    wall2: Tile.create(Tilesets.lair.wall_alt, true, false, {}, {FLAG_OVERWORLD})
    size: {rng\random(25,45), rng\random(25,45)}
    n_subareas: 3
    n_enemies: 0
    n_encounter_vaults: 0
    enemy_entries: {
        {enemy: "Adder", guaranteed_spawns: 6, chance: 100}
        {enemy: "Black Mamba", guaranteed_spawns: 2, chance: 25}
        {enemy: "Mouther", guaranteed_spawns: 2}
    }
    item_groups: {{ItemGroups.basic_items, 8}}
    number_regions: 2
    room_radius: () ->  13
    rect_room_num_range: {0, 0}
    rect_room_size_range: {1, 1}
    n_statues: 4
    n_subareas: 1
    size: {45, 45}
    n_items: 4
    n_enemies: 0
    n_stores: 2
    n_epic_stores: 2
    n_encounter_vaults: 3
    enemy_entries: {
        {enemy: "Sheep", guaranteed_spawns: 5, chance: 100}
        {enemy: "Mouther", guaranteed_spawns: 4}
        {enemy: "Black Mamba", guaranteed_spawns: 4}
        {enemy: "Hydra", guaranteed_spawns: 1}
        {enemy: "Gragh", guaranteed_spawns: 1}
        {enemy: "Elephant", guaranteed_spawns: 5}
    }
    item_groups: {{ItemGroups.enchanted_items, 4}}
    :number_regions
    :room_radius
    rect_room_num_range: {0, 0}
    rect_room_size_range: {1, 1}
    n_statues: 4
}

floor_plans = (rng) ->
    tileset = TileSets.lair
    {:number_regions, :room_radius} = rng\random_choice {
        {
            number_regions: 5
            room_radius: () -> rng\random(5, 12)
        }
        {
            number_regions: 1
            room_radius: () -> 25
        }
    }
    plans = {
        [1]: {
            wandering_enabled: false
            n_subareas: 1
            size: {45, 45}
            n_items: 4
            n_enemies: 0
            n_stores: 2
            n_epic_stores: 2
            n_encounter_vaults: 3
            enemy_entries: {
                {enemy: "Sheep", guaranteed_spawns: 5, chance: 100}
                {enemy: "Mouther", guaranteed_spawns: 4}
                {enemy: "Black Mamba", guaranteed_spawns: 4}
                {enemy: "Hydra", guaranteed_spawns: 1}
                {enemy: "Gragh", guaranteed_spawns: 1}
                {enemy: "Elephant", guaranteed_spawns: 5}
            }
            item_groups: {{ItemGroups.enchanted_items, 4}}
            :number_regions
            :room_radius
            rect_room_num_range: {0, 0}
            rect_room_size_range: {1, 1}
            n_statues: 4
        }
    }
    return for i, plan in ipairs plans
        base = table.merge NewDungeons.dungeon_defaults(rng), NewDungeons.create_dungeon_scheme(tileset)
        table.merge(base, plan)

M.N_FLOORS = 1
M.TEMPLATE  = (rng, floor, connector) -> 
    n_stairs_up = connector.n_portals("up")
    plan = floor_plans(rng)[floor]
    subtemplates = for i=1,plan.n_subareas
        floor_plans(rng)[floor]
    return NewDungeons.make_dungeon_template {
        map_label: "Gragh's Lair"
        tileset: TileSets.lair
        :subtemplates
        :connector
        w: plan.size[1] * 2.5 + 10
        h: plan.size[2] * 2.5 + 10
        _enemy_entries: () =>
            return plan.enemy_entries
        _n_enemies: () =>
            return math.ceil(plan.n_enemies) -- * OldMaps.enemy_bonus())
        _n_encounter_vaults: () =>
            return plan.n_encounter_vaults
        _encounter_vault: () =>
            return Vaults.crypt_encounter_vault
        _item_groups: () =>
            return plan.item_groups
        _create_stairs_down: (map) =>
            area = {0,0,map.size[1],map.size[2]}
            for i =1,connector.n_portals("down")
                if not connector.random_portal("down", map, area)
                    return nil
            return true
        _create_stairs_up: (map) =>
            base_conf = @_default_vault_config()
            for i =1,connector.n_portals("up")
                up_stairs_placer = (map, xy) ->
                    connector.spawn_portal("up", map, xy)
                conf = table.merge base_conf, {rng: map.rng, item_placer: up_stairs_placer}
                vault = SourceMap.area_template_create(Vaults.crypt_entrance_vault conf)
                if not NewDungeons.place_feature(map, vault)
                    return nil
            if connector.n_portals("up") == 0 -- Player spawn
                for i =1,1
                    player_placer = (map, xy) ->
                        {x, y} = xy
                        append map.player_candidate_squares, {x * 32 + 16, y * 32 + 16}
                    conf = table.merge base_conf, {rng: map.rng, item_placer: player_placer}
                    vault = SourceMap.area_template_create(Vaults.crypt_entrance_vault conf)
                    if not NewDungeons.place_feature(map, vault)
                        return nil
            return true
        on_create_source_map: (map) =>
            if not @_create_stairs_up(map)
                print("ABORT: stairs up")
                return nil
            if not @_create_stairs_down(map)
                print("ABORT: stairs down")
                return nil
            if not @_create_encounter_rooms(map)
                print("ABORT: stairs encounter")
                return nil
            if not @_spawn_statues(map)
                print("ABORT: statues")
                return nil
            if not @_spawn_stores(map)
                print("ABORT: statues")
                return nil
            if not @_spawn_items(map)
                print("ABORT: items")
                return nil
            if not @_spawn_enemies(map)
                print("ABORT: enemies")
                return nil
            NewMaps.generate_door_candidates(map, rng, map.regions)
            if #map.player_candidate_squares > 0
                @_player_spawn_points = MapUtils.pick_player_squares(map, map.player_candidate_squares)
                if not @_player_spawn_points
                    print("ABORT: player spawns")
                    return nil
            @connector.post_generate(map)
            return true
    }

return MapDesc.create {
    map_label: "Gragh's Lair"
    size: {130, 130}
    default_content: Tilesets.snake.wall
    arc_chance: 1.0 -- Always use arcs in snake pit
    wandering_enabled: false
    children: {
        MapNode.create {
            place: () =>
                -- TODO allow templates not to be root node? or just toss the concept
                template = make_template(@map.rng, @desc.back_links, @desc.forward_links)
                if not NewMaps.map_try_create(@map, @map.rng, template)
                    return false
                if not template\on_create_source_map(@map)
                    return false
                return true
        }
    }
}
