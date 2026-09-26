----
-- The rootways' features: life among the roots. Hollow giant trees, rabbit warrens and badger setts
-- dug into yellow earth, a stone root cellar, rings of giant toadstools, a woodcutter's camp, a
-- fallen giant across the meadow, spider-webbed root tangles and traders up trees and down holes.
----

-- Dark bark around dark wood floors under the canopy, olive roots
TRUNK = {floor: "orc.floor", floor_alt: "grass.dirt", path: "grass.dirt", wall: "pebble.wall", wall_alt: "orc.wall"}
-- Yellow earth, packed pebbles and dirt
BURROW = {floor: "brick.floor", floor_alt: "grass.dirt", path: "grass.dirt", wall: "lair.wall", wall_alt: "orc.wall"}
-- Dark stone and brick below ground
CELLAR = {floor: "crypt.floor_alt", floor_alt: "brick.floor", path: "grass.dirt", wall: "brick.wall_alt", wall_alt: "brick.wall"}
-- Pink and orange toadstool caps, lush grass outside and damp ground within
TOADSTOOL = {floor: "grass.floor_alt1", floor_alt: "orc.floor_alt", wall: "spider.wall_alt", wall_alt: "spider.wall"}
-- Log piles and a canvas tent on trampled dirt and dry grass
CAMP = {floor: "grass.dirt", floor_alt: "grass.floor_alt2", path: "grass.dirt", wall: "brick.wall", wall_alt: "orc.wall"}
-- Webbed roots
WEB = {floor: "spider.floor", floor_alt: "spider.floor_alt", path: "grass.dirt", wall: "orc.wall", wall_alt: "spider.wall"}

return {
    {
        name: "rootways_hollow_tree", title: "Hollow Giant", kind: "treasure", themes: {"rootways"}
        blurb: "A giant tree gone hollow, its roots splayed out; something shines inside the trunk"
        palette: TRUNK
        layouts: {
            [[
..TT#TT..
.T##W##T.
T##ooo##T
T#oo*io#T
T#ooooo#T
T##o,o##T
.T##d##T.
.W":::"W.
...+++...
]]
            [[
...TTTTT...
.TT#####TT.
TT##ooo##TT
T##oi,*o##T
T#oo,,,oo#T
T##o,,,o##T
.T##o,o##T.
.WW##d##WW.
W."":::"".W
....+++....
]]
            [[
.TT####...
T##oooW...
T#o,i,oW..
T#o,,,o:+.
T##o*oo:+.
.T####W...
]]
        }
    }
    {
        name: "rootways_root_cellar", title: "Root Cellar", kind: "treasure", themes: {"rootways"}
        blurb: "A stone-lined cellar dug in under the roots, its shelves not quite emptied"
        palette: CELLAR
        layouts: {
            [[
.TTTTTTT.
T#######T
T#i,o,*#T
T#o###o#T
T#o,o,o#T
T###d###T
...":"...
...+++...
]]
            [[
.#######..
.#*o,oi#..
.#o#####..
.#o#......
.#d#......
.":"......
.+++......
]]
            [[
.#######.
.#,o*o,#.
.#o###o#.
##o###o##
#i,###,*#
##d###d##
..:":::..
....++...
]]
        }
    }
    {
        name: "rootways_warren", title: "Rabbit Warren", kind: "treasure", themes: {"rootways"}
        blurb: "A mound riddled with burrows, where shiny things get dragged underground"
        palette: BURROW
        legend: {
            '*': {spawn: "gold", chance: 0.5}
        }
        layouts: {
            [[
...#####....
.###o*#####.
.#oo,#,o*o#.
##*#,,,#o##.
#o,,,#,,,o#.
###,#i#,###.
..#,,,,,#...
..##,:,##...
....+++.....
]]
            [[
.####.####.
##*o###o*##
#o,,#,#,,o#
##,#,i,#,##
.#,,,#,,,#.
.##,,,,,##.
..###:###..
....+++....
]]
            [[
....####.......
..###*o####....
.##,,#,,,*##...
.#i,###,##,##..
.##,,,,,,,,:+..
..####,####:+..
.....###.......
]]
        }
    }
    {
        name: "rootways_mushroom_ring", title: "Toadstool Ring", kind: "landmark", themes: {"rootways"}
        blurb: "Giant pink toadstools grown in a ring on swampy ground"
        palette: TOADSTOOL
        base: "floor_alt"
        legend: {
            'i': {tile: "floor_alt", spawn: "item", chance: 0.5}
        }
        layouts: {
            [[
....+++....
..ooo:ooo..
.oo#o,o#oo.
.o#,,,,,#o.
.oo,,,,,oo.
.o#,,i,,#o.
.oo,,,,,oo.
.o#,,,,,#o.
.oo#o#o#oo.
..ooooooo..
...........
]]
            [[
...+++....
..oo:oo...
.o#o,oW...
.oo,,,,#o.
.W,,i,,oo.
.oo,,,,Wo.
..o#oo#...
...oooo...
..........
]]
            [[
...........
.oWoo...oo.
.o,,#o.o#o.
.oo,,ooo,o.
..W,,o,,Wo.
.oo,,o,,oo.
.o#oo:oo#..
..oo+++oo..
]]
        }
    }
    {
        name: "rootways_woodcutter_camp", title: "Woodcutter's Camp", kind: "landmark", themes: {"rootways"}
        blurb: "Log piles, a chopping block and a tent in a trampled clearing"
        palette: CAMP
        legend: {
            'i': {spawn: "item", chance: 0.5}
        }
        layouts: {
            [[
.TT.TTTTT.TT.
T##o#####o##T
To,,oooooo,oT
Too,WWWo,oa,T
T,o,WiWo,o,oT
To,oo,oo,,o,T
.T,oTo,oT,oT.
..,o:::o,....
....+++......
]]
            [[
..TTTTTTT..
.T###o###T.
Too,,o,,ooT
To,a,o,WWWT
T,,o,:,WiW.
.T,##o,,,:+
..T##o,,:+.
...TTo:::..
.....+++...
]]
            [[
..........TT.
.###.###.T,oT
.,o,o,o,oo,aT
.o,WWW,,,o,oT
.o,WiW,o,,oT.
..,o,o,::oT..
....TT:+:....
]]
        }
    }
    {
        name: "rootways_fallen_giant", title: "Fallen Giant", kind: "landmark", themes: {"rootways"}
        blurb: "A giant tree lies across the meadow, torn roots and all; its hollow end hides a little gold"
        palette: TRUNK
        base: "ground_alt"
        legend: {
            'o': {tile: "floor"}
            '*': {tile: "floor", spawn: "gold"}
            'i': {tile: "floor", spawn: "item", chance: 0.5}
        }
        layouts: {
            [[
.W..W"""""""""""".
WWWW############".
++":o,oooooo,o*#".
WWWW############".
.W..W"""""""""""".
]]
            [[
.hTTTh.
hTT"TTh
.T"""T.
..#*#..
..#o#..
..#,#..
..#o#..
..#o#..
.W#:#W.
WW":"WW
.W+++W.
]]
            [[
.W.W..............
WWWW#####""######.
"":o,o*o:"":o,i,#.
WWWW#####""######.
.W.W.""+++""......
]]
        }
    }
    {
        name: "rootways_treehouse", title: "Treehouse Trader", kind: "store", themes: {"rootways"}
        blurb: "A trader keeps shop on a plank floor up in the roots of a giant tree"
        palette: TRUNK
        layouts: {
            [[
..TTTTTTT..
.TT#####TT.
TT##ooo##TT
T##oo$oo##T
T#,ooooo,#T
T##oooo,##T
.T###d###T.
.W":::::"W.
....+++....
]]
            [[
.TTTTTT...
TT####TT..
T##o$o##T.
T#,ooo,#T.
T##oo,##T.
.T##o###..
..W#d#W...
...:::....
...+++....
]]
            [[
..TTTTTTTTT..
.TT###T###TT.
TT##o##,o##TT
T##oo$oo,oo#T
T#o,oo##ooo#T
TT##,o##,o##T
.TT##d##d##T.
...W:::::W...
.....+++.....
]]
        }
    }
    {
        name: "rootways_burrow_market", title: "Burrow Market", kind: "store", themes: {"rootways"}
        blurb: "A trader has dug out a snug chamber under a root mound and sells from it"
        palette: BURROW
        layouts: {
            [[
..#####....
.##o,o###..
##o,$,oo##.
#o,o,o,o,#.
##o,o###,#.
.####.#,,##
.......#,:+
........::+
]]
            [[
...#####...
..##,o,##..
.##o,$,o##.
.#o,o,o,o#.
.##o,o,o##.
..###,###..
....#:#....
....+++....
]]
            [[
.#####.........
##o,o####......
#o,$,o,o,####..
##o,o###,o,,#..
.#####.#,,#:#..
.......###::+..
..........++...
]]
        }
    }
    {
        name: "rootways_badger_sett", title: "Badger Sett", kind: "encounter", themes: {"rootways"}
        blurb: "Tunnels under an earth bank, and what lives in them guards its hoard"
        palette: BURROW
        enemies: {"Giant Rat", "Hound"}
        brutes: {"Mouther"}
        legend: {
            'k': {tile: "floor_alt", decor: "skull"}
        }
        layouts: {
            [[
..#######...
.##e,o*###..
##o,#,o,e##.
#i,##k##,o#.
##o,#,,#e##.
.##,,e,,##..
..###:###...
....+++.....
]]
            [[
.####..####.
##o*##.#i,##
#e,,####,,e#
##,o,E,,o,##
.#,##,##,##.
.##..:..###.
....+++.....
]]
            [[
..###....###..
.##i#....#*##.
.#e,##..##,e#.
.##,,####,,##.
..##,,e,,,##..
...###k,###...
.....#::#.....
.....+++......
]]
        }
    }
    {
        name: "rootways_spider_roots", title: "Webbed Roots", kind: "encounter", themes: {"rootways"}
        blurb: "Spiders have webbed over a tangle of roots; a traveller's pack hangs in the middle"
        palette: WEB
        enemies: {"Giant Spider"}
        legend: {
            'k': {tile: "floor_alt", decor: "skull"}
        }
        layouts: {
            [[
..W#.....#W..
.W,,#W.W#,,W.
.#,e,,,,,e,#.
..,,,#i#,,,..
.W#,,,*,,,#W.
..#,e,,,k,#..
...,,,:,,,...
....+++......
]]
            [[
.#W....W#.
#,,#WW#,e#
W,e,,,,,,W
.#,,i,,k#.
.W,,*,,,W.
.#,e,,,,#.
..W,:::W..
...+++....
]]
        }
    }
}
