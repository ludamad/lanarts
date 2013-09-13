local GameMap = import "core.GameMap"
local GameObject = import "core.GameObject"
local MapGen = import "core.MapGeneration"
local Display = import "core.Display"

local gmap = GameMap.create { 
    map = MapGen.map_create { 
        label = "TestLevel", 
        size = {256,256},            
        content = Data.tile_create { 
            images = {
                Display.images_load (path_resolve "test_tile.png")
            }
        }
    }
}

function TestCases.rectangle_collision_check()
    local object_in = GameObject.object_create{ map = gmap, xy = {75,75}, radius = 10 }
    local object_out = GameObject.object_create{ map = gmap, xy = {25,25}, radius = 10 }
    local instances = GameMap.rectangle_collision_check(gmap, {50,50,150,150}, nil)
    assert(#instances == 1)
    assert(instances[1].id == object_in.id)
end
