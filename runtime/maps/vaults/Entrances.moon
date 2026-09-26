----
-- Dungeon entrances for any dungeon, drawn in the dungeon's palette: the sealed halls the overworld
-- always had. Walls marked 2 are set into existing rock or forest.
----

return {
    {
        name: "sealed_hall", title: "Sealed Hall", kind: "entrance", themes: {"any"}
        blurb: "A walled hall dug into the hillside, entered through a double door"
        layouts: {
            [[
...WWWWWWWWWWW...
..WWppppppppWW...
..Wpppp*pppppW...
..Wpppp*pppppdoo+
..Wpp*pp**pppdoo+
WWWpppp*ppppWW...
Wpppppp*ppppW....
2pDppppppppWW....
22ppWWWWWWWW.....
.222W............
]]
            [[
...WWWWWW...
..WWpppWW...
..WpppppW...
..WppDppdoo+
..Wpppppdoo+
WWWppppWW...
WppppppW....
2pppppWW....
22ppWWW.....
.222W.......
]]
            [[
...WWWWW....
..WWpDpdoo+.
.WWppppdoo+.
.WpppppWWW..
.WWppWWW....
..WWWW......
]]
        }
    }
    {
        name: "sealed_vault", title: "Sealed Vault", kind: "entrance", themes: {"any"}
        blurb: "A domed vault with a gold hoard beside the portal"
        layouts: {
            [[
.....WWWWWWW....
...222Dppppdooo+
.222pppppppdooo+
.22ppppppp22....
.22***22222.....
.2p*p*222.......
.2222222........
]]
            [[
......W++W..
......WooW..
.....WWddWW.
...WWWppppWW
.WWWppppppWW
.WWpDpppppWW
.WW***WWWWW.
.Wp*p*WWW...
.WWWWWWW....
]]
        }
    }
    {
        name: "sealed_bastion", title: "Sealed Bastion", kind: "entrance", themes: {"any"}
        blurb: "A round bastion with a treasury, entered from a gatehouse on its flank"
        layouts: {
            [[
.......WWWWWWW
.....WWWpppppW
...WWWppppppWW
...WWpDpppppWW
+oodp***p2222.
+oodp*p*pp2...
...WWW22222...
]]
            [[
.......WWWWWWW...
.....WWWpppppWWW.
...WWWpppppppppWW
...WWppppppppppWW
+oodp***ppDp2222.
+oodp***pppp2222.
+oodp*p*ppppp222.
...WWppppppppppWW
...WWWpppppppppWW
.....WWWpppppWWW.
.......WWWWWWW...
]]
            [[
.......WWWWWWW
.....WWWpppppW
...WWWppppDpWW
...WWpppppppWW
+oodp***p2222.
+oodp***p2222.
+oodp*p*pp222.
...WWpppppppWW
...WWWppppppWW
.....WWWpppppW
.......WWWWWWW
]]
        }
    }
    {
        name: "any_stair_shaft", title: "Stair Shaft", kind: "entrance", themes: {"any"}
        blurb: "A small square shaft cut into the rock, a stair going down behind a single door"
        layouts: {
            [[
.22222.
22pDp22
2ppppp2
2p,,,p2
22#d#22
..+o+..
]]
            [[
.2222..
22pD22.
2pppp2.
2p,,pd+
22,,pd+
.22222.
]]
        }
    }
    {
        name: "any_portico", title: "Pillared Portico", kind: "entrance", themes: {"any"}
        blurb: "A columned porch set into the rock, the portal waiting between the pillars"
        layouts: {
            [[
.2222222.
22spDps22
2ppp,ppp2
2sp,,,ps2
22W#d#W22
...+o+...
]]
            [[
..22222..
.22sDs22.
22pp,pp22
2sp,,,ps2
2pp,,,pp2
22W#d#W22
...+o+...
]]
        }
    }
    {
        name: "any_vestibule", title: "Double-Doored Vestibule", kind: "entrance", themes: {"any"}
        blurb: "An inner chamber behind an antechamber, each with its own door"
        layouts: {
            [[
.22222.
22pDp22
2ppppp2
22#d#22
.2,,,2.
.2#d#2.
..+o+..
]]
            [[
.222222..
22ppDp22.
2pppppp2.
22W#d#W2.
.2,,,,,d+
.22222W2.
]]
        }
    }
}
