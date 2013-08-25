local Relations = import "lanarts.objects.relations"
local MapGen = import "core.map_generation"
local GameMap = import "core.GameMap"
local GameObject = import "core.GameObject"

-- Dead simple, but valid, 1x1 test game-map. All objects are stacked on square {0,0}.
local gmap = GameMap.create { 
    map = MapGen.map_create { 
        label = "TestLevel", 
        size = {1,1},            
        content = Data.tile_create { 
            images = {
                image_load (path_resolve "test_tile.png")
            }
        }
    }
}

local NEUTRAL, HOSTILE, FRIENDLY = Relations.NEUTRAL, Relations.HOSTILE, Relations.FRIENDLY

local function create_with_teams(...)
    local instances = {}
    for team in values{...} do
        local obj = GameObject.object_create{ map = gmap, xy = {0,0} }
        obj.team = team
        table.insert(instances, obj)
    end

    return unpack(instances)
end

local function test_expected_resolution(team1, team2, expected)
    local obj1, obj2 = create_with_teams(team1, team2)
    lunit.assert_equal(expected, Relations.team_get_relationship(team1, team2))
    lunit.assert_equal(expected, Relations.get_relationship(obj1, obj2))

    lunit.assert_equal( (expected == HOSTILE),  Relations.is_hostile(obj1, obj2)   )
    lunit.assert_equal( (expected == FRIENDLY), Relations.is_friendly(obj1, obj2)  )
    lunit.assert_equal( (expected == NEUTRAL),  Relations.is_neutral(obj1, obj2)   )
end

function TestCases.default_relationship_resolution()
    local expected_resolutions = {
        {HOSTILE,  HOSTILE,  --[[ => ]] HOSTILE},
        {HOSTILE,  FRIENDLY, --[[ => ]] HOSTILE},
        {NEUTRAL,  HOSTILE,  --[[ => ]] NEUTRAL},
        {NEUTRAL,  NEUTRAL,  --[[ => ]] NEUTRAL},
        {NEUTRAL,  FRIENDLY, --[[ => ]] NEUTRAL},
        {FRIENDLY, FRIENDLY, --[[ => ]] FRIENDLY}
    }

    for v in values(expected_resolutions) do
        test_expected_resolution(Relations.team_create(v[1]), Relations.team_create(v[2]), v[3])
    end
end

-- Test simple relationship setting.
function TestCases.simple_relationship_resolution()
    local relations = {NEUTRAL, HOSTILE, FRIENDLY}

    for relation in values(relations) do
        local team1, team2 = Relations.team_create(), Relations.team_create()
        Relations.team_set_relationship(team1, team2, relation)
        test_expected_resolution(team1, team2, relation)
    end
end

-- Test parent chain resolution.
-- If your parent is the enemy of my pair, we are enemies EXCEPT if you are explicitly my friend.
function TestCases.inherited_relationship_resolution()
    local relations = {false, NEUTRAL, HOSTILE, FRIENDLY}

    -- Tries to be somewhat exhaustive
    for parent_relation in values(relations) do
        parent_relation = parent_relation or nil -- Convert false -> nil
        local parent1, parent2 = Relations.team_create(), Relations.team_create()
        Relations.team_set_relationship(parent1, parent2, parent_relation)

        for default in values{NEUTRAL, HOSTILE, FRIENDLY} do
            for child_relation in values(relations) do
                child_relation = child_relation or nil -- Convert false -> nil

                local child1 = Relations.team_create(default, parent1)
                local child2 = Relations.team_create(default, parent2)
                Relations.team_set_relationship(child1, child2, child_relation)
                if child_relation ~= nil then
                    test_expected_resolution(child1, child2, child_relation)
                elseif parent_relation ~= nil then
                    test_expected_resolution(child1, child2, parent_relation)
                else
                    test_expected_resolution(child1, child2, default)
                end
            end
        end
    end
end