local Relations = import "lanarts.objects.relations"
local MapGen = import "core.map_generation"
local GameMap = import "core.GameMap"
local GameObject = import "core.GameObject"

module( ..., lunit.testcase, package.seeall )

local test_tile = Data.tile_create { 
    images = {
        image_load (path_resolve "test_tile.png")
    }
}

local function create_with_teams(gmap, ...)
    local instances = {}
    for team in values{...} do
        local obj = GameObject.object_create {
            map = gmap,
            xy = {0,0}
        }
        obj.team = team
        table.insert(instances, obj)
    end

    return unpack(instances)
end

local function test_default_resolution(gmap, relation1, relation2, expected)
    local is_hostile = (expected == Relations.HOSTILE)
    local is_friendly = (expected == Relations.FRIENDLY)
    local is_neutral = (expected == Relations.NEUTRAL)

    local team1, team2 = Relations.team_create(relation1), Relations.team_create(relation2)
    local obj1, obj2 = create_with_teams(gmap, team1, team2)

    lunit.assert_equal(expected, Relations.team_get_relationship(team1, team2))
    lunit.assert_equal(expected, Relations.get_relationship(obj1, obj2))

    lunit.assert_equal(is_hostile, Relations.is_hostile(obj1, obj2))
    lunit.assert_equal(is_friendly, Relations.is_friendly(obj1, obj2))
    lunit.assert_equal(is_neutral, Relations.is_neutral(obj1, obj2))
end

function test()
    local gmap = GameMap.create { 
        map = MapGen.map_create { 
            label = "TestLevel", 
            size = {1,1},            
            content = test_tile
        } 
    }

    test_default_resolution(gmap, Relations.HOSTILE, Relations.FRIENDLY, Relations.HOSTILE)
    test_default_resolution(gmap, Relations.HOSTILE, Relations.HOSTILE, Relations.HOSTILE)
    test_default_resolution(gmap, Relations.NEUTRAL, Relations.HOSTILE, Relations.NEUTRAL)
    test_default_resolution(gmap, Relations.NEUTRAL, Relations.FRIENDLY, Relations.NEUTRAL)
    test_default_resolution(gmap, Relations.FRIENDLY, Relations.FRIENDLY, Relations.FRIENDLY)
end