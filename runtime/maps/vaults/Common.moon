----
-- Features any overworld can draw on, mixed in with each theme's own: wayside things found in
-- grass and forest anywhere. Kept to grass, dirt, timber and plain stone so they suit every theme.
----

WATER = {tile: "pixulloch.wall", solid: true, seethrough: true}
-- Timber (brown brick) walls on dirt and pebble floors
TIMBER = {floor: "pebble.floor", floor_alt: "grass.dirt", path: "grass.dirt", wall: "brick.wall", wall_alt: "brick.wall_alt"}

return {
    {
        name: "common_wagon", title: "Merchant's Wagon", kind: "store", themes: {"any"}
        blurb: "A travelling merchant's wagon pulled off the track, wares laid out beside it"
        palette: TIMBER
        base: "path"
        legend: {
            'f': {tile: "volcanic.floor_alt"}   -- the cooking fire
            'W': {like: 'W', tile: "brick.wall"}   -- the wagon
        }
        layouts: {
            [[
.++++++.
.::::::.
.:WWW$:.
.:WWW::.
.::::f:.
..gg::g.
]]
            [[
..++.....
.g::g....
.:::::g..
.:WW:ff..
.:WW$:...
.:WW::g..
.g:::....
]]
            [[
......++..
..gg::::g.
.g:WWWW:..
.h:WWWW$g.
..g::f::h.
...gggg...
]]
        }
    }
    {
        name: "common_hermit", title: "Hermit's Hut", kind: "store", themes: {"any"}
        blurb: "A hermit's hut deep in the trees; he'll part with a few things for gold"
        palette: "druid"
        legend: {
            '#': {like: '#', tile: "brick.wall"}
        }
        layouts: {
            [[
...T++T...
.T"":"""T.
T""#d##"T.
T"h#,$#"TT
T""#,,#""T
TT"####"TT
.T""""""T.
..TTTTTT..
]]
            [[
.++.......
.:"TTTT...
T:""""TT..
T"##d##T..
T"#,,,#T..
T"h#$,#T..
TTT####T..
.TTTTTTT..
]]
            [[
....++....
.TTT::TTT.
TT##dd##TT
TT#,,,,#TT
Th#$,,,#hT
TT######TT
.TTTTTTTT.
]]
        }
    }
    {
        name: "common_well", title: "Old Well", kind: "landmark", themes: {"any"}
        blurb: "A stone well by a track, a coin or two glinting at the bottom"
        palette: "stone"
        legend: {
            'w': WATER
            ':': {tile: "grass.dirt"}
        }
        layouts: {
            [[
..+++..
.g:::g.
g:###:g
g:#w#:g
g:###:g
.g:*:g.
]]
            [[
.++.....
.:::....
..::ggg.
.h:###g.
..:#w#g.
..g###h.
...ggg..
]]
            [[
...++...
..g::g..
.g:##:g.
.:#ww#:.
.:#ww#:.
.g:##:g.
..g:*g..
]]
        }
    }
    {
        name: "common_campsite", title: "Abandoned Campsite", kind: "treasure", themes: {"any"}
        blurb: "A woodland clearing with a cold fire pit and bedrolls someone left in a hurry"
        palette: TIMBER
        legend: {
            'f': {tile: "volcanic.floor_alt"}
            'b': {tile: "brick.floor_alt"}                   -- a bedroll
            'I': {tile: "brick.floor_alt", spawn: "item"}
        }
        layouts: {
            [[
...++....
.TT::TTT.
TTg::gTT.
Tgb,,bgT.
Tg,ff,gTT
TgI,,*gT.
TTgg,ggTT
.TTTTTTT.
]]
            [[
.++.......
.::gTT....
T"g,gTTT..
Tb,ff,bT..
Tg,ff,IT..
TTg,,g*T..
.TTTTTTT..
]]
            [[
....++....
..TT::TT..
.Tgg,,ggT.
TgI,ff,bgT
Tgb,ff,,gT
.Tgg,,ggT.
..TTTTTT..
]]
        }
    }
    {
        name: "common_watchtower", title: "Ruined Watchtower", kind: "treasure", themes: {"any"}
        blurb: "The broken stump of a watchtower; the old guard's chest is still inside"
        palette: "stone"
        legend: {
            'r': {tile: "floor_alt"}   -- fallen masonry
            'R': {like: 'R', tile: "wall"}
        }
        layouts: {
            [[
....++...
...:::...
..##d###.
.##ooo,##
.#o,i,ro#
.#oro,*r#
.##o,,o##
..#R##R#.
]]
            [[
..+++....
..:::....
.##::R#..
##o,oo##.
#o,i,*o#.
#r,oo,,#.
##ro,o##.
.######..
]]
            [[
.........
..##R##..
.##oioo#.
.#ooro#.+
.#r*o,d:+
.##,o,##.
..##R##..
]]
        }
    }
    {
        name: "common_shrine", title: "Wayside Shrine", kind: "landmark", themes: {"any"}
        blurb: "A little marble shrine by the path, with an offering left at its feet"
        palette: "shrine"
        legend: {
            ':': {tile: "grass.dirt"}
        }
        layouts: {
            [[
..+++..
..:::..
.h,:,h.
.#,*,#.
.#,s,#.
.#####.
]]
            [[
.++......
.::::....
..h:,,h..
...,s,...
..h,*,h..
.........
]]
            [[
...++...
.g::::g.
.:W,,W:.
g:,s*,:g
.:W,,W:.
.g::::g.
]]
        }
    }
    {
        name: "common_bandits", title: "Bandit Camp", kind: "encounter", themes: {"any"}
        blurb: "A palisaded bandit camp; their dogs and rats guard the loot"
        palette: TIMBER
        enemies: {"Hound", "Giant Rat"}
        legend: {
            'W': {like: 'W', tile: "orc.wall"}   -- palisade of sharpened stakes
            'f': {tile: "volcanic.floor_alt"}
        }
        layouts: {
            [[
....++....
.WWW::WWW.
.W,,,,,,W.
.W,e,f,eW.
.W,,ff,,W.
.W#d#,e,W.
.W#*#,,,W.
.WWWWWWWW.
]]
            [[
.++.........
.::WWWWWWW..
.W,,,,e,,W..
.W,ff,,,#W..
.We,,,e,dW..
.W,,,,,#iW..
.WWWWWWW*W..
........W...
]]
            [[
.....++.....
..WWW::WWW..
.W,,e,,,,,W.
.W,##,f,e,W.
.W,#i,,,,,W.
.W,d#,,e,,W.
..W,,,,,,W..
...WWWWWW...
]]
        }
    }
    {
        name: "common_barrow", title: "Old Barrow", kind: "encounter", themes: {"any"}
        blurb: "A grassy burial mound with its door broken open; the dead inside are restless"
        palette: "crypt"
        enemies: {"Giant Rat", "Giant Bat"}
        brutes: {"Skeleton", "Zombie"}
        legend: {
            'T': {like: 'T', tile: "grass.wall"}
            'M': {tile: "grass.floor_alt1", solid: true}   -- the mound's turf
        }
        layouts: {
            [[
...++....
..g::g...
.MM#d#MM.
MM##,##MM
M#,,e,,#M
M#,E,*,#M
M#e,,,i#M
MM#####MM
.MMMMMMM.
]]
            [[
.++.........
.:::........
.M#d#MMM....
.M#e,##MM...
.M##,,E##M..
..M##,e,#M..
..MM##,*#M..
....M#i##M..
....MMMMM...
]]
            [[
....++....
....::....
..MM#d#M..
.MM##e##M.
.M#,,,,#M.
.M#E,,i#M.
.M#,*,E#M.
.MM#####M.
..MMMMMM..
]]
        }
    }
    {
        name: "common_grave", title: "Forgotten Grave", kind: "landmark", themes: {"any"}
        blurb: "Weathered headstones under an old tree, far from any village"
        palette: "meadow"
        legend: {
            ':': {tile: "grass.dirt"}
        }
        layouts: {
            [[
..++...
..::...
.gtgt".
."g"gg.
.gTTtg.
.hTTg".
..ggg..
]]
            [[
.++.......
.::gg"g...
..t"t"tg..
..g"g"gg..
..t"t"tTT.
...ggg*TT.
]]
            [[
...++...
..h::h..
.g"gg"g.
.gt"t"g.
.g"TT"g.
.gtTTtg.
..g"*g..
]]
        }
    }
    {
        name: "common_cache", title: "Lost Adventurer's Cache", kind: "treasure", themes: {"any"}
        blurb: "An adventurer's bones under a rock overhang, with their pack beside them"
        palette: "pebble"
        legend: {
            'R': {like: 'R', tile: "grass.wall_alt"}
        }
        layouts: {
            [[
..RRR....
.RRoRR...
RRoi*oR..
.Rook,+..
..R,,++..
]]
            [[
.RRRR.....
RRoikRR...
RoooooR...
.RRR,,++..
....,++...
]]
            [[
...++.
..,,R.
.Roo,R
RRkio.
RR*oRR
.RRRR.
]]
        }
    }
}
