----
-- The crossroads overworld's features: a village along four roads. Brick and dressed stone
-- buildings (a coaching inn, a smithy, a toll gate, a moneylender's house), a market, a well,
-- milestones by the road, a watchtower, and trouble on the roads: gallows hill, an ambush, a
-- stable of loose horses.
----

-- Brown brick houses on grey flagstones
INN = {floor: "stonework.floor", floor_alt: "brick.floor", wall: "brick.wall", wall_alt: "stonework.wall"}
-- Sandy cobbles with pale stone stalls
MARKET = {floor: "brick.floor_alt", floor_alt: "stonework.floor", wall: "stonework.wall", wall_alt: "brick.wall"}
-- Dark stone, a glowing hearth
SMITHY = {floor: "brick.floor", floor_alt: "grass.dirt", wall: "brick.wall_alt", wall_alt: "brick.wall"}
-- Bare hill: dry grass, grey dirt, weathered posts
GALLOWS = {floor: "blood.floor_alt", floor_alt: "grass.dirt", ground: "grass.floor_alt2", wall: "brick.wall_alt", wall_alt: "crypt.wall_alt"}
-- Straw and dirt in brick stalls
STABLE = {floor: "grass.dirt", floor_alt: "grass.floor_alt2", wall: "brick.wall", wall_alt: "stonework.wall_alt"}

return {
    {
        name: "crossroads_inn", title: "Coaching Inn", kind: "store", themes: {"crossroads"}
        blurb: "A brick coaching inn by the road; the landlord sells to travellers"
        palette: INN
        layouts: {
            [[
.....++.....
.####dd####.
.#oooooooo#.
.#oWoooWoo#.
.#oooooooo#.
.#oWoooWoo#.
.#oooooooo#.
.#WWWWoWWW#.
.#,,$,,,,,#.
.##########.
]]
            [[
.###########.....
.#ooooo#,,,#.....
.#oWoWo#,$,#.....
.#ooooodo,,#.....
.#oWoWo#####.....
.#ooooo#h:::h....
.##d####::::::++.
...:::::::::::++.
]]
            [[
...######....
..##oooo##...
.##oWooWo##..
.#oooo$ooo#..
.##oWooWo##..
..###dd###...
...h:oo:h....
....:::::....
....:++:.....
]]
        }
    }
    {
        name: "crossroads_market", title: "Market Square", kind: "store", themes: {"crossroads"}
        blurb: "A cobbled square of stalls where the roads meet"
        palette: MARKET
        base: "floor"
        legend: {
            'S': {like: 's', tile: "floor_alt"}
        }
        layouts: {
            [[
...++++......
..:oooo:.....
.oooooooooo..
.o#,#oo#,#o..
.o,,,oo,,,o..
.ooooSo$ooo..
.o,,,oo,,,o..
.o#,#oo#,#o..
.oooooooooo..
..h.h..h.h...
]]
            [[
..h.h.h.h.h.h.
..#,#,#,#,#,#.
++:::::::::::.
++:::::::::::.
..#,#,#$#,#,#.
..h.h.h.h.h.h.
]]
            [[
....+++....
..hoooooh..
.oo,W$W,oo.
.o,,,,,,,o.
.oW,,S,,Wo.
.o,,,,,,,o.
.oo,W,W,oo.
..hoooooh..
]]
        }
    }
    {
        name: "crossroads_smithy", title: "Roadside Smithy", kind: "treasure", themes: {"crossroads"}
        blurb: "A smith's forge open to the road, with work left on the anvils"
        palette: SMITHY
        legend: {
            'f': {like: '#', tile: "volcanic.wall"}     -- the forge hearth
        }
        layouts: {
            [[
.#######...
.#ffoooWW..
.#ooaooi#..
.#oooooo:++
.#*oaooi:++
.#ffoooWW..
.#######...
]]
            [[
.######.....
.#ffff#.....
.#oaoo#.....
.#oooid:::++
.#oaoo#:::++
.#i**o#.....
.######.....
]]
            [[
..#####......
..#fff#......
.##ooo##.....
.#oaoao#hh...
.#ooioo,:::++
.#*oooo,:::++
.#######hh...
]]
        }
    }
    {
        name: "crossroads_tollgate", title: "Toll Gate", kind: "treasure", themes: {"crossroads"}
        blurb: "A gate across the road, with the toll collected in the booths beside it"
        palette: "stonework"
        base: "floor"
        layouts: {
            [[
....++....
.WWW::WWW.
.W*d::d*W.
.WiW::WWW.
.WWWddW...
....::....
....::....
]]
            [[
.......++.....
..WWWW.::.....
..W*,W.::.....
..W,i,d::.....
..WWWW.::.....
hhhhhhhddhhhhh
.......::.....
.......::.....
]]
            [[
..++++....
..::::....
WW#dd#WWWW
.W,:,,,*,W
.W,:,,,,iW
.WW:WWWWWW
...:......
]]
        }
    }
    {
        name: "crossroads_moneylender", title: "Moneylender's House", kind: "treasure", themes: {"crossroads"}
        blurb: "A tidy brick house with a strongroom at the back"
        palette: INN
        layouts: {
            [[
.#########.
.#**#oooo#.
.#*id,,,o#.
.####oooo#.
....##dd##.
......++...
]]
            [[
..++......
.#dd####..
.#oooo##..
.#o,,oW*#.
.#oooodi#.
.#o,,oW*#.
.#oooo###.
.#######..
]]
        }
    }
    {
        name: "crossroads_milestone", title: "Milestone Shrine", kind: "landmark", themes: {"crossroads"}
        blurb: "A carved milestone at a fork in the road, with coins left for luck"
        palette: "village"
        base: "path"
        layouts: {
            [[
..h:h..
..g:g..
hg:::gh
:::s:::
hg:*:gh
..g:g..
..h+h..
]]
            [[
.WWWWW.
.WsgsW.
.Wg*gW.
.hg:gh.
..:+:..
]]
            [[
....h....
..ggsgg..
.g::::*g.
+::::::::
.g::::g..
..g.h.g..
]]
        }
    }
    {
        name: "crossroads_well", title: "Village Well", kind: "landmark", themes: {"crossroads"}
        blurb: "A stone well on a cobbled patch, where the roads' travellers water their horses"
        palette: "village"
        legend: {
            'w': {like: '#', tile: "pixulloch.wall"}     -- the water
            'q': {tile: "underdungeon.floor"}            -- wet stones
        }
        layouts: {
            [[
...ooo...
..oWWWo..
.ooWwWoo.
..oWWWo..
.h,ooo,h.
..,o+o,..
]]
            [[
.h.......h.
..,,ooo,,..
.,oWWWWWo,.
.,oWwwwWo*.
.,oWWWWWo,.
..,oqqqo,..
...,,+,,...
]]
            [[
...T.T...
..oo,oo..
.To,W,oT.
.o,WwW,o.
.To,W,oT.
..oo,oo..
....++...
]]
        }
    }
    {
        name: "crossroads_watchtower", title: "Watchtower", kind: "landmark", themes: {"crossroads"}
        blurb: "A round stone watchtower over the road"
        palette: "stonework"
        layouts: {
            [[
..WWWW..
.WWooWW.
.WoooiW.
.WooooW.
.WWodWW.
..W::W..
...++...
]]
            [[
..W.WW...
.Wo,oW...
W,ooo,W..
.o,*o,,++
Wo,,oW.++
.WW.W....
]]
            [[
...WWW.....
..WW,WW....
..W,,,W....
..WW,WW....
.hhWdWhh...
.h:::::h...
.h:ooo:::++
.h:::::h...
.hhhhhhh...
]]
        }
    }
    {
        name: "crossroads_gallows", title: "Gallows Hill", kind: "encounter", themes: {"crossroads"}
        blurb: "A bare hill with a gibbet and shallow graves; the dead don't rest here"
        palette: GALLOWS
        base: "ground"
        enemies: {"Giant Bat", "Giant Rat"}
        brutes: {"Skeleton"}
        legend: {
            'P': {like: 'W', tile: "wall"}           -- gallows posts
            'O': {tile: "floor"}                     -- the scaffold
            'I': {tile: "floor", spawn: "item"}
            'B': {tile: "floor", spawn: "brute"}
        }
        layouts: {
            [[
.....+++.....
...gg:::gg...
..gtg:::gtg..
.gkgOOOOOkgg.
.gtgOPBPOgtg.
.ggkOO*OOkgg.
..gtgOIOgtg..
...ggegegg...
.....ggg.....
]]
            [[
..t.t.t..
.ggggggg.
+g:kPk:g.
+:::B:::.
+g:kPk:g.
.gegIgeg.
..t.t.t..
]]
            [[
.ttt.......
.gggg:::++.
.tkg::gk++.
.ggOOOOgg..
.gePBPOeg..
.ggOOIOgk..
.ttgkgtgg..
..........
]]
        }
    }
    {
        name: "crossroads_ambush", title: "Ambush on the Road", kind: "encounter", themes: {"crossroads"}
        blurb: "An overturned cart with its cargo spilled, and hungry dogs in the bushes"
        palette: "village"
        base: "path"
        enemies: {"Hound", "Hound", "Giant Rat"}
        legend: {
            'c': {like: '#', tile: "orc.wall"}     -- the wrecked cart
        }
        layouts: {
            [[
..hhh...hhh..
.hheh...heh..
+:::::::::::+
+::cc*:i::::+
+:::cc::::::+
..heh..hheh..
..hhh...hh...
]]
            [[
..RRRR.RRR..
.RReRRRReRR.
++:::::::::.
++::*cc:i::.
.RRR:RRReRR.
..RRR..RRR..
]]
            [[
...hh......
..heh.hhh..
++::::heh..
++:cc::::h.
..hc*i::eh.
...h:::hh..
....heh....
....hhh....
]]
        }
    }
    {
        name: "crossroads_stable", title: "Loose Stable", kind: "encounter", themes: {"crossroads"}
        blurb: "A coaching stable whose beasts have gone wild, with the ostler's savings in the loft"
        palette: STABLE
        enemies: {"Horse", "Chicken", "Sheep"}
        layouts: {
            [[
.##########.
.#,e#,e#,e#.
.#,,#,,#,,#.
.#oooooooo#.
.#i*oooooo#.
.####dd####.
....:++:....
]]
            [[
.hhhhhhhhh.....
.h,,,,,,,h.....
.h,e,,,e,h####.
.h,,,,,,,,,oi#.
.h,,,e,,,h##*#.
.hhhh,hhhh.###.
.....:.........
....:++........
]]
            [[
..#######.
..#,,#i,#.
..#e,#*,#.
..#,,#,,#.
..##o#o##.
.h,,ooooo.
++,,,e,oh.
++o,oooh..
]]
        }
    }
}
