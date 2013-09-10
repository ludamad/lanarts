local GameMap = import "core.GameMap"
local MapGen = import "core.MapGeneration"

local SimulationMap = newtype()

function SimulationMap:init()
	self.gmap = GameMap.create { 
        map = MapGen.map_create { 
            label = "TestLevel", 
            size = {256,256},            
            content = Data.tile_create { 
                images = {
                    image_cached_load (path_resolve "test_content/test_tile.png")
                }
            }
    }
    self.enemies = {}
    self.players = {}
end

function SimulationMap:add_enemy(enemy, xy)
    
end

return SimulationMap