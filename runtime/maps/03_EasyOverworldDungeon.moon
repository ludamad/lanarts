import map_place_object, ellipse_points,
    LEVEL_PADDING, Region, RVORegionPlacer,
    random_rect_in_rect, random_ellipse_in_ellipse,
    ring_region_delta_func, default_region_delta_func, spread_region_delta_func,
    center_region_delta_func,
    towards_region_delta_func,
    rectangle_points,
    random_region_add, subregion_minimum_spanning_tree, region_minimum_spanning_tree,
    Tile, tile_operator from require "maps.GenerateUtils"

import MapRegion from require "maps.MapRegion"

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

{:find_bbox, :find_square, :selector_filter} = require "maps.MapRegionUtils"

{:generate_map_node} = require "maps.01_Overworld"

rng = NewMaps.new_rng()

Rooms = {
    carve: () =>
        region_set = selector_filter(@region_set, {
            matches_all: SourceMap.FLAG_SOLID
            matches_none: SourceMap.FLAG_PERIMETER
        })
        -- Ensure it is a range
        n_rooms = 40
        event_log("(RNG #%d) generating %d rooms", rng\amount_generated(), n_rooms)
        size = {4, 4}
        for i=1,n_rooms
            bbox = find_bbox(region_set, size)
            if not bbox
                return false
            SourceMap.rectangle_apply {
                area: bbox
                fill_operator: {add: SourceMap.FLAG_SEETHROUGH, remove: SourceMap.FLAG_SOLID, content: floor_tile}
            }
        event_log("(RNG #%d) after generating %d rooms", rng\amount_generated(), n_rooms)
        return true
}

-- Stairs = {
--     apply: () =>
--         @region_set
-- }

map = NewMaps.source_map_create {
    :rng
    size: {40, 40}
    default_content: Tilesets.pebble.wall
    default_flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
    map_label: "Enclave"
}

return map
