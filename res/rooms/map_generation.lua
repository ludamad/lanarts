require "utils"
require "tiles.tilesets"

local function simple_fill(map, tileset) 
    local oper = MapGen.random_placement_operator {
        child_operator = MapGen.rectangle_operator { 
	        area_query = MapGen.rectangle_query { 
				fill_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_PERIMETER }, 
				perimeter_width = 1, perimeter_selector = { matches_all = MapGen.FLAG_SOLID }
			},
	        perimeter_width = 1, perimeter_operator = { add = {MapGen.FLAG_SOLID, MapGen.FLAG_PERIMETER}, content = tileset.wall },
	        fill_operator = { remove = {MapGen.FLAG_SOLID}, content = tileset.floor }
	    },
        size_range = {6,9}, amount_of_placements_range = {2, 4}
    } 
    oper(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))
end

function first_map_create() 
	local tileset = TileSets.grass
	local solid_square = MapGen.square { flags = MapGen.FLAG_SOLID, content = tileset.floor}
    local map = MapGen.map_create { size = {80,40}, fill = solid_square, instances = {} }

	simple_fill(map, tileset)

	local map_id = World.map_create { map = map, instances = map.instances }
	World.players_spawn(map_id)
	return map_id
end