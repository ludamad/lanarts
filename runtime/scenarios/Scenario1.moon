import MapRegion, combine_map_regions, map_regions_bbox from require "maps.MapRegion"

World = require "core.World"
{:MapCompiler} = require "maps.MapCompiler"
Vaults = require "maps.Vaults"
MapUtils = require "maps.MapUtils"
TileSets = require "tiles.Tilesets"
GenerateUtils = require "maps.GenerateUtils"
import has_overlaps from require "maps.B2GenerateUtils"
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

place_encounter_behind_door = (node, enemies) =>
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
    if #enemy_candidate_squares < #enemies
        return true --false
    for i=1,#enemies
        MapUtils.spawn_enemy(@map, enemies[i], enemy_candidate_squares[i])

N_JUNCTION_TRIES = 4
junction = (nodes) ->
    connector_node = Shape {
        shape: "deformed_ellipse", size: {10, 20},
        properties: {wall_tile: TileSets.hive.wall}
    }
    return Spread {
        regions: table.tconcat {connector_node}, nodes
        spread_scheme: () =>
            {jw, jh} = @as_size connector_node
            -- Keep the junction at 0,0
            for node in *nodes
                region = @as_region node
                region\rotate(@rng\randomf(-math.pi, math.pi))
                angle = @property node, "angle", @rng\randomf(-math.pi, math.pi)
                drift = @property node, "drift", @rng\randomf(1.2, 1.5)
                {w, h} = @as_size node
                for i=1,N_JUNCTION_TRIES
                    dx, dy = (w*drift+jw)*math.cos(angle), (h*drift+jh)*math.sin(angle)
                    region\translate(dx, dy)
                    if false and has_overlaps {regions: [@as_region node for node in *nodes]}
                        if i == N_JUNCTION_TRIES
                            return false
                        region\translate(-dx, -dy)
                        angle = @rng\randomf(-math.pi, math.pi)
                        drift = @rng\randomf(1.2, 1.7)
                    else
                        break
            return true
        properties: {
            tunnel_tile: TileSets.lair.floor
        }
        connection_scheme: () =>
            for node in *nodes
                @_connect_regions 'direct_light', {@as_region(node), @as_region(connector_node)}
    }

create_scenario = (rng) ->
    size_scale = 1
    enemy_n_scale = 1

    adjusted_size = (size) ->
        {w, h} = size
        w *= (rng\randomf(-0.1,0.1) + rng\randomf(0.9, 1.1)) * size_scale
        h *= (rng\randomf(-0.1,0.1) + rng\randomf(0.9, 1.1)) * size_scale
        return {random_round(w, rng), random_round(h, rng)}

    -- NODES
    initial_room = Shape {
        shape: 'deformed_ellipse'
        size: {10, 20}
        properties: {wall_tile: TileSets.hive.wall}
        paint: (node) =>
            @tile_paint(node, TileSets.hive.floor)
        place_objects: (node) =>
            val = place_encounter_behind_door @, node, for i=1,10
                'Ciribot'
            return false if val == false
    }

    enemy_enclosure  = Shape {
        shape: 'deformed_ellipse'
        size: {10, 20}
        properties: {wall_tile: TileSets.hive.wall, floor_tile: TileSets.hive.floor_alt}
    }

    root_node = junction {initial_room, enemy_enclosure}

    -- EVENTS/METHODS
    random_player_square = () =>
        return MapUtils.random_square(@map, @as_bbox(initial_room), {
            matches_group: @as_group_set(initial_room)
            matches_none: {SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID, SourceMap.FLAG_TUNNEL}
        })

    return newtype {
        parent: MapCompiler, :root_node
        tileset: TileSets.lair, generate: do_nothing
        :random_player_square
    }

return {
    Scenario: {
        name: "Firelord's Kingdom"
        place_func: create_scenario
    }
}
