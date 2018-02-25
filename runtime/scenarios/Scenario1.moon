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

import Spread, Shape
    from require "maps.MapElements"

create_scenario = () ->

    generate_handlers = {}
    _generate = (node, f) ->
        append generate_handlers, f

    -- NODES
    caverns = for i=1,2
        Spread {
            regions: {
                Shape {
                    shape: 'deformed_ellipse'
                    size: {10, 20}
                }
                Shape {
                    shape: 'deformed_ellipse'
                    size: {20, 10}
                }
            }
            spread_scheme: {type: 'rvo_spread', iters: 100}
            connection_scheme: 'direct_light'
        }

    root_node = Spread {
        regions: caverns
        spread_scheme: {type: 'rvo_spread', iters: 100}
        connection_scheme: 'direct_light'
    }

    -- EVENTS/METHODS
    generate = (args) =>
        -- TODO fix
        enemies = {
            "Red Slime": 10
        }
        for enemy, amount in pairs enemies do for i=1,amount
           sqr = MapUtils.random_square(@map, @get_node_bbox(caverns[1]), {
               matches_group: @get_node_group_set(caverns[1])
               matches_none: {SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}
           })
           if not sqr
               continue
           MapUtils.spawn_enemy(@map, enemy, sqr)
        return nil

    random_player_square = () =>
        return MapUtils.random_square(@map, @get_node_bbox(caverns[1]), {
            --matches_group: @get_node_group_set(caverns[1])
            matches_none: {SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}
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
