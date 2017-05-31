local Map = require "core.Map"
local GameObject = require "core.GameObject"
local SourceMap = require "core.SourceMap"
local Display = require "core.Display"

local function gmap_create(size, --[[Optional]] solid_tiles)
    local gmap = Map.create {
        map = SourceMap.map_create { 
            label = "TestLevel", size = size,        
            content = Data.tile_create { 
                images = {
                    Display.image_load (path_resolve "test_tile.png")
                }
            }
        }
    }
    for _, xy in ipairs(solid_tiles or {}) do
        Map.tile_set_solid(gmap, xy, true)
    end
    return gmap
end

function TestCases.rectangle_collision_check()
    local gmap = gmap_create({256,256})
    local object_in = GameObject.object_create{ map = gmap, xy = {75,75}, radius = 10 }
    local object_out = GameObject.object_create{ map = gmap, xy = {25,25}, radius = 10 }
    local instances = Map.rectangle_collision_check(gmap, {50,50,150,150}, nil)
    assert(#instances == 1)
    assert(instances[1].id == object_in.id)
end

function TestCases.line_tile_check()
    local gmap = gmap_create({256,256}, {{1,1}})
    local hit_xy = Map.line_tile_check(gmap, {-12,48},{96+12,48})
    assert(hit_xy and hit_xy[1] == 1 and hit_xy[2] == 1)
    assert(not Map.line_tile_check(gmap, {-12,16},{96+12,16}))
    assert(not Map.line_tile_check(gmap, {-12,90},{96+12,90}))
    hit_xy = Map.line_tile_check(gmap, {-12,48},{48,48})
    assert(hit_xy and hit_xy[1] == 1 and hit_xy[2] == 1)
end
