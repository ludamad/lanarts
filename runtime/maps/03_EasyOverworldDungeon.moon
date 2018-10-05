import map_place_object, ellipse_points,
    LEVEL_PADDING, Region, RVORegionPlacer,
    random_rect_in_rect, random_ellipse_in_ellipse,
    ring_region_delta_func, default_region_delta_func, spread_region_delta_func,
    center_region_delta_func,
    towards_region_delta_func,
    rectangle_points,
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree,
    Tile, tile_operator from require "maps.GenerateUtils"

{:MapRegion, :from_bbox} = require "maps.MapRegion"

DebugUtils = require "maps.DebugUtils"
NewMaps = require "maps.NewMaps"
NewDungeons = require "maps.NewDungeons"
Tilesets = require "tiles.Tilesets"
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

{:MapCompilerContext, :make_on_player_interact} = require "maps.MapCompilerContext"
Places = require "maps.Places"

-- Generation constants and data
{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

{:find_bbox, :find_square, :selector_filter, :selector_map} = require "maps.MapRegionUtils"

{:generate_map_node} = require "maps.01_Overworld"

PADDING = 20

rng = NewMaps.new_rng()

Carver = newtype {
    init: (@children, @carve) =>
        @compiled = false
        @_compiled_region_set = false
    map_compile: (args) =>
        map_args = table.merge {
            :rng
            size: {100, 100}
            default_content: Tilesets.crypt.wall
            default_flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
            map_label: "Dungeon"
        }, args
        map_args.size[1] += PADDING * 2
        map_args.size[2] += PADDING * 2
        map = NewMaps.source_map_create map_args

        room_selector = {
            matches_all: SourceMap.FLAG_SOLID
            matches_none: SourceMap.FLAG_PERIMETER
        }
        region_set = {:map, regions: {from_bbox(PADDING, PADDING, map.size[1] - PADDING, map.size[2] - PADDING)\with_selector(room_selector)}}
        @compile(0, region_set)
        return map
    compile: (@parent_group, @region_set) =>
        if @compiled
            return @_compiled_region_set
        @compiled = true
        @_compiled_region_set = @_compile()
        return @_compiled_region_set
    _compile: () =>
        @group = @region_set.map.next_group
        @region_set.map.next_group += 1
        @carve()
        @group_range = {@group, @region_set.map.next_group - 1}
        return selector_map @region_set, (s) ->
            return table.merge s, {matches_group: @group_range}
    chance: (prob) => @region_set.map.rng\chance(prob)
    get: {
        map: () => @region_set.map
        rng: () => @region_set.map.rng
    }
}

Rooms = Carver.create {}, () =>
    -- Ensure it is a range
    n_rooms = 40
    event_log("(RNG #%d) generating %d rooms", rng\amount_generated(), n_rooms)
    regions = {}
    for i=1,n_rooms
        size = {4, 4}
        if @chance(0.2)
            size = {5, 5}
        {:bbox} = find_bbox(@region_set, size)
        if not bbox
            return false
        append regions, from_bbox(unpack(bbox))
        SourceMap.rectangle_apply {
            map: @map
            area: bbox
            fill_operator: {add: SourceMap.FLAG_SEETHROUGH, remove: SourceMap.FLAG_SOLID, content: Tilesets.pebble.floor}
            perimeter_width: 1
            perimeter_operator: {add: SourceMap.FLAG_SEETHROUGH, remove: SourceMap.FLAG_SOLID, content: Tilesets.pebble.floor_alt}
        }
    for region in *regions
        if not SourceMap.try_tunnel_apply {
            map: @map
            rng: @rng
            start_area: region\bbox()
            validity_selector: {
                fill_selector: {matches_all: SourceMap.FLAG_SOLID, matches_none: SourceMap.FLAG_TUNNEL }
                perimeter_selector: { matches_all: SourceMap.FLAG_SOLID, matches_none: SourceMap.FLAG_TUNNEL }
            },

            completion_selector: {
                fill_selector: { matches_none: { SourceMap.FLAG_SOLID, SourceMap.FLAG_PERIMETER, SourceMap.FLAG_TUNNEL } },
                perimeter_selector: { matches_none: SourceMap.FLAG_SOLID }
            },

            fill_operator: { add: {SourceMap.FLAG_TUNNEL, SourceMap.FLAG_SEETHROUGH}, remove: SourceMap.FLAG_SOLID, content: Tilesets.pebble.floor},
            perimeter_operator: {
                matches_all: SourceMap.FLAG_SOLID, remove: SourceMap.FLAG_SEETHROUGH,
                add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_TUNNEL, SourceMap.FLAG_PERIMETER}, content: Tilesets.pebble.wall}

            perimeter_width: 1
            size_range: {1,1}
            tunnels_per_room_range: {1,1}
        }
            return false
    event_log("(RNG #%d) after generating %d rooms", rng\amount_generated(), n_rooms)
    return true

-- Tunnels = Filler.create {Rooms}, () =>
    

return Rooms\map_compile {
    size: {40,40}
}
