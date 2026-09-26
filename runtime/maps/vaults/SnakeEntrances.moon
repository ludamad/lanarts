----
-- Entrances to the snake pit: a coiled ring of snake stone, an overgrown pit, a serpent-headed gate
-- and a sunken swampy cistern. The players may start inside, among the 'p' squares.
----

-- Snake stone and snake-scale floors, tunnel paving for paths
SNAKE = {floor: "snake.floor", floor_alt: "snake.floor_alt", path: "snake.floor_tunnel", wall: "snake.wall", wall_alt: "lair.wall"}
-- A pit of snake-scale floor edged in yellow lair stone
PIT = {floor: "snake.floor", floor_alt: "lair.floor", path: "snake.floor_alt", wall: "lair.wall", wall_alt: "snake.wall"}
-- Swamp water and vines inside dark mossy brick, snake stone pillars
CISTERN = {floor: "moss.floor_alt", floor_alt: "moss.floor", path: "snake.floor_tunnel", wall: "moss.wall", wall_alt: "snake.wall"}

return {
    {
        name: "snake_coil", title: "Coiled Snake Stones", kind: "entrance", themes: {"snake"}
        blurb: "A ring of yellow-scaled stone coiled like a sleeping serpent; follow it round to the portal at its heart"
        palette: SNAKE
        layouts: {
            [[
WWWWWWWWWWWWW
W:::::::::::W
W:WWWWWWWWW:W
W:W,,,,,,,W:W
W:W,WWWWW,W:W
W:W,W,,,W,W:W
W:W,W,D,W,W:W
W:W,W,,,W,W:W
W:W,W,WWW,W:W
W:W,,,W,,,W:W
W:WWWWW:WWW:W
Wpppp:W:::::W
WWWWWdWWWWWWW
....+++......
]]
            [[
WWWWWWWWW
W::::::pW
W:WWWWW:W
W:W,,,W:W
W:W,D,W:W
W:W,,,W:W
W:W,WWWpW
W:::WpppW
WWWWWdWWW
....+++..
]]
        }
    }
    {
        name: "snake_pit", title: "Overgrown Pit", kind: "entrance", themes: {"snake"}
        blurb: "A sunken pit walled in lair stone, choked with trees and scrub"
        palette: PIT
        layouts: {
            [[
...T.....T....
..TTWWWWWWTT..
.TWW,,,,,,WWT.
.TW,,h,,,h,WT.
..W,,oooo,,W..
..W,,oDoo,,W..
.TW,,oooo,,WT.
.TW,h,,,,h,WT.
.TWW,pppp,WWT.
..TWWWddWWWT..
......++......
]]
            [[
..TWWWWWWT...
.TWW,,,,WWT..
TWW,,oo,,,WT.
TW,,oooo,,WWT
TW,,oDo,,ppd+
TW,h,ooo,ppd+
TWW,,,,,,,WWT
.TWWh,,,hWWT.
..TWWWWWWWT..
]]
        }
    }
    {
        name: "snake_gate", title: "Serpent Gate", kind: "entrance", themes: {"snake"}
        blurb: "A hall of snake stone shaped like a serpent's head, entered through its jaws"
        palette: SNAKE
        legend: {
            'y': {like: '#', tile: "volcanic.wall"}  -- the serpent's red eyes
        }
        layouts: {
            [[
....WWWWWWW....
..WWW,,,,,WWW..
.WW,,,,D,,,,WW.
.y,,,,,,,,,,,y.
.W,s,,,,,,,s,W.
.WW,,,pppp,,WW.
..WW,,,,,,,WW..
...WWW:::WWW...
.....WWdWW.....
.....W+++W.....
]]
            [[
.....WWW.....
....WW,WW....
...WW,D,WW...
..WW,,,,,WW..
.Wy,,,,,,,yW.
.WW,p,,,p,WW.
..WW,p,p,WW..
...WWW:WWW...
.....WdW.....
....W+++W....
]]
        }
    }
    {
        name: "snake_cistern", title: "Sunken Cistern", kind: "entrance", themes: {"snake"}
        blurb: "A flooded cistern of mossy brick and snake-stone pillars, the portal standing in the swamp"
        palette: CISTERN
        layouts: {
            [[
.WWWWWWWWWWWW.
.W,,,,,,,,,,W.
.W,oooooooo,W.
.W,oWooooWo,W.
.W,oooDoooo,W.
.W,oWooooWo,W.
.W,oooooooo,W.
.W,,pp,,pp,,W.
.WWWWWddWWWWW.
......++......
]]
            [[
...WWWWW....
.WWW,,,WWW..
.W,,ooo,,W..
WW,oWoWo,WW.
W,,ooDoo,pd+
WW,oWoWo,pd+
.W,,ooo,,pW.
.WWW,p,WWW..
...WWWWW....
]]
        }
    }
}
