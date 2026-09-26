----
-- Entrances to the hive, whose doors take the Azurite Key: honeycomb clusters, a waxen mound, a
-- hollow tree dripping with comb and a village of wild bee skeps.
----

-- The hive's acid-green floors, honey-cell paving and beehive walls; 'c' is dripping comb
HONEY = {floor: "pebble.floor_alt", floor_alt: "hive.floor", path: "lair.floor", wall: "hive.wall", wall_alt: "hive.wall"}
-- Honey-cell wax floors inside the mound
WAX = {floor: "pebble.floor_alt", floor_alt: "pebble.floor_alt", path: "lair.floor", wall: "hive.wall", wall_alt: "hive.wall"}
-- Rotten heartwood inside a ring of trees
TREE = {floor: "lair.floor", floor_alt: "pebble.floor_alt", path: "grass.dirt", wall: "hive.wall", wall_alt: "hive.wall"}
-- Wild lawns around skeps; honey floors inside
SKEPS = {floor: "pebble.floor_alt", floor_alt: "hive.floor", path: "grass.dirt", wall: "hive.wall", wall_alt: "hive.combs"}

COMB = {'c': {like: '#', tile: "hive.combs"}}

return {
    {
        name: "hive_combs", title: "Honeycomb Cluster", kind: "entrance", themes: {"hive"}
        blurb: "Hexagonal cells of beehive wall grown together, comb dripping between them"
        palette: HONEY
        legend: COMB
        layouts: {
            [[
..WWWW...WWWW..
.WW,,WW.WW,,WW.
.W,,,,WWW,,D,W.
.W,,p,,c,,,,,W.
.WW,,,WWW,,,WW.
..WWc,,WW,,WW..
...WW,,,,,WW...
...Wpp,,,ppW...
...WW,,,,,WW...
....WWWdWWW....
......+++......
]]
            [[
...WWWWWWW...
.WWc,,,,,cWW.
WW,,,cDc,,,WW
W,,,,c,c,,,,W
Wc,,,,,,,,,cW
W,,cc,,,cc,,W
WW,pp,,,pp,WW
.WW,,,,,,,WW.
..WWWWdWWWW..
.....+++.....
]]
        }
    }
    {
        name: "hive_mound", title: "Waxen Mound", kind: "entrance", themes: {"hive"}
        blurb: "A domed mound of wax and hive wall, the portal cradled in a ring of comb"
        palette: WAX
        legend: COMB
        layouts: {
            [[
....WWWWW....
..WWW,,,WWW..
.WW,,,,,,,WW.
.W,,cc,cc,,W.
WW,,c,D,c,,WW
W,,,c,,,c,,,W
WW,,cc,cc,,WW
.W,,p,,,p,,W.
.WW,,p,p,,WW.
..WWW,,,WWW..
....WWdWW....
.....+++.....
]]
            [[
....WWWWW......
..WWW,,,WWWWW..
.WW,,,,,,,,,WW.
WW,,cc,,,cc,,WW
W,,,cD,,,,c,,,W
WW,,cc,,,cc,,WW
.WWp,,,,,,,pWW.
..WWWp,,,pWWW..
.....WWdWW.....
......+++......
]]
        }
    }
    {
        name: "hive_hollow_tree", title: "Hollow Hive Tree", kind: "entrance", themes: {"hive"}
        blurb: "A great hollow tree, its rotten heart hung with dripping comb"
        palette: TREE
        legend: COMB
        layouts: {
            [[
....TTTTT....
..TTTcccTTT..
.TTc,,,,,cTT.
.Tc,,,D,,,cT.
TTc,,,,,,,cTT
TT,,c,,,c,,TT
.T,p,,,,,p,T.
.TT,p,,,p,TT.
..TTT,,,TTT..
....TTdTT....
.....+++.....
]]
            [[
...TTTTT...
.TTTcccTTT.
.Tc,,D,,cT.
TT,,,,,,,TT
Tc,p,,,p,cT
TT,p,,,p,TT
.TT,,,,,TT.
..TTTdTTT..
.T..+++..T.
]]
        }
    }
    {
        name: "hive_skeps", title: "Wild Skep Village", kind: "entrance", themes: {"hive"}
        blurb: "Bee skeps gone wild in the long grass, the largest grown into a sealed hive"
        palette: SKEPS
        base: "ground"
        legend: table.merge COMB, {
            ',': {tile: "floor"}
            'D': {tile: "floor", spawn: "portal"}
        }
        layouts: {
            [[
...ggggggggg...
..##g"ggg"g##..
..##gg###gg##..
..gg"##c##"gg..
..g"#,,D,,#"g..
..gg#,,,,,#gg..
..g"##,,,##"g..
..gg"##d##"gg..
..g##p:::p##g..
..g##p:::p##g..
..ggg"":""ggg..
......+++......
]]
            [[
..gg###gg....
.g"#####"g...
.g##,D,##g...
.g"#,,,#"gg..
..g##d##"g##.
.#gg:::g"####
####p:p"g"##.
.##"p:p"gg...
..gg":"gg##..
.....+.......
]]
        }
    }
}
