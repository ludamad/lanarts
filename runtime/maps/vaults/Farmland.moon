----
-- The farmland overworld's features: brick barns stacked with hay, stone granaries, farmsteads
-- and market stalls along the lanes, chicken runs and pig pens, wells, windmills and scarecrows.
-- Fields are drawn square, fenced with dry-stone walls you can see over.
----

-- Brick and plank farm buildings, hay bales for wall_alt, dry-stone walls for rock
FARM = {
    floor: "stonework.floor_alt", floor_alt: "grass.dirt", path: "grass.dirt"
    wall: "brick.wall", wall_alt: "lair.wall", rock: "quarry.wall_alt"
}
-- Grey stone and cobbles for granaries, wells and the mill
STONE = table.merge FARM, {floor: "pebble.floor", wall: "stonework.wall", wall_alt: "quarry.wall_alt"}

-- Characters shared by the farm vaults
LEGEND = {
    'F': {tile: "quarry.wall_alt", solid: true, seethrough: true}   -- dry-stone wall / fence
    "'": {tile: "ground_dry"}                                       -- stubble, straw
    'f': {tile: "orc.floor"}                                        -- ploughed soil
    'm': {tile: "moss.floor_alt"}                                   -- mud
    'w': {tile: "pixulloch.wall", solid: true, seethrough: true}    -- water
    'x': {tile: "stonework.floor_alt"}                              -- planks
    'c': {tile: "pebble.floor"}                                     -- cobbles
}
legend = (extra = {}) -> table.merge LEGEND, extra

return {
    {
        name: "farmland_barn", title: "Old Barn", kind: "treasure", themes: {"farmland"}
        blurb: "A brick barn stacked with hay bales, something left in the loft"
        palette: FARM
        legend: legend!
        layouts: {
            [[
....++++....
.###dddd###.
.#WWooooWW#.
.#Wo,oo,oW#.
.#oo,oo,oo#.
.#Wo,oo,oW#.
.#WWoiioWW#.
.#WWWo*WWW#.
.##########.
]]
            [[
.++..........
.::..........
##d######....
#oo,oooW#....
#oo,ooWW#....
#WW,oooo####.
#i,,,,,,,,d:+
#WWooooWi#.:+
##########...
]]
            [[
....++++....
..''::::''..
.WW'::::'WW.
.WW'####d##.
'''#Wooooo#.
'''#oo,,*o#.
.WW#Wo,,oW#.
.WW#WWoiWW#.
'''########.
]]
        }
    }
    {
        name: "farmland_granary", title: "Granary", kind: "treasure", themes: {"farmland"}
        blurb: "A stone grain store; the farmer's takings are hidden among the sacks"
        palette: STONE
        legend: legend!
        layouts: {
            [[
....+++....
....:::....
...##d##...
..##ooo##..
.##oo*oo##.
.#oo*i*oo#.
.##oo*oo##.
..##ooo##..
...#####...
]]
            [[
.+++.........
.:::.........
##d##...###..
#ooo##.##o##.
#o*oo###oio#.
#ooooddoooo#.
##ooo###o*##.
.#####.#####.
]]
            [[
...++...
...::...
.FxxxxF.
xFxxxxFx
xxx*ixxx
xFxxxxFx
.FxxxxF.
]]
        }
    }
    {
        name: "farmland_farmstead", title: "Farmstead", kind: "store", themes: {"farmland"}
        blurb: "A farmhouse behind a walled yard; the farmer sells what the land gives"
        palette: FARM
        legend: legend!
        layouts: {
            [[
.....++.....
.FFFF::FFFF.
.F''''::''F.
.Fh'''':''F.
.F'''''::'F.
.F###d###WF.
.F#ooo,oo#F.
.F#o$oooo#F.
.F#Woo,,a#F.
.F########F.
.FFFFFFFFFF.
]]
            [[
..............++
.FFFFFFFFFFFF.::
.F''''''''''F.::
.F'WW''''''''::.
.F'WW''''''h'F..
.F######d####F..
.F#ooooooooo#F..
.F#o$oo#oooi#F..
.F######d####F..
.FhhhhhhhhhhhF..
.FFFFFFFFFFFFF..
]]
            [[
++......
::FFFF..
:::''F..
#:#d##..
#o,oo###
#oooo,o#
#oo$oo,#
##oooa##
.######.
]]
        }
    }
    {
        name: "farmland_market", title: "Market Stall", kind: "store", themes: {"farmland"}
        blurb: "A trader's stall where two lanes meet, hay and hand-carts about"
        palette: FARM
        base: "path"
        legend: legend!
        layouts: {
            [[
..++++..
.::::::.
:cccccc:
:c#$#Wc:
:cccccc:
:chcchc:
.::::::.
]]
            [[
...++...
...::...
...::...
WW:::cc#
::::c$c#
::::ccc#
WW::::a.
...::...
]]
            [[
.++++++++.
.::::::::.
.cccccccc.
.cW#$##Wc.
.cccccccc.
.ch'ah'hc.
.cccccccc.
]]
        }
    }
    {
        name: "farmland_coop", title: "Chicken Coop", kind: "encounter", themes: {"farmland"}
        blurb: "An angry run of hens, and a rooster the size of a pony guarding the eggs"
        palette: FARM
        enemies: {"Chicken"}
        brutes: {"Super Chicken"}
        legend: legend {
            'e': {tile: "ground_dry", spawn: "enemy"}
        }
        layouts: {
            [[
...++....
.FF::FFF.
.F''e''F.
.F'e''eF.
.F'''''F.
.F##d##F.
.F#oii#F.
.F#E*o#F.
.F#####F.
.FFFFFFF.
]]
            [[
.++.........
.::FFFFFFFF.
.:''e''''eF.
.F''''''''F.
.F'e''FFdFF.
.F''''F#o*#.
.FFFFFF#oE#.
.......#ii#.
.......####.
]]
            [[
...+++...
.FF:::FF.
F''':'''F
F'e'''e'F
F''###''F
F''#E#''F
F''d*d''F
F'e###''F
F'''''''F
.FFFFFFF.
]]
        }
    }
    {
        name: "farmland_pigpen", title: "Pig Pen", kind: "encounter", themes: {"farmland"}
        blurb: "The pigs are long gone; the rats that ate their swill are not"
        palette: FARM
        base: "moss.floor_alt"
        enemies: {"Giant Rat"}
        legend: legend!
        layouts: {
            [[
...++.....
.FF::FFFF.
.Fmmmmmmm#.
.Fmemmmem#.
.FmmFFmmm#.
.FmmmmmFF#.
.FmemmmmW#.
.FFFFFFd##.
.....#i*#..
.....####..
]]
            [[
.++.........
.::FFFFFFFF.
.:mmmmFmmmF.
.FmemmdmemF.
.FmmmmFmmmF.
.FFdFFFFFdF.
.Fmmmm*mmmF.
.F###dd###F.
.F#iommmoe#.
.F########F.
]]
        }
    }
    {
        name: "farmland_well", title: "Village Well", kind: "landmark", themes: {"farmland"}
        blurb: "A stone well where the lanes cross"
        palette: STONE
        legend: legend!
        layouts: {
            [[
...++...
...::...
...::...
..FFFF..
::FwwF::
::FwwF::
..FFFF..
...::...
...::...
]]
            [[
..++++..
.cccccc.
cchccchc
ccFFFFcc
ccFwwFcc
ccFwwFcc
ccFFFF*c
chcccchc
.cccccc.
]]
            [[
.++.......
.::.......
.:::::::..
.:''''':..
.:'Fw'':..
.:'wwF':..
.:''''':..
.:::::::..
]]
        }
    }
    {
        name: "farmland_windmill", title: "Windmill", kind: "landmark", themes: {"farmland"}
        blurb: "The stump of a stone windmill, its lattice sails fallen in the grass"
        palette: STONE
        legend: legend!
        layouts: {
            [[
.......++.....
.......:::....
:::::..##d##..
:":":..#ooo#..
:::::..#o*o#..
:":":..#ooo#..
:::::..#####..
]]
            [[
.......++....
...###d::....
..##ooo#.....
.##oooo##....
.#oo*ooo#....
.##oooo##....
..##oo##.....
...####......
..:::::::....
..:":":":....
..:::::::....
]]
            [[
..+++......
..:::......
.##d##.....
##ooo##:::.
#ooooo#:":.
#ooooo#:::.
##ooo##:":.
.#####.:::.
..::::::":.
..:":":"::.
..::::::::.
]]
        }
    }
    {
        name: "farmland_scarecrow", title: "Scarecrow Field", kind: "landmark", themes: {"farmland"}
        blurb: "Rows of ploughed soil and green shoots, watched over by a scarecrow"
        palette: FARM
        base: "ground_dry"
        legend: legend!
        layouts: {
            [[
++..........
::ffffffffff
::""""""""""
::ffffffffff
::"""""s""""
::ffffffffff
::""""""""""
::ffffffffff
]]
            [[
.....++.....
fff""::""fff
fff""::""fff
fff""::""fff
"""""::"""""
:::::s::::::
"""""::"""""
fff""::""fff
fff""::""fff
fff""::""fff
]]
            [[
...++...
.::::::.
.:ffff:.
.:f""f:.
.:f"s"f:
.:f""f:.
.:ffff:.
.::::::.
]]
        }
    }
    {
        name: "farmland_orchard", title: "Walled Orchard", kind: "treasure", themes: {"farmland"}
        blurb: "Fruit trees in rows behind a dry-stone wall, windfall and a picker's purse in the grass"
        palette: FARM
        base: "ground_alt"
        legend: legend!
        layouts: {
            [[
.....++.....
.FFFF""FFFF.
.F""""""""F.
.F"T""T""TF.
.F""""""""F.
.F"T"*T""TF.
.F""""""""F.
.F"T""T"iTF.
.F""""""""F.
.FFFFFFFFFF.
]]
            [[
.++............
.""FFFFFFFFFFF.
.""""T""T""T"F.
.F"""""""""""F.
.F"T""T"*T""TF.
.F"""""""""""F.
.F"T""T""T"iTF.
.FFFFFFFFFFFFF.
]]
            [[
...+++...
.FF"""FF.
F"""T"""F
F"T"""T"F
F""""*""F
T"""T"""T
F"T"i"T"F
F"""""""F
.FFFFFFF.
]]
        }
    }
    {
        name: "farmland_stable", title: "Horse Stable", kind: "encounter", themes: {"farmland"}
        blurb: "Stalls of spooked horses kicking at anyone who comes near the tack chest"
        palette: FARM
        enemies: {"Horse"}
        legend: legend!
        layouts: {
            [[
....++....
.###dd###.
.#eW''We#.
.#'W''W'#.
.#oooooo#.
.#eW''Wi#.
.#'W''W*#.
.########.
]]
            [[
..++.........
..dd#########.
..''''''''''#.
.#W'W'W'W'W'#.
.#e'e''e'i*'#.
.############.
]]
        }
    }
}
