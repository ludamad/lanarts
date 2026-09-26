----
-- The ringwood's features: druid country around the central mesa. Moss-green standing stones on
-- lush lawns, fairy rings with crystal hearts, marble shrines the moss is taking back, caves in
-- rock outcrops like the mesa's flanks, offering cairns, and a hounds' cairn and a barrow to rob.
----

-- Green carved stones and mossy brick on bare earth and lush grass
GROVE = {floor: "grass.dirt", floor_alt: "grass.floor_alt1", path: "grass.dirt", wall: "moss.wall_alt", wall_alt: "moss.wall"}
-- Marble overgrown with moss
SHRINE = {floor: "marble.floor", floor_alt: "grass.floor_alt1", path: "grass.dirt", wall: "marble.wall", wall_alt: "moss.wall"}
-- Crystal set in lush grass
FAIRY = {floor: "grass.floor_alt1", floor_alt: "temple.floor", wall: "crystal.wall", wall_alt: "marble.wall_alt"}
-- The mesa's own snake stone, sandy floors and pebbles
MESA = {floor: "quarry.floor", floor_alt: "brick.floor", path: "grass.dirt", wall: "grass.wall_alt", wall_alt: "quarry.wall", rock: "grass.wall_alt"}
-- Grey earth, moss and old catacomb stone
BARROW = {floor: "quarry.floor", floor_alt: "crypt.floor_alt", path: "grass.dirt", wall: "moss.wall", wall_alt: "crypt.wall"}

return {
    {
        name: "ringwood_druid_circle", title: "Druid Circle", kind: "landmark", themes: {"ringwood"}
        blurb: "Green standing stones in a ring on a lush lawn, around a weathered idol"
        palette: GROVE
        base: "ground_alt"
        legend: {
            '*': {tile: "ground_alt", spawn: "gold", chance: 0.5}
        }
        layouts: {
            [[
.....+++.....
.."""""""""..
..""#":"#""..
.""""":""""".
."#""":"""#".
.""""":""""".
."""":s:"""".
.""""""""""".
."#"""""""#".
.""""""""""".
..""#"""#""..
..."""""""...
.............
]]
            [[
...........
..#"#"#"#..
.#"""""""#.
.#"""s"""#.
.#"""*"""#.
..#"":""#..
....+:+....
]]
            [[
...+++...
.#"":""#.
.."":""..
.#"":""#.
.."":""..
.#"":""#.
.""":""".
."""s""".
.#""*""#.
..#"""#..
.........
]]
        }
    }
    {
        name: "ringwood_fairy_ring", title: "Fairy Ring", kind: "landmark", themes: {"ringwood"}
        blurb: "A ring of shrubs or crystal pillars around a glittering floor; something was left in the middle"
        palette: FAIRY
        base: "ground"
        legend: {
            'i': {tile: "floor_alt", spawn: "item", chance: 0.6}
            'h': {tile: "floor", decor: "shrub"}
        }
        layouts: {
            [[
.....+++.....
.."""""""""..
.""hhh"hhh"".
."hh,,,,,hh".
."h,,,,,,,h".
."h,,,,,,,h".
."h,,,i,,,h".
."h,,,,,,,h".
."h,,,,,,,h".
."hh,,,,,hh".
.""hhh"hhh"".
.."""""""""..
.............
]]
            [[
....+++....
.."""""""..
.""#"""#"".
."#",,,"#".
."",,,,,"".
."",,i,,"".
."",,,,,"".
."#",,,"#".
.""#"""#"".
.."""""""..
...........
]]
            [[
....+++....
..h"""""h..
.hh,,,,,hh.
.h,,,#,,,h.
.h,,#i#,,h.
.h,,,,,,,h.
..hh,,,hh..
....hhh....
...........
]]
        }
    }
    {
        name: "ringwood_sacred_grove", title: "Sacred Grove", kind: "landmark", themes: {"ringwood"}
        blurb: "A marble walk into a ring of old trees, where a statue stands on a lawn"
        palette: {floor: "marble.floor", path: "marble.floor", wall: "marble.wall"}
        base: "ground_alt"
        legend: {
            'S': {tile: "floor", decor: "statue"}
        }
        layouts: {
            [[
.....+++.....
.....:::.....
...TTT:TTT...
..TT"":""TT..
.TT""":"""TT.
.T"""ooo"""T.
.T""ooSoo""T.
.T"""ooo"""T.
.TT"""""""TT.
..TT"""""TT..
...TTTTTTT...
.............
]]
            [[
.........++.
.TTTTTTT.::.
TT""""""T:..
T"h"ooo"":..
T""ooSoo:::.
T"h"ooo""...
TT"""""""T..
.TTTTTTTTT..
]]
            [[
...+++...
...:::...
.TT:::TT.
T"":S:""T
T"h:::h"T
T""":"""T
TT""s""TT
.TTTTTTT.
.........
]]
        }
    }
    {
        name: "ringwood_hermit_stall", title: "Hermit's Stall", kind: "store", themes: {"ringwood"}
        blurb: "A hermit druid trades from a mossy lean-to under the trees"
        palette: GROVE
        layouts: {
            [[
..TTTTT..
.TT###TT.
.T#o$o#T.
.ThooohT.
.gg,,,gg.
..gg:gg..
...+++...
]]
            [[
.TTTTTTT.
TT"""""TT
T""h$h""T
T"""o"""T
T""ooo""T
.""":""".
...+++...
]]
            [[
...WWW...
..WoooW..
.Woo$ooW.
.Wooo,oW.
..W,ooW..
...WdW...
...g:g...
...+++...
]]
        }
    }
    {
        name: "ringwood_druid_lodge", title: "Druid's Lodge", kind: "store", themes: {"ringwood"}
        blurb: "A long hall of marble and mossy brick where the druids sell what the grove gives them"
        palette: SHRINE
        layouts: {
            [[
.WWWWWWWWW.
.Wo,ooo,oW.
.Wos,$,soW.
.Wo,ooo,oW.
.WWWWdWWWW.
....":"....
....+++....
]]
            [[
.WWWWWWW...
.Wo,oooWW..
.Wo$o,ooW..
.WWWoo,oW..
...Wo,oWW..
...WWdWW...
....":"....
....+++....
]]
            [[
..WWWWW..
.WWo,oWW.
.Wo,$,oW.
.Ws,o,sW.
.Wo,o,oW.
.WWodoWW.
..W:::W..
...+++...
]]
        }
    }
    {
        name: "ringwood_moss_shrine", title: "Mossy Shrine", kind: "treasure", themes: {"ringwood"}
        blurb: "A small marble shrine the moss has half swallowed, with offerings still on the altar"
        palette: SHRINE
        layouts: {
            [[
.#W#W#W#.
.W,oio,W.
.#o,s,o#.
.W,o*o,W.
.#,ooo,#.
.WW#d#WW.
...":"...
...+++...
]]
            [[
.#W..#W#.
#o,"",oW.
Wo,s"o,#.
#"oio,"..
W,o*""":+
.#",,"::+
..W"#W...
]]
            [[
...WWW...
..W,i,W..
.W,o,o,W.
.#o#s#o#.
.#,o,o,#.
.WW,*,WW.
..WWdWW..
...:::...
...+++...
]]
        }
    }
    {
        name: "ringwood_flank_cave", title: "Mesa Cave", kind: "treasure", themes: {"ringwood"}
        blurb: "A cave in a spur of the mesa's snake stone, where something bright was hidden"
        palette: MESA
        layouts: {
            [[
..RRRRRRR...
.RRooo,RRRR.
RRo,*o,,oRR.
Ro,,RR,o*oR.
RRoi,RRo,RR.
.RR,oo,RR...
..RRo:RR....
....++......
]]
            [[
.RRRRR.....
RRo*oRR....
Ro,i,oRRR..
RRo,,,,oRR.
.RRRRo,,oR.
....RR,o:+.
.....RR::+.
......RR...
]]
            [[
....RRRRR....
..RRRo*oRRR..
.RRo,,,,,oRR.
.Ro,,RRR,,oR.
.Ri,RR.RR,*R.
.RR,R...Ro,R.
..R:R...R:RR.
..+++...+++..
]]
        }
    }
    {
        name: "ringwood_offering_cairns", title: "Offering Cairns", kind: "treasure", themes: {"ringwood"}
        blurb: "Little cairns on a dry lawn, with coins left at their feet"
        palette: MESA
        base: "ground_dry"
        legend: {
            "'": {tile: "ground_dry"}
            '*': {tile: "ground_dry", spawn: "gold", chance: 0.4}
        }
        layouts: {
            [[
..'''''''..
.'R*'''*R'.
.'''''''''.
.R*''i''*R.
.'''''''''.
.'R*'''*R'.
..''':'''..
....+++....
]]
            [[
.....'''.....
..R*'''''*R..
.'''''R'''''.
''''*RiR*''''
.'''''''''''.
..'''':''''..
.....+++.....
]]
            [[
.RR''''''....
.R*'''R*''...
.''''''''''..
..''R*'''R*'.
...''''i'''..
....'''R*''..
......':'''..
......+++....
]]
        }
    }
    {
        name: "ringwood_hound_cairn", title: "Hounds' Cairn", kind: "encounter", themes: {"ringwood"}
        blurb: "A pack of hounds lairs by a stone cairn with a chamber in its side"
        palette: MESA
        base: "ground_dry"
        enemies: {"Hound"}
        legend: {
            "'": {tile: "ground_dry"}
            'o': {tile: "floor"}
            'i': {tile: "floor", spawn: "item"}
            '*': {tile: "floor", spawn: "gold"}
        }
        layouts: {
            [[
....+++....
..''':''''..
.'e'''''e'.
.''RRRRR''.
.'RRo*oRR'.
.'RRoioRR'.
.'k'RoR'''.
.''e'o''k'.
..'''''''..
...........
]]
            [[
..RRRRRRR..
.RRo*i*oRR.
.RRRoooRRR.
..'k'o'e'..
.''e''''k'.
.'''e'''''.
..'''+++'..
]]
            [[
.....RRR....
...RRRoRRR..
..RRo*i*RR..
..'RRoRRR'..
.''e'''''e'.
.'k'''e'''..
..'''''k''..
....'++''...
]]
        }
    }
    {
        name: "ringwood_barrow", title: "Druid Barrow", kind: "encounter", themes: {"ringwood"}
        blurb: "A turf-covered barrow among tombstones; its dead druid doesn't rest"
        palette: BARROW
        brutes: {"Skeleton", "Zombie"}
        legend: {
            't': {tile: "ground", decor: "tombstone"}
        }
        layouts: {
            [[
...TTTTTTT...
..TWWWWWWWT..
.TWWo,i,oWWT.
.TWo,,E,,oWT.
.TWWo,*,oWWT.
..TWWWdWWWT..
..t""g:g""t..
...g"e:e"g...
.....+++.....
]]
            [[
.TTTTTTTT....
TWWWWWWWWT...
TW*,o,oiWT...
TWE,o,o,WT...
TWWWWd,WWTT..
.t"gg:gg"t...
..t"e:gt"g...
...g:::g.....
....+++......
]]
            [[
....WWWWW....
..WWWoioWWW..
.WWoo,E,ooWW.
.W*oo,,,oooW.
.WWWo,,,oWWW.
..tWWWdWWWt..
..g"e:e"g....
.t"g":"g"t...
....+++......
]]
        }
    }
}
