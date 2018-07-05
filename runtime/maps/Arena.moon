import MapRegion, combine_map_regions, map_regions_bbox from require "maps.MapRegion"

World = require "core.World"
{:MapCompiler} = require "maps.MapCompiler"
MapUtils = require "maps.MapUtils"
TileSets = require "tiles.Tilesets"
GenerateUtils = require "maps.GenerateUtils"
SourceMap = require "core.SourceMap"
Map = require "core.Map"
Region1 = require "maps.Region1"
OldMaps = require "maps.OldMaps"

import Arena from require "maps.Places"
import Spread, Shape
    from require "maps.MapElements"

TEMPLATE = Spread {
    regions: for i=1,40
        Shape {
            shape: 'deformed_ellipse'
            size: {5, 5}
        }
    spread_scheme: 'box2d_solid_center'
    connection_scheme: 'direct'
}

--TEMPLATE = Shape {
--    shape: 'windows'
--    size: {40,40}
--}

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
    for enemy, amount in pairs loadstring(os.getenv "ARENA_ENEMIES")()
        for i=1,amount
            {x, y} = MapUtils.random_square(compiler.map, nil)
            MapUtils.spawn_enemy(compiler.map, enemy, {x, y})
    for i=1,10
        {x, y} = MapUtils.random_square(compiler.map, nil)
        Region1.generate_epic_store(compiler.map, {x, y})
    for i=1,10
        {x, y} = MapUtils.random_square(compiler.map, nil)
        MapUtils.spawn_decoration(compiler.map, OldMaps.statue, {x, y}, random(0,17), true)
    for i=1,10
        {x, y} = MapUtils.random_square(compiler.map, nil)
        MapUtils.spawn_chest(compiler.map, {x, y}, {
            {type: 'Arrow', amount: 10}
            {type: 'Arrow', amount: 10}
            {type: 'Arrow', amount: 10}
            {type: 'Arrow', amount: 10}
        })

    return compiler\compile(), compiler.map, player_spawn_points

main = () ->
    DebugUtils = require "maps.DebugUtils"
    DebugUtils.enable_visualization(800, 600)
    rng = require('mtwist').create(os.time())
    map = generate(rng)
    DebugUtils.debug_show_source_map(map)

return {:generate, :main}
