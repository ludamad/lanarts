----
-- The classic overworld's features: pebble-floored ridges with caches, snake-stone huts, stone
-- circles and a cavern shop.
----

return {
    {
        name: "classic_ridge", title: "Rocky Ridge", kind: "landmark", themes: {"classic"}
        blurb: "A walled ledge in the rocks, with a few things left lying about"
        palette: "pebble"
        layouts: {
            [[
....+++.......
...WdddW......
..WWoooWWW....
.WWooooooWWWW.
.WooioioioooW.
.WoooiooooooW.
.WooooooooooW.
.WWW22222WWWW.
..............
]]
            [[
..........++++..
.......ooooooo..
...222oooooooo..
.22ooooioooo2...
.22ooioooo222...
.22ioooo222.....
.2oooo222.......
.2222222........
]]
            [[
.........++.....
.....WWWWd++....
...WWWoooddW....
.WWWoioioioW....
.WWoooooooWW....
.WWoooooWWW.....
.+doooWWW.......
.WWWWWWW........
]]
            [[
.........++.....
.....WWWWd++....
...WWWoooddW....
.WWWoooioooW....
.WWooiooooWW....
.WWiooooWWW.....
.WooooWWW.......
.WW22WWW........
]]
            [[
................
.....WWWW+......
...WWWooo++.....
.WWWoooiooW.....
.WWoioooooW.....
.WWooooiWWW.....
.+ooooWW2.......
.+WWWWWW........
]]
        }
    }
    {
        name: "classic_hut", title: "Snake-stone Hut", kind: "treasure", themes: {"classic"}
        blurb: "A small stone hut; maybe an animal, maybe a shop, maybe a hoard"
        palette: "snake"
        legend: {
            '*': {spawn: "gold", chance: 0.5}
            'i': {spawn: "item", chance: 0.5}
            'e': {spawn: "enemy", chance: 0.5}
            '$': {spawn: "store", chance: 0.5}
        }
        layouts: {
            [[
...++++++
..++####+
++###$e#+
+##e**e#+
+#oooo##+
+#dd###++
++++++++.
]]
            [[
..+++++++.
.+######+.
++#eoo$#+.
+##oiio#+.
+#ooeoo#+.
+#dd####+.
+++++++++.
]]
            [[
..+++++++
.+######+
++#ooee#+
+##oooo#+
+#ooo$o#+
+#dd####+
+++++++++
]]
            [[
..++++++.
.+#####++
++#ooo#e+
+##o#oo*+
+#oo#o$e+
+#dd####+
+++++++++
]]
            [[
.+++++++.
.+#####++
++#ooo##+
+##o#e*#+
+#oo#e$#+
+#dd####+
+++++++++
]]
            [[
.+++++++.
.+#####++
++#oee##+
+##o#e$#+
+#oo#oo#+
+#dd#dd#+
+++++++++
]]
            [[
.+++++++.
.+#####++
++#oee##+
+d#o#e$d+
+di*#ood+
+#######+
+++++++++
]]
        }
    }
    {
        name: "classic_henge", title: "Standing Stones", kind: "landmark", themes: {"classic"}
        blurb: "Snake-stone pillars on a dry lawn, with offerings between them"
        palette: "stone"
        legend: {
            "'": {tile: "ground_dry"}
            'S': {spawn: "gold", chance: 0.5}
            'i': {tile: "ground_dry", spawn: "item"}
        }
        layouts: {
            [[
.#..#..
##oo##.
.oSSo+.
.oSSo+.
##oo##.
.#..#..
]]
            [[
..''''''''''''...
.''''''''''''''..
.'''S''''''S''''.
.'''''''''''''''.
.'''''''''''''''.
''''''''''''''''.
''''S''''''S''''.
''''''''''''''''.
'''''''''''''''..
.'''''''''''''...
]]
            [[
+'''''''''+
''##'#'##''
''#S'#'S#''
'''''''''''
''##'i'##''
'''''''''''
''#S'#'S#''
''##'#'##''
+'''''''''+
]]
            [[
##.#.##.
#S'#'S#.
.'''''+.
##'i'##.
.'''''+.
#S'#'S#.
##.#.##.
]]
            [[
.#...#..
##'''##.
.'S'S'+.
.''i''+.
.'S'S'+.
##'''##.
.#...#..
]]
        }
    }
    {
        name: "classic_cavern", title: "Cavern Shop", kind: "store", themes: {"classic"}
        blurb: "A trader's cave in the hillside"
        palette: "stone"
        layouts: {
            [[
..........#########.......
.....######oooooo######...
..####oooooooooooooooo###.
###oooooooo$oooooooooooo##
#oooooooooooooooooooooo##.
###ooooooooooooooooooo##..
###########....++.........
]]
            [[
.....#######.......
.....#oooo######...
..####ooooooooo##..
###ooooo$ooooooo#..
###oooooooooooo##..
######..++.........
]]
            [[
.....#########.......
.....#oooooo######...
..####ooooooooooo###.
###ooo$oooooooooooo##
###oooooooooooooo##..
######....++.........
]]
        }
    }
}
