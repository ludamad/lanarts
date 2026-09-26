----
-- Entrances to the temple: marble porticos, crystal-spired sanctums, a stepped ziggurat, a sunken
-- court of statues and a walled garden. The players may start inside, among the 'p' squares.
----

-- White marble floors and crystal paving between the temple's marble walls
MARBLE = {floor: "marble.floor", floor_alt: "temple.floor", path: "marble.floor_alt", wall: "temple.wall", wall_alt: "marble.wall_alt"}
-- The temple's own crystal floors, with crystal spires ('C') and cobalt paving
CRYSTAL = {floor: "temple.floor", floor_alt: "temple.floor_alt", path: "cobalt.floor", wall: "temple.wall", wall_alt: "crystal.wall"}
-- Tiers: relief-carved inner walls ('#'), marble outer walls ('W'), a crystal shrine ('C') on top
TIERS = {floor: "temple.floor", floor_alt: "marble.floor", path: "marble.floor_alt", wall: "marble.wall_alt", wall_alt: "temple.wall"}
-- Lawns inside marble walls
GARDEN = {floor: "marble.floor", floor_alt: "temple.floor", path: "marble.floor_alt", wall: "temple.wall", wall_alt: "marble.wall_alt"}

SPIRE = {'C': {like: '#', tile: "crystal.wall"}}

return {
    {
        name: "temple_portico", title: "Marble Portico", kind: "entrance", themes: {"temple"}
        blurb: "A marble hall fronted by a colonnaded porch, statues flanking the portal"
        palette: MARBLE
        layouts: {
            [[
....WWWWWWWWWWW....
....Ws,,,D,,,sW....
....W,,,,,,,,,W....
....W,pp,,,pp,W....
..WWWWWWWdWWWWWWW..
..ooooooo:ooooooo..
..#o#o#o#:#o#o#o#..
..ooooooo:ooooooo..
........+++........
]]
            [[
.WWWWWWWWWW.
.Ws,,D,,,sW.
.W,,,,,,,,W.
.W,pp,,pp,W.
.WWWWddWWWW.
.#o#oooo#o#.
.oooooooooo.
....:++:....
]]
            [[
.WWWWWWW..
.W,,s,,W..
.W,,D,,W..
.Ws,,,sW..
.Wpp,ppW..
.WWWdWWW..
.#o#o#o#..
.ooooooo++
.#o#o#o#..
]]
        }
    }
    {
        name: "temple_sanctum", title: "Crystal Sanctum", kind: "entrance", themes: {"temple"}
        blurb: "A domed sanctum ringed with crystal spires, the portal among them"
        palette: CRYSTAL
        legend: SPIRE
        layouts: {
            [[
.....+++.....
.....:::.....
...WWWdWWW...
..WC,,,,,CW..
.WC,pp,pp,CW.
.W,,,C,C,,,W.
.W,,,,D,,,,W.
.W,,,C,C,,,W.
.WC,,,,,,,CW.
..WC,,,,,CW..
...WWWWWWW...
]]
            [[
....C+C....
....WdW....
..CWW,WWC..
..W,p,p,W..
CWW,,,,,WWC
W,p,,D,,p,W
CWW,,,,,WWC
..W,,,,,W..
..CWW,WWC..
....WCW....
]]
            [[
...++...
.CWddWC.
.W,pp,W.
.W,pp,W.
.WC,,CW.
.W,,D,W.
.CWWWWC.
]]
        }
    }
    {
        name: "temple_ziggurat", title: "Stepped Ziggurat", kind: "entrance", themes: {"temple"}
        blurb: "A broad terrace of crystal pillars around a raised shrine"
        palette: TIERS
        legend: SPIRE
        layouts: {
            [[
......+++......
.WWWWWWdWWWWWW.
.W,,,,,:,,,,,W.
.W,ppp,:,ppp,W.
.W,,C,,:,,C,,W.
.W,,,##d##,,,W.
.W,,,#ooo#,,,W.
.W,C,#oDo#,C,W.
.W,,,#ooo#,,,W.
.W,,,#####,,,W.
.W,,,,,,,,,,,W.
.WWWWWWWWWWWWW.
]]
            [[
....+++....
.WWWWdWWWW.
.Wpp,:,ppW.
.W,C,:,C,W.
.W,##d##,W.
.W,#oDo#,W.
.W,#ooo#,W.
.W,#####,W.
.W,,,,,,,W.
.WWWWWWWWW.
]]
        }
    }
    {
        name: "temple_sunken_court", title: "Sunken Court", kind: "entrance", themes: {"temple"}
        blurb: "A walled court sunk below the ground, lined with statues, the portal in its apse"
        palette: MARBLE
        layouts: {
            [[
......+......
.WWWWWdWWWWW.
.W,,,,:,,,,W.
.W,s,,:,,s,W.
.W,,,,:,,,,W.
.W,s,p:p,s,W.
.W,,,p:p,,,W.
.W,s,,:,,s,W.
.W,,,,:,,,,W.
.WWW#ooo#WWW.
....#oDo#....
....#####....
]]
            [[
..WWWWWWWW..
.WW,s,,s,WW.
.W,,,,,,,,W.
.Ws,,ooo,sW.
.W,,,oDo,,W.
.Ws,,ooo,sW.
.W,,,,,,,,W.
.WW,pppp,WW.
..WWWddWWW..
.....++.....
]]
        }
    }
    {
        name: "temple_garden", title: "Temple Garden", kind: "entrance", themes: {"temple"}
        blurb: "A walled garden of lawns and shrubs around a small marble shrine"
        palette: GARDEN
        base: "ground"
        legend: {
            'D': {tile: "floor", spawn: "portal"}
        }
        layouts: {
            [[
.......+.......
.WWWWWWdWWWWWW.
.WT"h"":""h"TW.
.W""""":"""""W.
.Wh""pp:pp""hW.
.W""":::::"""W.
.WT""WW:WW""TW.
.W"""W,,,W"""W.
.Wh""W,D,W""hW.
.W"""WWWWW"""W.
.WTh"""""""hTW.
.WWWWWWWWWWWWW.
]]
            [[
...WWWWWWW...
.WWT"""""TWW.
.W"""WWW"""W.
.W"h"WDW"h"W.
.W"""",""""W.
.WTp"":""pTW.
.W"p"":""p"W.
.WWT"":""TWW.
...WWWdWWW...
.....+++.....
]]
        }
    }
}
