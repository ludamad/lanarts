----
-- The craters overworld's features: an impact field. Star-metal shards in scorched pits,
-- prospectors and a smelter working the fall, glassy slag, a crater lake and a sky cult.
----

-- Scorched ground where something hot came down: infernal floor, red embers, rim rock
SCORCH = {floor: "volcanic.floor", floor_alt: "volcanic.floor_alt", path: "grass.dirt", wall: "quarry.wall", wall_alt: "cobalt.wall"}
-- Rock fused into glass: black cobalt, crystal floor, cobalt and crystal walls
GLASS = {floor: "cobalt.floor", floor_alt: "crystal.floor", path: "grass.dirt", wall: "cobalt.wall", wall_alt: "crystal.wall"}
-- Diggings: sandy rock, pebbles, rough rock walls and dark stone
DIGGINGS = {floor: "quarry.floor", floor_alt: "grass.dirt", path: "grass.dirt", wall: "quarry.wall", wall_alt: "brick.wall_alt"}

SHARD = {tile: "crystal.wall", solid: true, seethrough: true}         -- a star-metal shard
WATER = {tile: "pixulloch.wall", solid: true, seethrough: true}      -- still crater water
EMBER = {tile: "volcanic.floor_alt"}

return {
    {
        name: "craters_starshard", title: "Fallen Shard", kind: "treasure", themes: {"craters"}
        blurb: "A shard of star-metal glinting in the scorched pit it dug"
        palette: SCORCH
        legend: {
            'C': SHARD
            'v': EMBER
            'I': {tile: "floor_alt", spawn: "item"}
        }
        layouts: {
            [[
.....++....
...RR::RR..
..Rvoooovv.
.RvoCC,oovR
.Rvo,CCI,vR
.Rvo,,C,ovR
..Rvo*oovR.
...RRvvRR..
.....RR....
]]
            [[
..++..........
.R::RR........
.Rvo,vRRR.....
..Rvo,,ovRRR..
...Rvo,,,ooRR.
....RRvo,ICCR.
......RRo*CR..
........RRR...
]]
            [[
...+++...
.RRv:vRR.
RvoC,Covv
Rv,,I,,vR
RvoC,CovR
.RRvovRR.
...RRR...
]]
        }
    }
    {
        name: "craters_prospectors", title: "Prospectors' Camp", kind: "treasure", themes: {"craters"}
        blurb: "Tents and a cold fire where prospectors sifted the ejecta for star-metal"
        palette: DIGGINGS
        legend: {
            '#': {like: '#', tile: "brick.wall"}
            'f': EMBER
        }
        layouts: {
            [[
....++++....
.,,,::::,,,.
.,##,,,,##,.
.,#i,ff,*#,.
.,##,ff,##,.
..,,,,,,,,..
...,*,,,....
]]
            [[
.++.........
.::,,,,,,,,.
.,#d#,#d#,,.
.,#i#,#*#ff.
.,###,###ff.
.,,,,,,,,,,.
]]
            [[
..RRRRRR..
.RR,,*,RR.
.R,,ff,,R.
.R##,,,i+.
.R#*,,,,+.
..RR,,,++.
]]
        }
    }
    {
        name: "craters_orecart", title: "Overturned Ore Cart", kind: "treasure", themes: {"craters"}
        blurb: "A cart of star-metal ore tipped over on the haul road, its load spilled"
        palette: DIGGINGS
        legend: {
            'W': {like: 'W', tile: "iron.wall"}
            'c': {tile: "cobalt.floor"}
            'I': {tile: "cobalt.floor", spawn: "item"}
            'G': {tile: "cobalt.floor", spawn: "gold"}
        }
        layouts: {
            [[
.++.......
.::.......
..:::.....
...:WW:...
...:WWcG..
....::cI:.
......:::.
.......++.
]]
            [[
..+++....
..:::....
.c:W:c...
.G:W:cc..
..:W:I...
..:::....
...R.....
]]
            [[
.........
.+::::R..
.+:WWW:c.
..:GcIcc.
...cRc...
]]
        }
    }
    {
        name: "craters_smelter", title: "Star-metal Smelter", kind: "store", themes: {"craters"}
        blurb: "A smith's furnace built from rim stone, forging what falls from the sky"
        palette: {floor: "stonework.floor", floor_alt: "volcanic.floor_alt", path: "grass.dirt", wall: "brick.wall", wall_alt: "volcanic.wall"}
        layouts: {
            [[
.###########.
.#WWW,oooaa#.
.#W,,,oo$oo#.
.#WWW,ooooo#.
.####oo#####.
....#dd#.....
....:++:.....
]]
            [[
..++++..
.o::::o.
.WWo,ao.
.W,,o$o.
.WWoooo.
..oooo..
]]
            [[
..#####....
.##WW,###..
.#W,,,oa##.
.#W,,oo$o#.
.##,oooooo+
..####oo++.
]]
        }
    }
    {
        name: "craters_glasstrader", title: "Glass Trader", kind: "store", themes: {"craters"}
        blurb: "A pedlar of fused crater-glass, trading from a hut of the stuff"
        palette: GLASS
        layouts: {
            [[
...++...
.##dd##.
.#,oo,#.
#W,$o,W#
.#,oo,#.
.##WW##.
]]
            [[
.++......
.::oo,,..
..o,##W#.
..,,d$,W.
..o,#,,#.
...,#W##.
]]
            [[
...++....
..,::,...
.,,oo,,..
.WW,$,WW.
..W,,,W..
...WWW...
]]
        }
    }
    {
        name: "craters_slag", title: "Slag Field", kind: "landmark", themes: {"craters"}
        blurb: "Ground melted to black glass by the impact, with crystal jutting through"
        palette: GLASS
        layouts: {
            [[
...++.....
..,o,.,o..
.,oW,,oo,.
.o,,,W,.o.
..oW,*,o..
...,o,,...
]]
            [[
.++.............
.:,o............
..o,oW,.........
...,o,,o,W......
.....oW,,o,o,...
.......,oo,W,o..
..........,o*o..
]]
            [[
..+++..
.,o,o,.
,oWWWo,
o,WRW,o
,oWWWo,
.,o*o,.
]]
        }
    }
    {
        name: "craters_lakeshrine", title: "Crater Lake Shrine", kind: "landmark", themes: {"craters"}
        blurb: "Rain filled an old crater; a causeway leads to a shrine on its island"
        palette: "shrine"
        legend: {
            'w': WATER
            'g': {tile: "grass.floor_alt2"}
        }
        layouts: {
            [[
....++....
...R::R...
..Rww:wR..
.Rwww:wwR.
.Rww,o,wR.
.Rwwio,wR.
.Rww,s,wR.
..RwwwwR..
...RRRR...
]]
            [[
.++......
g::gwwR..
g,s,wwwR.
g,o,swwR.
g,,,wwwR.
.gwwwwR..
..RRRR...
]]
            [[
...++...
.Rs::sR.
RwwwwwwR
RwwwwwwR
.Rs,*sR.
...gg...
]]
        }
    }
    {
        name: "craters_skywatch", title: "Skywatchers' Circle", kind: "landmark", themes: {"craters"}
        blurb: "A cult's ring of statues, every face turned up to where the stars fell from"
        palette: "shrine"
        legend: {
            ':': {tile: "grass.dirt"}
        }
        layouts: {
            [[
..+++..
.s:::s.
s,o,o,s
.o,*,o.
s,o,o,s
.s,,,s.
]]
            [[
.++..........
.::s.s.s.s...
.:ooooooooo#.
.:o,,,,,,,*#.
.:ooooooooo#.
.::s.s.s.s...
]]
            [[
....++....
....::....
..s,oo,s..
....oo....
.s,oo,,os.
....oo....
..s,oo,s..
....*.....
]]
        }
    }
    {
        name: "craters_elementals", title: "Charged Shard", kind: "encounter", themes: {"craters"}
        blurb: "A crackling shard that draws cloud and storm elementals to circle it"
        palette: SCORCH
        enemies: {"Cloud Elemental"}
        brutes: {"Storm Elemental"}
        legend: {
            'C': SHARD
            'v': EMBER
        }
        layouts: {
            [[
....+++....
..vv:::vv..
.vv,,e,,vv.
.v,e,C,,,v.
.v,,Ci,E,v.
.v,,,C,e,v.
.vv,,,,,vv.
..vvvvvvv..
]]
            [[
..++.......
.v::v......
.v,e,vv....
..v,,C,vv..
...v,Ci,ev.
....v,E,,v.
.....vv,*v.
.......vv..
]]
            [[
....++....
.RRv::vRR.
.Rv,e,,vR.
.v,C,,C,v.
.v,,iE,,v.
.v,C,,C,v.
.Rv,,e,vR.
.RRvvvvRR.
]]
        }
    }
    {
        name: "craters_adit", title: "Collapsed Adit", kind: "encounter", themes: {"craters"}
        blurb: "A mine tunnel into the rim, half fallen in, where vermin nest among the spoil"
        palette: DIGGINGS
        enemies: {"Giant Rat", "Giant Spider", "Giant Bat"}
        legend: {
            'W': {like: 'W', tile: "brick.wall"}   -- pit props
        }
        layouts: {
            [[
..++.........
.#::#........
.#oo##.......
.W,ooW#####..
.#o,oe,ooo##.
.##W#o##e,*#.
....#,,o,i##.
....###e###..
.......#.....
]]
            [[
.++.............
.::#############
.#oW,oeoWo,oe*#.
.#o,,RR,,o,R,i#.
.##oWoe,RWoo,##.
..#############.
]]
            [[
....+++....
...#:::#...
..##ooo##..
..#W,o,W#..
..#Roeo,#..
..##,RR,##.
...#e,,,#..
...#W*iW#..
...######..
]]
        }
    }
}
