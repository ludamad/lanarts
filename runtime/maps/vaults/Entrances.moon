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
}
