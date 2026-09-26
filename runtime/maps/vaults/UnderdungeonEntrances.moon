----
-- Entrances to the Underdungeon, set in the overworld and sealed with the Dandelite Key: slime-slick
-- sinkholes, a collapsed stairwell, a barrow mound and a bone-strewn crevasse. Drawn in the
-- underdungeon's tiles: frozen floor, green bones, slime walls.
----

-- Slime that has seeped up around the way down
SLIME = {tile: "hive.floor"}

return {
    {
        name: "underdungeon_sinkhole", title: "Slime Sinkhole", kind: "entrance", themes: {"underdungeon"}
        blurb: "A round pit slick with slime, sunk into the earth, with the way down at its bottom"
        legend: {'m': SLIME}
        layouts: {
            [[
....WWWWW....
..WW#,,,#WW..
.W#,,mmm,,#W.
.W,mmpppmm,W.
W#,mpDpppm,#W
W,,mppppmm,,W
.W#,mmmmm,#W.
..WW##d##WW..
....#,o,#....
.....+o+.....
]]
            [[
...WWWWWWWWW...
.WW#,,mmm,,#WW.
W#,,mmpppmm,,#W
W,mmpppDpppmm,W
W#,,mmpppmm,,#W
.WW##,,m,,##WW.
....###d###....
......+o+......
]]
            [[
..WWWW.....
.W#mm#WW...
W#mpDm,#W..
W,mppppm#W.
W#,mpppm,W.
.WW#,mmm,#W
...W##d##W.
.....,o,...
.....+o+...
]]
        }
    }
    {
        name: "underdungeon_stairwell", title: "Collapsed Stairwell", kind: "entrance", themes: {"underdungeon"}
        blurb: "The stone frame of an old stairwell, half choked with fallen rubble, leading into the dark"
        palette: {floor: "underdungeon.floor", floor_alt: "stonework.floor", wall: "stonework.wall", wall_alt: "underdungeon.wall", rock: "quarry.wall"}
        layouts: {
            [[
.RR#######..
RR.#pppDp#..
.R.#ppp,p#..
..R#,pppR#..
..R#pp,RR#..
...##d####..
....:o:.....
....+:+.....
]]
            [[
..R########R..
.RR#pppDppp#R.
..R#,,ppp,,#..
..R#ppRRRpp#..
...#pp,,,pp#R.
...#####d###..
.R.....:o:....
.......+o+....
]]
        }
    }
    {
        name: "underdungeon_barrow", title: "Barrow Mound", kind: "entrance", themes: {"underdungeon"}
        blurb: "A grassy burial mound among old headstones, its stone passage leading down among the bones"
        palette: {floor: "underdungeon.floor_alt", floor_alt: "crypt.floor", wall: "crypt.wall_alt", wall_alt: "moss.wall"}
        base: "floor_alt"
        legend: {'b': {tile: "floor"}, 't': {like: 't', tile: "ground"}}
        layouts: {
            [[
....WWWWWWW....
..WWWWWWWWWWW..
.WWW#######WWW.
WWW##pbDbp##WWW
WW##ppbbbpp##WW
WW#,pppbppp,#WW
.WW####d####WW.
..WW..,o,..WW..
.t...::o::...t.
....t.+o+.t....
]]
            [[
..WWWWWWWWW..
.WWW#####WWW.
WW##ppDpp##WW
W##,pbbbp,##W
W#pppbbbppp#W
WW####d####WW
.WW..,o,..WW.
..t..:o:..t..
.....+o+.....
]]
        }
    }
    {
        name: "underdungeon_crevasse", title: "Bone Crevasse", kind: "entrance", themes: {"underdungeon"}
        blurb: "A jagged crack in the ground strewn with bones and skulls, narrowing into the deep"
        palette: {floor: "underdungeon.floor_alt", floor_alt: "underdungeon.floor", wall: "underdungeon.wall_alt", wall_alt: "underdungeon.wall", rock: "quarry.wall"}
        layouts: {
            [[
.......RRRR....
.....RRR,DRR...
....RR,,p,,RR..
...RR,pppp,RR..
..RR,ppk,pRR...
..RpppRR,RR....
.RRkp,RRR......
.RR##d##.......
.RR,o,.........
..++o..........
]]
            [[
..RRRRRR..........
.RR,,p,RRRR.......
RR,Dp,k,p,RRR.....
RR,,pRRRp,p,RRRR..
.RRRRR.RRk,,p,RRR.
.........RRR,,pdo+
............RRRdo+
..............RR..
]]
            [[
....RRRR.......
..RRR,DRRR.....
.RR,,pp,,RR....
.R,kppRRpk,R...
.RR,pRR.Rpp,R..
..RRRR...R,pRR.
.........RR,kR.
..........R#d#.
..........,oo+.
...........+...
]]
        }
    }
    {
        name: "underdungeon_cistern", title: "Drowned Cistern", kind: "entrance", themes: {"underdungeon"}
        blurb: "The broken roof of a cistern that once fed the halls below, its steps slick with slime"
        palette: {floor: "underdungeon.floor", floor_alt: "underdungeon.floor_alt", wall: "brick.wall_alt", wall_alt: "underdungeon.wall_alt"}
        legend: {'m': SLIME}
        layouts: {
            [[
.WWWWWWWWWW.
.WmmpppmmmW.
.WmsmpDmsmW.
.WmpppppmmW.
.WmsmpppsmW.
.WWWWWdWWWW.
....,,o,,...
......++....
]]
            [[
..WWWWWWW..
.WWmmpmmWW.
.WmsppDsmW.
.WmpppppmW.
.WmsmppsmW.
.WmmmmmmmW.
.WWWWdWWWW.
.....o,....
....+o+....
]]
        }
    }
}
