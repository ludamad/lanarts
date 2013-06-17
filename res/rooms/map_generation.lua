require "utils"
require "tiles.tilesets"

local function first_map_fill(map) 
    local oper = MapGen.random_placement_operator {
        child_operator = MapGen.rectangle_operator { 
	        area_query = MapGen.rectangle_query { 
				fill_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_PERIMETER }, 
				perimeter_width = 1, perimeter_selector = { matches_all = MapGen.FLAG_SOLID }
			},
	        perimeter_width = 1, perimeter_operator = { add = {MapGen.FLAG_PERIMETER}, content = 2 },
	        fill_operator = { remove = {MapGen.FLAG_SOLID}, content = 1 }
	    },
        size_range = {6,9}, amount_of_placements_range = {20, 20}
    } 
    oper(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))
end

function first_map_create() 
	local solid_square = MapGen.square { flags = MapGen.FLAG_SOLID }
    local map = MapGen.map_create { size = {80,40}, fill = solid_square, instances = {} }

	first_map_fill(map)

	return game_map_create(map, map.instances)
end