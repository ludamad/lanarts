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
-- TODO investigate whether we want this to simply be HAS_OBJECT:
M.FLAG_HAS_VAULT = SourceMap.FLAG_CUSTOM8 -- SourceMap.FLAG_HAS_OBJECT -- SourceMap.FLAG_CUSTOM8

M._warning_skull = Display.image_load "features/sprites/warning.png"
M._rune_door_closed = Display.image_load "spr_doors/runed_door.png"
M._closed_door_crypt = Display.image_load "spr_doors/closed_door_crypt.png"
M._door_key1 = Display.image_load "spr_keys/door01.png"
M._door_key2 = Display.image_load "spr_keys/door02.png"
M._door_key3 = Display.image_load "spr_keys/door03.png"
M._anvil = Display.image_load "features/sprites/anvil.png"

-- Common definitions:
UNSOLID_ADD = {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
make_legend = (args, legend) ->
    args.tileset or= {}
    return table.merge {
        '!': {add: UNSOLID_ADD, content: args.tileset.floor, on_placement: args.store_placer, matches_none: SourceMap.FLAG_SOLID}
        '.': { -- '.' means 'any tile'
            remove: {}
        }
        '+': {  -- '+' means 'walkable tile'
            remove: {}
            matches_none: {M.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}
        }
        '*': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            matches_none: {M.FLAG_HAS_VAULT}
            content: args.tileset.floor
            on_placement: args.gold_placer
        }
        'd': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            matches_none: {M.FLAG_HAS_VAULT}
            content: args.tileset.floor
            on_placement: args.door_placer
            remove: SourceMap.FLAG_SOLID
        }
        'p': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN, M.FLAG_NO_ITEM_SPAWN, M.FLAG_HAS_VAULT}
            matches_none: {M.FLAG_HAS_VAULT}
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
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN, M.FLAG_NO_ITEM_SPAWN, M.FLAG_HAS_VAULT}
            content: args.tileset.floor
            on_placement: (map, xy) ->
                {x, y} = xy
                if args.player_spawn_area
                    append map.player_candidate_squares, {x*32+16, y*32+16}
        }

        'D': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: args.tileset.floor
            on_placement: args.dungeon_placer
        }
        'W': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_HAS_VAULT}
            content: args.tileset.wall_alt
            matches_all: SourceMap.FLAG_SOLID
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'w': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_HAS_VAULT}
            content: args.tileset.wall_alt
            matches_none: {M.FLAG_HAS_VAULT}
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
        [=[
.........++.....
.....wwwwd++....
...wwwpppddw....
.wwwpppDpppw....
.wwpppppppww....
.wwpppppwww.....
.+dpppwww.......
.wwwwwww........
]=]
        [=[
.........++.....
.....wwwwd++....
...wwwpppddw....
.wwwpppDpppw....
.wwpppppppww....
.wwpppppwww.....
.wppppwww.......
.wwWWwww........
]=]
        [=[
................
.....wwww+......
...wwwppp++.....
.wwwpppDppw.....
.wwpppppppw.....
.wwpppppwww.....
.+ppppwwW.......
.+wwwwww........
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
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: TileSets.snake.floor
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'b': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: TileSets.snake.floor_alt
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'c': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: TileSets.pebble.floor_alt
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'B': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: TileSets.pebble.floor_alt
            on_placement: args.boss_placer
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'e': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: TileSets.pebble.floor_alt
            on_placement: args.enemy_placer
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'E': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: TileSets.snake.floor
            on_placement: args.enemy_placer
            matches_none: {M.FLAG_HAS_VAULT}
        }
        '*': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: TileSets.pebble.floor_alt
            on_placement: args.gold_placer
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'i': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            content: TileSets.pebble.floor_alt
            on_placement: args.item_placer
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'W': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_HAS_VAULT}
            content: TileSets.snake.wall
            matches_all: SourceMap.FLAG_SOLID
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'd': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_HAS_VAULT}
            remove: SourceMap.FLAG_SOLID
            content: TileSets.snake.floor
            on_placement: args.door_placer
            matches_none: {M.FLAG_HAS_VAULT}
        }
        'w': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_HAS_VAULT}
            content: TileSets.snake.wall_alt
            matches_none: {M.FLAG_HAS_VAULT}
        }
        '<': {
           add: {SourceMap.FLAG_SOLID, M.FLAG_HAS_VAULT}
           content: TileSets.snake.floor
           on_placement: (map, xy) ->
                MapUtils.spawn_decoration(map, M._anvil, xy)
           matches_none: {M.FLAG_HAS_VAULT}
        }
        's': {
            add: {SourceMap.FLAG_SEETHROUGH, M.FLAG_NO_ENEMY_SPAWN}
            content: TileSets.snake.floor
            on_placement: (map, xy) ->
                MapUtils.spawn_decoration(map, M._warning_skull, xy, 0, false)
            matches_none: {M.FLAG_HAS_VAULT}
        }
    }
    data: [=[................................++...........
...............................wddw..........
...............................waaw..........
...............................waaw..........
...............................waaw..........
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

M.big_encounter1 = (args) -> {
    legend: table.merge M.anvil_encounter(args).legend, {
        'P': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_ENEMY_SPAWN}
            content: TileSets.pebble.wall
        }
        'W': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_HAS_VAULT}
            content: TileSets.pebble.wall
            matches_all: SourceMap.FLAG_SOLID
            matches_none: {M.FLAG_HAS_VAULT}
        }
    }
    data: [=[
..............................................................................wwwwwwwwwww..................
.................................................................wwwwwwwwwwwwwwwwwwwwwwwwwwww..............
......................................................wwwwwwwwwwwwwwwwwwwwwwwwwwaaaaaaaawwwwwww............
.................................................wwwwwwwwwwwwwwwwwaaaaaaaaaaaaaaaEaaaaaaaaawwwww...........
...........................................wwwwwwwwwwwwaaaaaaaaaaaaaaaeaaaaaaaaaaaaaaaaaaaaaawww...........
........................................wwwwwwwwwwaaaaaaaaaaaEaaaa*aaaaaaaaaaaaaaa*aaa*aaaaaaaww...........
....................................wwwwwwwwwaaaaaaaeaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaww...........
..................................wwwwwwwaaaaaEaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaww...........
.................................wwwwwaaaaaaaaaaaaaaaaaaaaaaaEaaaaaaaaaaaaaaaEaaaaaaaEaaaaaaa.ww...........
................................wwwwaaaaaaaeaaaeeaaaaaaeaaaaaaaaaaaaaaaaaaaeaaaaaaaaaaaaeaaaawww...........
................................wwwaaaaaaaaaaaaaaaaaaaaaaaaeaaaaaaaaaaaaaaaaeaaaaaaaaaaaaaaawwww...........
................................wwwaaaaaaEaaaaaaaaaaaaaaaaaaaaaaaaaaaaa*aaaaaaaaaaaaaaaaaawwwwww...........
................................wwwaaaaaaaaaaaaaaaaaaeaaiaaaaa*aaawwwwwwwwwwwwwwwwwwwwwwwwwwwwww...........
.................................wwaaaaaaaiaaaeaaaaaaaaawwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww..ww...........
.................................wwaaaaaeaaaaaaaeaaawwwwwwwwwwwwwwwwwwwwwwww...............................
.................................wwwwwwaeaaaaaaaawwwwwwwwwwwwwww...........................................
.................................wwwwwwaaaaaaaaaawwwwwwww..................................................
.................................wwwwwwaaawwwwwwww.........................................................
.................................wwwwwwaaawwwwwwww.........................................................
.................................wwwwwwdddwwwwwwww.........................................................
.................................wwwwww+++wwwwwwww.........................................................]=]
}

M.big_encounter2 = (args) -> {
    legend: table.merge M.anvil_encounter(args).legend, {
        'P': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_ENEMY_SPAWN}
            content: TileSets.pebble.wall
        }
        'W': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_HAS_VAULT}
            content: TileSets.pebble.wall
            matches_all: SourceMap.FLAG_SOLID
            matches_none: {M.FLAG_HAS_VAULT}
        }
    }
    data: [=[
.............wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww...
......wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.
....wwwwwwwwwwwbbbbbbbbbbbbbbbbbbbbbbbbbbiwwwwwwbbbbbbbbbbww
.wwwwwwwbbbbbbbbbebbbbbebbbbbbbbbbbbbbbbbbbbbbbbbbbbebbbbbbw
wwwwwwbbbbbbbbbbbbbbbbbbbb*bbbbbbbbbebbbbbbbbbbbbbbbbbebbbbw
wwwbbbbbbbebbbbbbbbbbbbbbbbbbbbbbbbbbibbbbbbbbbbbbbebbbbbbbw
wwbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbeebbw
wwbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbw
wwwwbbbbbbbbbbbbbbwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwbbbbbw
wwwwwwbbbbbbbbbbwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwbbbw
..wwwwwwaaaaaaaawww.......................wwwwwwwwwwwwwwwwbw
....wwwwwaaaaaawww....................................wwwwww
......wwwwaaaaaww.......................................wwww
........wwwaaawww.........................................w.
........wwwaaawww...........................................
.........wwdddww............................................
.........ww+++ww............................................
]=]
}


M.tunnel = (args) -> {
    legend: table.merge M.anvil_encounter(args).legend, {
        'w': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_ENEMY_SPAWN, M.FLAG_HAS_VAULT}
            matches_none: {M.FLAG_HAS_VAULT}
            content: TileSets.snake.wall
        }
        'G': {
            remove: {}
            matches_none: {M.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}
            matches_content: TileSets.pebble.floor
        }
    }
    data: random_choice {
--[=[
--.....wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.
--.wwwwwwccccccccccc*cccccccccccccccc*cccccccccccw.
--+dccccceccccccc*cccccccccccccccccecccccccccccccd+
--+dcccccccccccccccccccccccccccccccccccccccccccccd+
--.wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.
--]=]
        [=[
.....wwwwwww.....................................
.wwwwwwccccw.....................................
Gdcccccecccw.....................................
.wwwwwwwcccw.....................................
.....wwwccwwwwwwwwwwwwwwwwwwwwwww................
......wwccccccccccccccccccccccccwwwwwwwwwwwwwwww.
......wwwwwwwwwccccccccccccccccccccccccccccccccd+
............wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww.
]=]
    }
}

M.stone_henge = (args) -> {
    legend: table.merge M.anvil_encounter(args).legend, {
        'w': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_ENEMY_SPAWN, M.FLAG_HAS_VAULT}
            matches_none: {M.FLAG_HAS_VAULT}
            content: TileSets.snake.wall
        }
        'p': {add: UNSOLID_ADD, content: TileSets.snake.floor, matches_none: SourceMap.FLAG_SOLID}
        'P': {add: UNSOLID_ADD, content: TileSets.grass.floor_alt2, matches_none: SourceMap.FLAG_SOLID}
        'i': {add: UNSOLID_ADD, content: TileSets.grass.floor_alt2, on_placement: args.item_placer, matches_none: SourceMap.FLAG_SOLID}
    }
    data: random_choice {[=[
.w..w..
wwppww.
.pppp+.
.pppp+.
wwppww.
.w..w..
]=]
      [=[
..PPPPPPPPPPPP...
.PPPPPPPPPPPPPP..
.PPPPPPPPPPPPPPP.
.PPPPPPPPPPPPPPP.
.PPPPPPPPPPPPPPP.
PPPPPPPPPPPPPPPP.
PPPPPPPPPPPPPPPP.
PPPPPPPPPPPPPPPP.
PPPPPPPPPPPPPPP..
.PPPPPPPPPPPPP...
]=]
        [=[
+PPPPPPPPP+
PPwwPwPwwPP
PPwPPwPPwPP
PPPPPPPPPPP
PPwwPiPwwPP
PPPPPPPPPPP
PPwPPwPPwPP
PPwwPwPwwPP
+PPPPPPPPP+
]=]
        [=[
ww.w.ww.
wPPwPPw.
.PPPPP+.
wwPiPww.
.PPPPP+.
wPPwPPw.
ww.w.ww.
]=]
        [=[
.w...w..
wwPPPww.
.PPPPP+.
.PPiPP+.
.PPPPP+.
wwPPPww.
.w...w..
]=]
    }
}


M.cavern = (args) -> {
    legend: table.merge M.anvil_encounter(args).legend, {
        'w': {
            add: {SourceMap.FLAG_SOLID, M.FLAG_NO_ENEMY_SPAWN, M.FLAG_HAS_VAULT}
            matches_none: {M.FLAG_HAS_VAULT}
            content: TileSets.snake.wall
        }
    }
    data: [=[
..........wwwwwwwww.......
.....wwwwwwaaaaaawwwwww...
..wwwwaaaaaaaaaaaaaaaawww.
wwwaaaaaaaa!aaaaaaaaaaaaww
waaaaaaaaaaaaaaaaaaaaaaww.
wwwaaaaaaaaaaaaaaaaaaaww..
wwwwwwwwwww....++.........
]=]
}


-- args:
-- - gold_placer
-- - enemy_placer
-- - store_placer
-- - item_placer
-- - rng 
M.small_random_vault = (args) -> {
    legend: make_legend args, {
        '*': {add: UNSOLID_ADD, content: args.tileset.floor, on_placement: args.gold_placer, matches_none: UNSOLID_ADD}
        'i': {add: UNSOLID_ADD, content: args.tileset.floor, on_placement: args.item_placer, matches_none: UNSOLID_ADD}
        'e': {add: UNSOLID_ADD, content: args.tileset.floor, on_placement: args.enemy_placer, matches_none: UNSOLID_ADD}
        'p': {add: UNSOLID_ADD, content: args.tileset.floor, matches_none: UNSOLID_ADD}
        'd': {
            add: UNSOLID_ADD 
            remove: SourceMap.FLAG_SOLID
            content: args.tileset.floor_alt
            on_placement: (map, xy) ->
                MapUtils.spawn_door(map, xy)
        }
        'x': {add: {SourceMap.FLAG_SOLID, M.FLAG_HAS_VAULT}, content: args.tileset.wall_alt, matches_none: M.FLAG_HAS_VAULT}
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
+++++++++.]=],
        [=[
.xxxxxx...
xxipppxxxx
xpepipexxx
xxppppeexx
.xddxppxxx
.x++xxxxxx]=]
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

M.small_item_vault_multiway = (args) -> table.merge M.small_random_vault(args), {
    data: random_choice {
[=[
..+..
.xdx.
+did+
.xdx.
..+..]=]
    }
}


return M
