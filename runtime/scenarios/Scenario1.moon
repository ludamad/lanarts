import MapRegion, combine_map_regions, map_regions_bbox from require "maps.MapRegion"

World = require "core.World"
{:MapCompiler} = require "maps.MapCompiler"
Vaults = require "maps.Vaults"
MapUtils = require "maps.MapUtils"
TileSets = require "tiles.Tilesets"
GenerateUtils = require "maps.GenerateUtils"
SourceMap = require "core.SourceMap"
Map = require "core.Map"
Region1 = require "maps.Region1"
OldMaps = require "maps.OldMaps"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"

{   :FLAG_ALTERNATE, :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_ROOM, :FLAG_NO_ENEMY_SPAWN, :FLAG_NO_ITEM_SPAWN
} = Vaults

import get_door_candidates from require "maps.DoorGeneration"

import Spread, Shape
    from require "maps.MapElements"

create_scenario = () ->

    -- NODES
    initial_room = Shape {
        shape: "deformed_ellipse", size: {10, 20},
        properties: {wall_tile: TileSets.hive.wall}
        paint: (node) =>
            @tile_paint(node, TileSets.hive.floor)
        place_objects: (node) =>
            area = @get_node_bbox(node)
            group_set = @get_node_group_set(node)
            for xy in *get_door_candidates(@, node)
                -- Generate a runed door
                MapUtils.spawn_door(@map, xy, nil, Vaults._rune_door_closed, "dummykey")
            enemy_candidate_squares = @node_match node, {
                padding: 10
                selector: {
                    matches_all: {SourceMap.FLAG_TUNNEL}
                    matches_none: {SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID, FLAG_DOOR_CANDIDATE}
                }
            }
            if #enemy_candidate_squares < 10
                return false
            for i=1,100
                sqr = enemy_candidate_squares[i]
                if not sqr
                    break
                MapUtils.spawn_enemy(@map, "Ciribot", sqr)
    }

    enemy_enclosure  = Shape {
        shape: "deformed_ellipse", size: {10, 20},
        properties: {wall_tile: TileSets.hive.wall}
        paint: (node) =>
            @tile_paint(node, TileSets.hive.floor_alt)
        place_objects: (node) =>
            enemies = {
                "Red Slime": 10
            }
            -- Loop over all enemies in this room
            for enemy, amount in pairs enemies do for i=1,amount
               -- Generate a single enemy of type 'enemy'
               sqr = MapUtils.random_square(@map, @get_node_bbox(node), {
                   matches_group: @get_node_group_set(node)
                   matches_none: {SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}
               })
               if not sqr
                   continue
               MapUtils.spawn_enemy(@map, enemy, sqr)
            return true
    }

    first_quarters = do
        connection_scheme = 'direct_light'
        properties = {
            tunnel_tile: TileSets.lair.floor
        }
        --connection_scheme = () =>
        --    @connect_map_regions()
            
        Spread {
            regions: {initial_room, enemy_enclosure}
            spread_scheme: {type: 'rvo_spread', iters: 100}
            :properties
            :connection_scheme
        }

    root_node = first_quarters

    --root_node = Spread {
    --    regions: caverns
    --    spread_scheme: {type: 'rvo_spread', iters: 100}
    --    connection_scheme: 'direct_light'
    --}

    -- EVENTS/METHODS
    generate = (args) =>
        nil -- Reconsider if args are passed here or in creation of the map compiler type

    random_player_square = () =>
        return MapUtils.random_square(@map, @get_node_bbox(initial_room), {
            matches_group: @get_node_group_set(initial_room)
            matches_none: {SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID, SourceMap.FLAG_TUNNEL}
        })

    return newtype {
        parent: MapCompiler, :root_node
        tileset: TileSets.lair, :generate
        :random_player_square
    }

return {
    Scenario: {
        name: "Firelord's Kingdom"
        place: create_scenario()
    }
}
