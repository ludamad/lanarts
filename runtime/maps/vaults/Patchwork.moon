----
-- The patchwork overworld's features: small, rounded things tucked into the meadows between the
-- hedgerows. Whitewashed stone cottages with flagged floors, gardens behind hedges and flower
-- beds, skeps, ponds, folds and stiles. Hedges are trees (T) or shrubs (h), rarely walls.
----

-- Whitewashed stone and flagstones, mossy brick for wall_alt
COTTAGE = {
    floor: "stonework.floor", floor_alt: "grass.dirt", path: "grass.dirt"
    wall: "stonework.wall", wall_alt: "moss.wall", rock: "quarry.wall_alt"
}
-- The hedge-witch's: vine-grown flags, mossy walls
WITCH = {floor: "moss.floor", floor_alt: "moss.floor_alt", path: "grass.dirt", wall: "moss.wall", wall_alt: "moss.wall"}

-- Characters shared by the patchwork vaults
LEGEND = {
    'F': {tile: "quarry.wall_alt", solid: true, seethrough: true}   -- dry-stone wall
    'B': {tile: "hive.wall", solid: true, seethrough: true}         -- bee skep
    'w': {tile: "pixulloch.wall", solid: true, seethrough: true}    -- pond water
    'x': {tile: "stonework.floor_alt"}                              -- planks
    "'": {tile: "ground_dry"}
    'I': {tile: "ground_alt", spawn: "item"}
    'G': {tile: "ground_alt", spawn: "gold"}
}
legend = (extra = {}) -> table.merge LEGEND, extra

return {
    {
        name: "patchwork_cottage_garden", title: "Cottage Garden", kind: "treasure", themes: {"patchwork"}
        blurb: "A walled garden of flower beds; the gardener's savings are buried under the roses"
        palette: COTTAGE
        base: "ground_alt"
        legend: legend!
        layouts: {
            [[
....++....
.FFF::FFF.
.Fhh::hhF.
.F""::""F.
.F"h""h"F.
.Fh"GI"hF.
.F"h""h"F.
.F""""""F.
.FFFFFFFF.
]]
            [[
.++.........
.::FFFFFFF..
.:::"h"h"F..
.Fh:::"""F..
.F"h"::h"FF.
.F""h":::"F.
.Fh"""h":IF.
.FFFh"""GhF.
...FFFFFFFF.
]]
            [[
...++...
..h::h..
.h":"h..
h""::""h
h"h"h""h
h"":I:"h
.h"h"hh.
..hhhh..
]]
        }
    }
    {
        name: "patchwork_apiary", title: "Apiary", kind: "treasure", themes: {"patchwork"}
        blurb: "Straw bee skeps in rows along a hedge, honey and wax left in the grass"
        palette: COTTAGE
        base: "ground_alt"
        legend: legend!
        layouts: {
            [[
..++......
."""""""".
.B"B"B"B".
.""""""""".
.B"B"B"B".
."G""""I".
.B"B"B"B".
.TTTTTTTT.
]]
            [[
....++....
..TT""TT..
.TB""""BT.
T"""BB"""T
TB""GI""BT
T"""BB"""T
.TB""""BT.
..TTTTTT..
]]
            [[
.++........
.::xxxx....
.:"xWWx"B..
.B"xIWx"B..
.""xxxx"B..
.B""""""G..
.BB"B"B"B..
]]
        }
    }
    {
        name: "patchwork_hedge_maze", title: "Hedge Maze", kind: "treasure", themes: {"patchwork"}
        blurb: "A folly of clipped hedges around a statue, with a prize at its heart"
        palette: COTTAGE
        base: "ground_alt"
        legend: legend!
        layouts: {
            [[
.....+.......
TTTTT"TTTTTTT
T"""""T"""""T
T"TTT"T"TTT"T
T"T"""""T"""T
T"T"TTTTT"T"T
T"""T"s"T"T"T
TTT"T"I"T"T"T
T"""T"""""T"T
T"TTTTTTTTT"T
T"""""""""""T
TTTTTTTTTTTTT
]]
            [[
....+....
TTTT"TTTT
T"""""""T
T"TTTTT"T
T"T"""T"T
T"T"s"T"T
T"T"I"""T
T"TTTTTTT
T"""""""T
TTTTTTTTT
]]
            [[
......+......
..TTTT"TTTT..
.TT"""""""TT.
TT""TTTTT""TT
T""TT"""TT""T
T"TT""s""TT"T
T"T""GIG""T"T
T"TT"""""TT"T
T""TTT"TTT""T
TT"""""""""TT
.TTTTTTTTTTT.
]]
        }
    }
    {
        name: "patchwork_tinker", title: "Tinker's Cart", kind: "store", themes: {"patchwork"}
        blurb: "A travelling tinker's cart pulled into a gateway, pots and an anvil set out"
        palette: COTTAGE
        base: "path"
        legend: legend!
        layouts: {
            [[
..++++..
.::::::.
.:xxxa:.
.:x$x::.
.:xxx:h.
.:::::..
]]
            [[
.++......
.::::....
h:xxx:...
.:x$xa:..
.:xxx::..
.h::::h..
]]
            [[
...+++...
.h:::::h.
h::xxx::h
T:ax$x:hT
TT:xxx:TT
.TT"""TT.
]]
        }
    }
    {
        name: "patchwork_hedge_witch", title: "Hedge-witch's Cottage", kind: "store", themes: {"patchwork"}
        blurb: "A mossy cottage in a herb garden, where the hedge-witch sells charms and remedies"
        palette: WITCH
        legend: legend {
            'y': {tile: "grass.floor_alt1", decor: "shrub"}
        }
        layouts: {
            [[
....++....
.hy"::"yh.
.h""::""h.
.hWWWdWWWh
.hWooooWW.
.hWo$o,kW.
.hWWo,oWW.
..hWWWWW..
]]
            [[
..++.......
.h::hhhhh..
.h"::""yh..
hWWWdWWWh..
hWooo,oWWh.
hWk,$ooo,W.
hWWoo,WWWh.
.hWWWWWhh..
]]
            [[
.....++....
..hhh::hhh.
.h"y"::"yh.
.h""":::"h.
.WWWWWdWWWW
.WooooooktW
.Woo$,,oooW
.WWWWWWWWWW
]]
        }
    }
    {
        name: "patchwork_sheepfold", title: "Sheepfold", kind: "encounter", themes: {"patchwork"}
        blurb: "A round dry-stone fold of sheep, and a hound that has got in among them"
        palette: COTTAGE
        base: "ground_dry"
        enemies: {"Sheep", "Sheep", "Sheep", "Hound"}
        legend: legend!
        layouts: {
            [[
...+++...
..FF'FF..
.F'e''eF.
F'''''''F
F'e'*'e'F
F'''''''F
.F''i''F.
..FFFFF..
]]
            [[
.++.........
.''FFFFFFFF.
.F''''''e'F.
.F'e''''''F.
.FFF''F'e'F.
.F*''eF'''F.
.Fi'''F'''F.
.FFFFFFFFFF.
]]
            [[
....++....
..FF''FF..
.F''''''F.
F''e''e''F
F''FFFF''F
F'e'*i''eF
F''FFFF''F
.F''''''F.
..FFFFFF..
]]
        }
    }
    {
        name: "patchwork_brambles", title: "Bramble Thicket", kind: "encounter", themes: {"patchwork"}
        blurb: "A tangle of briars with adders and spiders nesting over a lost pedlar's pack"
        palette: COTTAGE
        base: "ground_alt"
        enemies: {"Adder", "Giant Spider"}
        legend: legend!
        layouts: {
            [[
..++......
.h""hh.h..
h"e""hhhh.
h""h"""eh.
.hh"hh""h.
h""e"hG"h.
h"hh""I"h.
.hh.hhhh..
]]
            [[
.++.........
.""hh.hhh...
h""e"hh""h..
.hh""""e"hh.
..h"hh"h""h.
.hh"h"e"h"h.
h"""hh""IGh.
.hhhhh.hhh..
]]
            [[
...++...
..h""h..
.h"e""h.
h""hh""h
h"eh*""h
h"hIh"eh
.h""""h.
..hhhh..
]]
        }
    }
    {
        name: "patchwork_duck_pond", title: "Duck Pond", kind: "landmark", themes: {"patchwork"}
        blurb: "A round pond fringed with reeds, a plank jetty out over the water"
        palette: COTTAGE
        base: "ground_alt"
        legend: legend!
        layouts: {
            [[
...++++...
.""h""""".
""wwxww"h"
"wwwxwwww"
h"wwxxwww"
""wwwwww""
.h"wwww"h.
..""hh""..
]]
            [[
.++........
.""""""""h"
."wwwww"h""
h"wwwwwww""
"wwwwwwwww"
"wwww"wwwh"
.hwww"xww""
..hwwwxh"""
...h"""h...
]]
            [[
...++....
.""""""h.
h"wwww"""
"wwwwwwh"
"wwhwww""
"wwwwww""
.h"ww""h"
.."""""..
]]
        }
    }
    {
        name: "patchwork_stile", title: "Stile", kind: "landmark", themes: {"patchwork"}
        blurb: "A footpath climbs a stone stile over a thick hedgerow"
        palette: COTTAGE
        legend: legend {
            'S': {tile: "quarry.floor"}
        }
        layouts: {
            [[
...++...
...::...
...::...
.TTTSTT.
TTTWSWTT
.TTTSTT.
...::...
...::...
]]
            [[
.++......
.::......
..::.....
.TTSTTT..
TTWSWTTT.
.TTSTT...
...::....
....::...
]]
            [[
..++.....
..::.....
..:::....
hhhhSh...
hhhWSWhhh
...hShhhh
....::...
....:::..
]]
        }
    }
    {
        name: "patchwork_shepherds_hut", title: "Shepherd's Hut", kind: "landmark", themes: {"patchwork"}
        blurb: "A little wheeled hut on the hillside, crook and blanket still inside"
        palette: COTTAGE
        legend: legend!
        layouts: {
            [[
..++...
.:::...
.:xx##.
.#xxI#.
.#xxx#.
.#####.
]]
            [[
....++..
..'':''.
.##xd##.
.#xxxI#.
.##x###.
..'''h..
]]
            [[
.++.......
.::'''''..
.'x###''h.
.'dxxx#''.
.'#xIx#'h.
.'#####''.
]]
        }
    }
    {
        name: "patchwork_maypole", title: "Maypole Green", kind: "landmark", themes: {"patchwork"}
        blurb: "A ring of flowers around an old carved pole where the villages dance"
        palette: COTTAGE
        base: "ground_alt"
        legend: legend!
        layouts: {
            [[
...++...
..h""h..
.h""""h.
"""hh"""
""""s""h
"""h"h""
.h""""h.
..h""h..
]]
            [[
....++....
..""""""..
.""h""h"".
."h"""""h.
."""""s"".
.""h"""h".
..""h"hh..
....""....
]]
        }
    }
}
