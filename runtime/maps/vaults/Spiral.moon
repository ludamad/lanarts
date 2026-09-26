----
-- The spiral overworld's features: a pilgrims' road. White marble shrines and waystations, a
-- cobalt oracle's pavilion and labyrinth, a spiral cairn, a sundial court, penitents' cells, a holy
-- spring, and a reliquary with its wardens.
----

-- Blue-black cobalt with crystal pillars
ORACLE = {floor: "cobalt.floor", floor_alt: "temple.floor", wall: "cobalt.wall_alt", wall_alt: "cobalt.wall"}
-- A maze of labyrinth tiles in cobalt stone
LABYRINTH = {floor: "stonework.floor_alt", floor_alt: "cobalt.floor", wall: "cobalt.wall", wall_alt: "marble.wall_alt"}
-- Grey penitents' stone
CELLS = {floor: "crypt.floor_alt", floor_alt: "marble.floor_alt", wall: "quarry.wall_alt", wall_alt: "marble.wall_alt"}
-- A heap of grey stones on sand
CAIRN = {floor: "quarry.floor", floor_alt: "quarry.floor_alt", wall: "quarry.wall_alt", wall_alt: "marble.wall_alt"}
-- White marble paving, carved reliefs
COURT = {floor: "marble.floor", floor_alt: "marble.floor_alt", path: "marble.floor", wall: "marble.wall", wall_alt: "marble.wall_alt"}

return {
    {
        name: "spiral_waystation", title: "Pilgrims' Waystation", kind: "store", themes: {"spiral"}
        blurb: "A marble hostel for pilgrims on the road in, with a keeper selling supplies"
        palette: "shrine"
        layouts: {
            [[
.WWWWWWWWWWW.
.W,o,o,o,o,W.
.W,o,o,o,o,W.
.Wooooooooodoo+
.Wsoooooooodoo+
.WWWWoo$oWWW.
....WWWWWW...
]]
            [[
..WWWWWWW..
.WW,o,o,WW.
.W,oo$oo,W.
.W,ooooo,W.
.WWsodosWW.
...::o::...
...::+::...
]]
            [[
.#####.#####.
.#,,,###,,,#.
.#,o,,$,,o,#.
.#,,,#o#,,,#.
.###o#o#o###.
...#ooooo#...
...WsdddsW...
.....+++.....
]]
        }
    }
    {
        name: "spiral_oracle", title: "Oracle's Pavilion", kind: "store", themes: {"spiral"}
        blurb: "A pavilion of crystal pillars on a cobalt floor, where the oracle trades in portents"
        palette: ORACLE
        layouts: {
            [[
...#.o.#...
..oo,,,oo..
.#o,,,,,o#.
.o,,,$,,,o.
.#o,,,,,o#.
..oo,,,oo..
...#.+.#...
]]
            [[
.#o#o#o#.
.o,,,,,o.
.#,,$,,#.
.o,,,,,o.
.#o#+#o#.
]]
            [[
....#o#....
..#ooooo#..
.#oo,,,oo#.
.oo,,W,,oo.
#o,,W$W,,o#
.oo,,,,,oo.
.#oo,,,oo#.
..#oo+oo#..
....#o#....
]]
        }
    }
    {
        name: "spiral_reliquary", title: "Reliquary", kind: "treasure", themes: {"spiral"}
        blurb: "A sealed marble reliquary between two saints"
        palette: "shrine"
        layouts: {
            [[
..WWWWW..
.WW,i,WW.
.W,,*,,W.
.WW,,,WW.
..WWdWW..
.s:::::s.
...:+:...
]]
            [[
.WWWWWWWWW.
.W,i,W,*,W.
.W,,,W,,,W.
.WWdWWWdWW.
.soooooos..
..o+++o....
]]
            [[
....WWW....
..WWW,WWW..
.WW,,i,,WW.
.W,*,,,i,W.
.WW,,,,,WW.
..WWWdWWW..
..s::o::s..
.....+.....
]]
        }
    }
    {
        name: "spiral_cells", title: "Penitents' Cells", kind: "treasure", themes: {"spiral"}
        blurb: "A row of bare stone cells; some penitents left their worldly goods behind"
        palette: CELLS
        legend: {
            '*': {spawn: "gold", chance: 0.6}
            'i': {spawn: "item", chance: 0.6}
        }
        layouts: {
            [[
.#########.
.#*#o#i#*#.
.#o#o#o#o#.
.#d#d#d#d#.
.#oooooooo+
.##########
]]
            [[
.#####.
.#i#*#.
.#d#d#.
+ooooo#
.#d#d#.
.#*#i#.
.#####.
]]
            [[
..#####..
..#,*,#..
.##d#d##.
.#i:::*#.
.##d#d##.
..#,,,#..
..##d##..
....+....
]]
        }
    }
    {
        name: "spiral_labyrinth", title: "Walking Court", kind: "treasure", themes: {"spiral"}
        blurb: "A paved court of broken rings walked in meditation, with an offering left at its heart"
        palette: LABYRINTH
        layouts: {
            [[
....++++....
.##oooooo##.
.#oooooooo#.
.oo##oo##oo.
.oo#,,,,#oo.
.oo,,*i,,oo.
.oo#,,,,#oo.
.oo##,,##oo.
.#oooooooo#.
.##oooooo##.
]]
            [[
..+++++..
.#ooooo#.
.o#o,o#o.
.oo,i,oo.
.o#,*,#o.
.oo,,,oo.
.o#o,o#o.
.#ooooo#.
]]
            [[
.++++.......
.oooo####...
.oo,,,,,##..
.o##WW#,,#..
.oo,i*#,,#..
.oo#,,,,,#..
.oo####,##..
.ooooooooo..
]]
        }
    }
    {
        name: "spiral_wardens", title: "Guarded Reliquary", kind: "encounter", themes: {"spiral"}
        blurb: "A reliquary still kept by its dead wardens"
        palette: "shrine"
        enemies: {"Giant Bat", "Giant Spider"}
        brutes: {"Skeleton"}
        layouts: {
            [[
..WWWWWWW..
.WW,,i,,WW.
.W,,E*,,,W.
.Ws,,,,,sW.
.W,e,,,e,W.
.WWW,,,WWW.
...WWdWW...
....:+:....
]]
            [[
.WWWWWWWWWWW.
.W,i,,s,,*,W.
.W,,E,,,E,,W.
.WWW,,,,,WWW.
...W,e,e,W...
...WWWdWWW...
.....:+:.....
]]
            [[
....WWW....
..WWW,WWW..
.WW,,i,,WW.
.W,e,E,e,W.
.Ws,,,,,sW.
.WW,,,,,WW.
..WWWdWWW..
..s:::::s..
....:+:....
]]
        }
    }
    {
        name: "spiral_spring", title: "Holy Spring", kind: "encounter", themes: {"spiral"}
        blurb: "A pilgrims' font gone wild, full of frogs and slime, with offerings on the bottom"
        palette: "shrine"
        base: "ground"
        enemies: {"Giant Frog", "Green Slime"}
        brutes: {"Red Slime"}
        legend: {
            'w': {like: '#', tile: "pixulloch.wall"}     -- deep water
            'q': {tile: "underdungeon.floor"}            -- shallows
            'I': {tile: "underdungeon.floor", spawn: "item"}
            'e': {tile: "underdungeon.floor", spawn: "enemy"}
        }
        layouts: {
            [[
...gggg....
..gqqqqg...
.gqwwwqqg..
.gqwwIeqg..
.gqeqwwqg..
.sgqqqqgs..
...g+g.....
]]
            [[
.gggggggg.
.gsqqqqsg.
.gqqwwqqg.
.gqewwIqg.
.gqqwweqg.
.gqeqqqqg.
.gsqqqqsg.
....++....
]]
            [[
..ooooo..
.ooqqqoo.
.oqwwwqo.
.oqwEwqo.
.oqIqeqo.
.ooqqqoo.
..so+os..
]]
        }
    }
    {
        name: "spiral_cairn", title: "Spiral Cairn", kind: "landmark", themes: {"spiral"}
        blurb: "Pilgrims' stones heaped in a loose ring around a weathered idol"
        palette: CAIRN
        layouts: {
            [[
....+++....
..#ooooo#..
.#oo,,,oo#.
.oo,#,#,oo.
.o,,,s,,,o.
.oo,#c#,oo.
.#oo,,,oo#.
..#ooooo#..
]]
            [[
..++.......
.oooo###...
.oo,,,,##..
.#,s,c,,#..
.#,,,#,,o..
.##,,,,oo..
..###ooo...
]]
        }
        legend: {
            'c': {tile: "floor_alt", spawn: "gold"}
        }
    }
    {
        name: "spiral_prayer", title: "Prayer Shrine", kind: "landmark", themes: {"spiral"}
        blurb: "An avenue of saints leading to a marble prayer niche"
        palette: COURT
        layouts: {
            [[
..WWWWW..
..WsWsW..
..W,,,W..
.s,o,o,s.
..,o,o,..
.s,o,o,s.
..,o+o,..
]]
            [[
.WWW.
.WsW.
.,o,.
s,o,s
.,o,.
s,o,s
.,o,.
s,o,s
.,+,.
]]
            [[
..s...s..
.WWW,WWW.
.W,,,,,W.
.Ws,*,sW.
.W,,,,,W.
.WWW,WWW.
..s,+,s..
]]
        }
    }
    {
        name: "spiral_sundial", title: "Sundial Court", kind: "landmark", themes: {"spiral"}
        blurb: "A round marble court with the hours set in the paving about a tall gnomon"
        palette: COURT
        layouts: {
            [[
....,,,....
..,,ooo,,..
.,oo,o,oo,.
.,o,ooo,o,.
,oooosoooo,
.,o,ooo,o,.
.,oo,o,oo,.
..,,ooo,,..
....,+,....
]]
            [[
..WWWWWWW..
.W,ooooo,W.
W,oo,o,oo,W
Woo,ooo,ooW
Wo,oosoo,oW
Woo,ooo,ooW
W,oo,o,oo,W
.W,oo+oo,W.
..WW...WW..
]]
        }
    }
}
