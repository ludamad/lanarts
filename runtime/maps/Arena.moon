import MapRegion, combine_map_regions, map_regions_bbox from require "maps.MapRegion"


World = require "core.World"
{:MapCompiler} = require "maps.MapCompiler"
MapUtils = require "maps.MapUtils"
TileSets = require "tiles.Tilesets"
GenerateUtils = require "maps.GenerateUtils"
SourceMap = require "core.SourceMap"
Map = require "core.Map"

import ConnectedRegions, FilledRegion
    from require "maps.MapElements"

node = (args) ->

TEMPLATE = ConnectedRegions {
    regions: for i=1,10
        FilledRegion {
            --shape: 'deformed_ellipse'
            shape: 'windows'
            size: {25, 25}
        }
    spread_scheme: 'box2d'
    connection_scheme: 'direct'
}

TILESET = TileSets.lair

generate = (rng) ->
    compiler = MapCompiler.create {
        label: "Arena"
        :rng
        content: TILESET.wall
        root: TEMPLATE
    }
    i = 1
    -- Fill the room:
    compiler\for_all_nodes (node) =>
        @apply node, {
            operator: {
                remove: SourceMap.FLAG_SOLID
                add: SourceMap.FLAG_SEETHROUGH
                content: if i%2 == 1 then TILESET.floor else TILESET.floor_alt
            }
        }
        i += 1
    {w,h} = compiler.map.size
    SourceMap.area_fill_unconnected {
        map: compiler.map 
        seed: {w/2, h/2}
        unfilled_selector: {matches_none: {SourceMap.FLAG_SOLID}}
        mark_operator: {add: {SourceMap.FLAG_RESERVED2}}
        marked_selector: {matches_all: {SourceMap.FLAG_RESERVED2}}
        fill_operator: {content: TILESET.wall, add: SourceMap.FLAG_SOLID, remove: SourceMap.FLAG_SEETHROUGH}
    }
    -- Place items
    --compiler\for_all_nodes (node) =>


    player_spawn_points = for player in *World.players
        {x, y} = MapUtils.random_square(compiler.map, nil)
        {x*32+16,y*32+16}
    return compiler\compile(), compiler.map, player_spawn_points

main = () ->
    DebugUtils = require "maps.DebugUtils"
    DebugUtils.enable_visualization(800, 600)
    rng = require('mtwist').create(os.time())
    map = generate(rng)
    DebugUtils.debug_show_source_map(map)

return {:generate, :main}
