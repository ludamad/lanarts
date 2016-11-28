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
M.FLAG_NO_VAULT_SPAWN = SourceMap.FLAG_CUSTOM8

M._warning_skull = Display.image_load "features/sprites/warning.png"
M._rune_door_closed = Display.image_load "spr_doors/runed_door.png"
M._door_key1 = Display.image_load "spr_keys/door01.png"
M._door_key2 = Display.image_load "spr_keys/door02.png"
M._door_key3 = Display.image_load "spr_keys/door03.png"
M._anvil = Display.image_load "features/sprites/anvil.png"

-- Common definitions:
make_legend = (args, legend) ->
    args.tileset or= {}
    return table.merge {
        '.': { -- '.' means 'any tile'
            remove: {}
        }
        '+': {  -- '.' means 'walkable tile'
            remove: {}
            matches_none: {M.FLAG_NO_VAULT_SPAWN, SourceMap.FLAG_SOLID}
        }
        '*': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_VAULT_SPAWN}
            matches_none: {M.FLAG_NO_VAULT_SPAWN}
            content: args.tileset.floor
            on_placement: args.gold_placer
        }
        'd': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_VAULT_SPAWN}
            matches_none: {M.FLAG_NO_VAULT_SPAWN}
            content: args.tileset.floor
            on_placement: args.door_placer
        }
        'p': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN, M.FLAG_NO_ITEM_SPAWN, M.FLAG_NO_VAULT_SPAWN}
            matches_none: {M.FLAG_NO_VAULT_SPAWN}
            content: args.tileset.floor
            on_placement: (map, xy) ->
                {x, y} = xy
                if args.player_spawn_area
                    append map.player_candidate_squares, {x*32+16, y*32+16}
        }

    }, legend

M.ridge_dungeon = (args) -> {
    legend: make_legend args, {
        'p': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN, M.FLAG_NO_ITEM_SPAWN}
            content: args.tileset.floor
            on_placement: (map, xy) ->
                {x, y} = xy
                if args.player_spawn_area
                    append map.player_candidate_squares, {x*32+16, y*32+16}
        }

        'D': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_VAULT_SPAWN}
            content: args.tileset.floor
            on_placement: args.dungeon_placer
        }
        'W': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_VAULT_SPAWN}
            content: args.tileset.wall_alt
            matches_all: SourceMap.FLAG_SOLID
            matches_none: {M.FLAG_NO_VAULT_SPAWN}
        }
        'w': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_VAULT_SPAWN}
            content: args.tileset.wall_alt
            matches_none: {M.FLAG_NO_VAULT_SPAWN}
        }
    }
    data: random_choice {
        [=[
....+++.......
...wdddw......
..wwpppwww....
.wwppppppwwww.
.wppppppppppw.
.wpppDppppppw.
.wppppppppppw.
.wwwWWWWWwwww.
..............
]=],
        --Ridge dungeon:
        [=[
..........++++..
.......ppppppp..
...WWWpppppppp..
.WWWpppDppppW...
.WWpppppppWWW...
.WWpppppWWW.....
.WppppWWW.......
.WWWWWWW........
]=]
    }
}


M.sealed_dungeon = (args) -> table.merge M.ridge_dungeon(args), {
    data: [=[
.....wwwwwww....
...WWWpppppdppp+
.WWWpppppppdppp+
.WWpppppppWW....
.WW***WWWWW.....
.Wp*D*WWW.......
.WWWWWWW........]=]
}

M.skull_surrounded_dungeon = (args) -> {
    legend: make_legend args, {
        's': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN}
            content: args.tileset.floor
            on_placement: (map, xy) ->
                MapUtils.spawn_decoration(map, M._warning_skull, xy, 0, false)
        }
        'e': {
            add: {SourceMap.FLAG_SEETHROUGH}
            content: args.tileset.floor
            on_placement: args.enemy_placer
        }
        'D': {
            add: SourceMap.FLAG_SEETHROUGH
            content: args.tileset.floor
            on_placement: args.dungeon_placer
        }
        'W': {
            add: SourceMap.FLAG_SOLID
            content: args.tileset.wall
            matches_all: SourceMap.FLAG_SOLID
        }
        'w': {
            add: SourceMap.FLAG_SOLID
            content: args.tileset.wall_alt
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
    legend: make_legend args, {
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
        'B': {
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
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_VAULT_SPAWN}
            content: TileSets.snake.wall
            matches_all: SourceMap.FLAG_SOLID
            matches_none: {M.FLAG_NO_VAULT_SPAWN}
        }
        'd': {
            add: SourceMap.FLAG_SEETHROUGH
            content: TileSets.snake.floor
            on_placement: args.door_placer
        }
        'w': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_VAULT_SPAWN}
            content: TileSets.snake.wall_alt
            matches_none: {M.FLAG_NO_VAULT_SPAWN}
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
...............................wddw..........
...............................wssw..........
...............................wssw..........
...............................wssw..........
.............................wwwddwwwww......
..........................wwwwwwaaaawwww.....
.......................wwwwwwwwaaaaaawwww....
................wwwwwwwwwwwwaaaaaaaaaawww....
...........wwwwwwwwwwwwwwaaaaaaaaaaaaaaww....
.........wwwwwwwwwaaaaaaaaaaabbbbbaaaaaww....
.......WWWWWWaaaaaaaaaaaaaabbbbbbbbaaaaWWW...
.....WWWWWWaaaaaaabbbaaaEEaaabbbbaaaaaaWWW...
....WWWWWaaa<aaaabbbbbaaaaaaaaaaaaaaaaWWWW...
...WWWWaaaaaaaaaaabbbaaaEEaaaaaaaaaaaWWWWW...
..WWWWaaaaabbbbaaaaaaaaaaaa<aaaaaaaWWWWW.....
.WWWWaaaaabbbbbbaaaaaaaaaaaaaaaaaWWWWWW......
.WWWaa<aaaaabbbaaaaaaaaaaaaWWWWWWWWWWWW......
WWWWaaaaaaaaaaaaaaaaaWWWWWWWWWWWWWWbWWWWW....
WWWWWWWWaaaaaaaaaaaaadaaaaaaaaadaabbbWWWWWW..
WWWWWWWWWWWWWWWWWWWaadaaaaaaaaadaaabbbbWWWWW.
WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWaaaabbbbbWWWW
WWcicccccccccccccWWWWWWWWWWWaaaaaaEaaabbbbbWW
WWccccceccicc*cccccccWWWWWWWaaEaaaaaaaabbbbWW
WWcccecccceccccciccccWWWWWWaaaaaaaaaaaaabbbWW
WWcccccccccccc*cccccccdsaaaaaaaaaaEaaaaaabbWW
WWcc*ccciccBcccBccecccdsaaaaaaaaaaaaaaaaaabWW
WWicecccccccceccccccc*dsaaaaaaaaaaaaaaaWWWWWW
WWWWcccccecccBceccccWWWWWWWWWaaaaaWWWWWWWWWWW
..WWWWccccc*cccecccWWWWWWWWWWWWWWWWWWWWWW....
....WWWWccccccccccWWW........................
......WWWWWWWWWWWWWW.........................
........WWWWWWWWWWW..........................]=]
}

-- args:
-- - gold_placer
-- - enemy_placer
-- - store_placer
-- - item_placer
-- - rng 
M.small_random_vault = (args) -> {
    legend: make_legend args, {
        '!': {add: SourceMap.FLAG_SEETHROUGH, content: args.tileset.floor, on_placement: args.store_placer, matches_none: SourceMap.FLAG_SOLID}
        '*': {add: SourceMap.FLAG_SEETHROUGH, content: args.tileset.floor, on_placement: args.gold_placer, matches_none: SourceMap.FLAG_SOLID}
        'i': {add: SourceMap.FLAG_SEETHROUGH, content: args.tileset.floor, on_placement: args.item_placer, matches_none: SourceMap.FLAG_SOLID}
        'e': {add: SourceMap.FLAG_SEETHROUGH, content: args.tileset.floor, on_placement: args.enemy_placer, matches_none: SourceMap.FLAG_SOLID}
        'p': {add: SourceMap.FLAG_SEETHROUGH, content: args.tileset.floor, matches_none: SourceMap.FLAG_SOLID}
        'd': {
            add: SourceMap.FLAG_SOLID
            content: args.tileset.floor_alt
            on_placement: (map, xy) ->
                MapUtils.spawn_door(map, xy)
        }
        'x': {add: SourceMap.FLAG_SOLID, content: args.tileset.wall_alt}
    }
    data: random_choice {[=[....++++++
..++xxxx++
++xxx!ex+.
+xxe**ex+.
+xppppxx+.
+xddxxx++.
++++++++..]=],
        [=[..++++++++
.+xxxxxx++
++xppppx+.
+xxpiipx+.
+xpppppx+.
+xddxxxx+.
+++++++++.]=],
        [=[..++++++++
.+xxxxxx++
++xppppx+.
+xxppppx+.
+xppp!px+.
+xddxxxx+.
+++++++++.]=],
        [=[..++++++++
.+xxxxx+++
++xpppxx+.
+xxpxppx+.
+xppxp!x+.
+xddxxxx+.
+++++++++.]=],
        [=[..++++++++
..++xxxx++
++xxxpex+.
+xxeppex+.
+xppppxx+.
+xddxxx++.
++++++++..]=],
        [=[..++++++++
.+xxxxxx++
++xpppex+.
+xxpeepx+.
+xpppppx+.
+xddxxxx+.
+++++++++.]=],
        [=[..++++++++
.+xxxxxx++
++xpeppx+.
+xxpeepx+.
+xpppppx+.
+xddxxxx+.
+++++++++.]=],
        [=[..++++++++
.+xxxxx+++
++xpppxx+.
+xxpxpex+.
+xppxpex+.
+xddxxxx+.
+++++++++.]=]
    }
}

M.small_item_vault = (args) -> table.merge M.small_random_vault(args), {
    data: random_choice {[=[++++++
+xxxx+
+xipd+
+xxxx+
++++++]=],[=[+++++
+xxx+
+xid+
+xxx+
+++++]=]
    }
}

return M
