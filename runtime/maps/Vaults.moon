-- Vaults.moon:
-- Contains schemas for dungeon components that are added via brute force and pattern matching.
-- These components try a bunch of times with the more sophisticated components, and have a fall-back 'simple component'. 

SourceMap = require "core.SourceMap"
MapUtils = require "maps.MapUtils"
TileSets = require "tiles.Tilesets"
Display = require "core.Display"

M = {}

M.FLAG_ALTERNATE = SourceMap.FLAG_CUSTOM1
M.FLAG_INNER_PERIMETER = SourceMap.FLAG_CUSTOM2
M.FLAG_DOOR_CANDIDATE = SourceMap.FLAG_CUSTOM3
M.FLAG_OVERWORLD = SourceMap.FLAG_CUSTOM4
M.FLAG_ROOM = SourceMap.FLAG_CUSTOM5
M.FLAG_NO_ENEMY_SPAWN = SourceMap.FLAG_CUSTOM6
M.FLAG_NO_ITEM_SPAWN = SourceMap.FLAG_CUSTOM7

M._warning_skull = Display.image_load "features/sprites/warning.png"
M._anvil = Display.image_load "features/sprites/anvil.png"

-- Common definitions:
make_legend = (legend) ->
    return table.merge {
        '.': { -- '.' means 'any tile'
            remove: {}
        }
        '+': {  -- '.' means 'walkable tile'
            remove: {}
            matches_none: SourceMap.FLAG_SOLID
        }
    }, legend

M.pebble_ridge_dungeon = (args) -> {
    legend: make_legend {
        'p': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN, M.FLAG_NO_ITEM_SPAWN}
            content: TileSets.pebble.floor
            on_placement: (map, xy) ->
                print "ON PLACEMENT"
                {x, y} = xy
                append map.player_candidate_squares, {x*32+16, y*32+16}
        }
        'D': {
            add: SourceMap.FLAG_SEETHROUGH
            content: TileSets.pebble.floor
            on_placement: args.dungeon_placer
        }
        'W': {
            add: SourceMap.FLAG_SOLID
            content: TileSets.grass.wall_alt
            matches_all: SourceMap.FLAG_SOLID
        }
        'w': {
            add: SourceMap.FLAG_SOLID
            content: TileSets.grass.wall_alt
        }
    }
    data: [=[
..........++++..
.......ppppppp..
...WWWpppppppp..
.WWWpppDppppW...
.WWpppppppWWW...
.WWpppppWWW.....
.WppppWWW.......
.WWWWWWW........]=]
}

M.skull_surrounded_dungeon = (args) -> {
    legend: make_legend {
        's': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN}
            content: TileSets.snake.floor
            on_placement: (map, xy) ->
                MapUtils.spawn_decoration(map, M._warning_skull, xy, 0, false)
        }
        'e': {
            add: {SourceMap.FLAG_SEETHROUGH}
            content: TileSets.snake.floor
            on_placement: args.enemy_placer
        }
        'D': {
            add: SourceMap.FLAG_SEETHROUGH
            content: TileSets.snake.floor
            on_placement: args.dungeon_placer
        }
        'W': {
            add: SourceMap.FLAG_SOLID
            content: TileSets.snake.wall
            matches_all: SourceMap.FLAG_SOLID
        }
        'd': {
            add: SourceMap.FLAG_SEETHROUGH
            content: TileSets.snake.floor
            on_placement: (map, xy) ->
                MapUtils.spawn_door(map, xy)
        }
        'w': {
            add: SourceMap.FLAG_SOLID
            content: TileSets.snake.wall_alt
        }
    }
    data: [=[.....WWW.
++..wwswW
+wwwwsesW
+dssdeDeW
+wwwwsesW
++..wwswW
.....WWW.]=]
}

-- args:
-- - boss_placer
-- - enemy_placer
-- - item_placer
-- - gold_placer
M.anvil_encounter = (args) -> {
    legend: make_legend {
        'a': {
            add: {SourceMap.FLAG_SEETHROUGH}
            content: TileSets.snake.floor
        }
        'b': {
            add: SourceMap.FLAG_SEETHROUGH
            content: TileSets.snake.floor_alt
        }
        'c': {
            add: SourceMap.FLAG_SEETHROUGH
            content: TileSets.pebble.floor_alt
        }
        'e': {
            add: {SourceMap.FLAG_SEETHROUGH}
            content: TileSets.pebble.floor_alt
            on_placement: args.boss_placer
        }
        'e': {
            add: {SourceMap.FLAG_SEETHROUGH}
            content: TileSets.pebble.floor_alt
            on_placement: args.enemy_placer
        }
        'E': {
            add: {SourceMap.FLAG_SEETHROUGH}
            content: TileSets.snake.floor
            on_placement: args.enemy_placer
        }
        '*': {
            add: {SourceMap.FLAG_SEETHROUGH}
            content: TileSets.pebble.floor_alt
            on_placement: args.gold_placer
        }
        'i': {
            add: {SourceMap.FLAG_SEETHROUGH}
            content: TileSets.pebble.floor_alt
            on_placement: args.item_placer
        }
        'W': {
            add: SourceMap.FLAG_SOLID
            content: TileSets.snake.wall
            matches_all: SourceMap.FLAG_SOLID
        }
        'd': {
            add: SourceMap.FLAG_SEETHROUGH
            content: TileSets.snake.floor
            on_placement: (map, xy) ->
                MapUtils.spawn_door(map, xy)
        }
        'w': {
            add: SourceMap.FLAG_SOLID
            content: TileSets.snake.wall_alt
        }
        '<': {
           add: SourceMap.FLAG_SOLID
           content: TileSets.snake.floor
           on_placement: (map, xy) ->
                MapUtils.spawn_decoration(map, M._anvil, xy)
        }
        's': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN}
            content: TileSets.snake.floor
            on_placement: (map, xy) ->
                MapUtils.spawn_decoration(map, M._warning_skull, xy, 0, false)
        }
    }
    data: [=[................................++...........
...............................WddW..........
...............................WssW..........
...............................WssW..........
...............................WssW..........
.............................WWWddWWWWW......
..........................WWWWWWaaaaWWWW.....
.......................WWWWWWWWaaaaaaWWWW....
................WWWWWWWWWWWWaaaaaaaaaaWWW....
...........WWWWWWWWWWWWWWaaaaaaaaaaaaaaWW....
.........WWWWWWWWWaaaaaaaaaaabbbbbaaaaaWW....
.......WWWWWWaaaaaaaaaaaaaabbbbbbbbaaaaWWW...
.....WWWWWWaaaaaaabbbaaaEEaaabbbbaaaaaaWWW...
....WWWWWaaa<aaaabbbbbaaaaaaaaaaaaaaaaWWWW...
...WWWWaaaaaaaaaaabbbaaaEEaaaaaaaaaaaWWWWW...
..WWWWaaaaabbbbaaaaaaaaaaaa<aaaaaaaWWWWW.....
.WWWWaaaaabbb*bbaaaaaaaaaaaaaaaaaWWWWWW......
.WWWaa<aaaaabbbaaaaaaaaaaaaWWWWWWWWWWWW......
WWWWaaaaaaaaaaaaaaaaaWWWWWWWWWWWWWWbWWWWW....
WWWWWWWWaaaaaaaaaaaaadaaaaaaaaadaabbbWWWWWW..
WWWWWWWWWWWWWWWWWWWaadaaaaaaaaadaaabbbbWWWWW.
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWaaaabbbbbWWWW
WWcicccccccccccccWWWWWWWWWWWaaaaaaEaaabbbbbWW
WWccccccccccc*cccccccWWWWWWWaaEaaaaaaaabbbbWW
WWcccccccceccccciccccWWWWWWaaaaaaaaaaaaabbbWW
WWccccccccccccccccccccdwaaaaaaaaaaEaaaaaabbWW
WWcc*ccciccBccccccecccdwaaaaaaaaaaaaaaaaaabWW
WWicecccccccccccccccc*dwaaaaaaaaaaaaaaaWWWWWW
WWWWccccceccccceccccWWWWWWWWWaaaaaWWWWWWWWWWW
..WWWWccccc*cccecccWWWWWWWWWWWWWWWWWWWWWW....
....WWWWccccccccccWWW........................
......WWWWWWWWWWWWWW.........................
........WWWWWWWWWWW..........................]=]
}


return M
