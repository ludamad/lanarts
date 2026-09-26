----
-- Entrances to the orc outpost, dug into the Underdungeon's rock (palisade gates, a sentry post, a
-- desolate waystation, a toppled watchtower), and to the ogre lair, dug into the Outpost Entrance
-- level and sealed with the Azurite Key (a bone-ringed maw, a crude gate, a feasting hall, a bone
-- pit). Both levels are cramped, so these are compact and set into the rock: their back walls (2)
-- must go into solid rock, drawn in the level's own wall so they blend in.
----

-- Snake-pit floors, orc-built walls, the Underdungeon's slime rock behind
OUTPOST = {
    floor: "snake.floor", floor_alt: "snake.floor_alt", path: "snake.floor_tunnel"
    wall: "orc.wall", wall_alt: "underdungeon.wall", rock: "quarry.wall"
}

-- Orc floors, swamp mud and blood, the Outpost Entrance's dark brick behind
OGRE = {
    floor: "orc.floor", floor_alt: "orc.floor_alt", path: "blood.floor"
    wall: "orc.wall", wall_alt: "pebble.wall", rock: "lair.wall"
}

-- Floor strewn with bones
BONES = {tile: "underdungeon.floor_alt"}

return {
    ---------------------------------------------------------------------------
    -- Outpost
    ---------------------------------------------------------------------------
    {
        name: "outpost_palisade", title: "Orc Palisade", kind: "entrance", themes: {"outpost"}
        blurb: "A stockade gate of piled stone, flanked by crude idols, guarding the way to the outpost"
        palette: OUTPOST
        layouts: {
            [[
.2222222.
22ppDpp22
2pp,,,pp2
2##,,,##2
.#s#d#s#.
..+ooo+..
]]
            [[
..2222222..
.22ppDpp22.
.2pp,,,pp2.
##W#,,,#W##
#s#W#d#W#s#
....+o+....
]]
            [[
.222222222.
22pp,D,pp22
2p,,,,,,,p2
2#,p,,,p,#2
2##W#d#W##2
...s+o+s...
]]
        }
    }
    {
        name: "outpost_sentry", title: "Sentry Post", kind: "entrance", themes: {"outpost"}
        blurb: "A guardroom cut into the rock beside the outpost gate, its weapon rack long since stripped"
        palette: OUTPOST
        layouts: {
            [[
.22222..
22pDp22.
2pppp,2.
2app,,d+
2pppp,d+
222222..
]]
            [[
..2222..
.22pD2..
.2pppp2.
.2,a,p22
.22,,,d+
..22,,d+
...2222.
]]
        }
    }
    {
        name: "outpost_waystation", title: "Desolate Waystation", kind: "entrance", themes: {"outpost"}
        blurb: "An abandoned waystation among graves and bones, its two doors barred from within long ago"
        palette: OUTPOST
        legend: {',': BONES}
        layouts: {
            [[
.2222222.
22tpDpt22
2pp,,,pp2
2p,k,,,p2
2#,,,,,#2
.##d#d##.
..+o.o+..
]]
            [[
..22222..
.22pDp22.
.2pp,pp2.
22,k,t,22
2,,,,,,,2
2##,,,##2
..#d#d#..
..+o.o+..
]]
        }
    }
    {
        name: "outpost_watchtower", title: "Toppled Watchtower", kind: "entrance", themes: {"outpost"}
        blurb: "The rubble-strewn stump of an orc watchtower, its cellar stair still open"
        palette: OUTPOST
        layouts: {
            [[
..2222..
.22pp22.
22pDpp22
2Rpppp,2
2RR,,,R2
.2R#d#2.
...+o+..
]]
            [[
...2222...
..22pp22..
.22pDppR2.
22RpppppR2
2R,,p,,RR2
.2RR#d#R2.
....+o+...
]]
        }
    }

    ---------------------------------------------------------------------------
    -- Ogre lair
    ---------------------------------------------------------------------------
    {
        name: "ogre_maw", title: "Bone-Ringed Maw", kind: "entrance", themes: {"ogre"}
        blurb: "A cave mouth ringed with gnawed bones and skulls, reeking of the ogres within"
        palette: OGRE
        legend: {'b': BONES, 'k': {like: 'k', tile: "underdungeon.floor_alt"}}
        layouts: {
            [[
..22222..
.22pDp22.
22ppppp22
2Rbpppbb2
2RRkbkRR2
.2R#d#R2.
..k+o+k..
]]
            [[
...2222....
..22pD22...
.22pppp222.
22bpppbbp22
2RRbkpkbRR2
.RRR#d#RRR.
....+o+....
]]
        }
    }
    {
        name: "ogre_gate", title: "Crude Ogre Gate", kind: "entrance", themes: {"ogre"}
        blurb: "A lopsided double gate of heaped stone, bigger than any orc would bother to build"
        palette: OGRE
        legend: {'k': {like: 'k', tile: "path"}}
        layouts: {
            [[
.22222222.
22pppDpp22
2pp,,,,pp2
2###dd###2
.s.+oo+.s.
]]
            [[
..222222..
.22pDpp22.
22pppppp22
2k,pp,,k,2
2###dd###2
..s+oo+s..
]]
        }
    }
    {
        name: "ogre_feasting_hall", title: "Feasting Hall", kind: "entrance", themes: {"ogre"}
        blurb: "A long hall of crude stone tables, bones and dropped coins, where the ogres gorge themselves"
        palette: OGRE
        legend: {'b': BONES}
        layouts: {
            [[
.22222222222.
22bpppDpppb22
2b,aaa,aaa,b2
2p,,,,,,,,,p2
22*,b,,,b,*22
.2222#dd#222.
.....+oo+....
]]
            [[
..2222222..
.22pDppb22.
22,aa,aa,22
2*,,,p,,,*2
22bp,,,pb22
.222#d#222.
....+o+....
]]
        }
    }
    {
        name: "ogre_bone_pit", title: "Bone Pit", kind: "entrance", themes: {"ogre"}
        blurb: "A sunken pit where the ogres throw what they don't eat, the lair's door at its bottom"
        palette: OGRE
        legend: {'b': BONES, 'k': {like: 'k', tile: "underdungeon.floor_alt"}}
        layouts: {
            [[
..222222..
.22bbDb22.
22bkpppb22
2bpppppkb2
22bppkpb22
.222#d#22.
....+o+...
]]
            [[
..22222...
.22bDbb2..
22kppppb2.
2bppkppb22
2bbpppkbb2
.222#d#22.
....+o+...
]]
        }
    }
}
