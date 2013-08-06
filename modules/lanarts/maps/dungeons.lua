local utils = import "core.utils"
local MapGen = import "core.map_generation"
local Maps = import "core.maps"
local GameObject = import "core.GameObject"

local TileSets = import "@tiles.tilesets"

local map_utils = import ".map_utils"
local PortalSet = import ".PortalSet"

local M = {} -- Submodule

function M.room_carve_operator(wall_tile, floor_tile, --[[Optional]] padding)
    return MapGen.rectangle_operator { 
        area_query = MapGen.rectangle_criteria { 
                        fill_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_PERIMETER }, 
                        perimeter_width = padding or 1, perimeter_selector = { matches_all = MapGen.FLAG_SOLID }
        },
        perimeter_width = padding or 1, perimeter_operator = { add = {MapGen.FLAG_SOLID, MapGen.FLAG_PERIMETER}, content = wall_tile },
        fill_operator = { add = MapGen.FLAG_SEETHROUGH, remove = MapGen.FLAG_SOLID, content = floor_tile }
    }
end

function M.simple_tunnels(map, width_range, tunnels_per_room, wall_tile, floor_tile, --[[Optional]] area, --[[Optional]] padding) 
    area = area or bbox_create({0,0}, map.size)
    local oper = MapGen.tunnel_operator {
        validity_selector = { 
            fill_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_TUNNEL },
            perimeter_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_TUNNEL }
        },

        completion_selector = {
            fill_selector = { matches_none = { MapGen.FLAG_SOLID, MapGen.FLAG_PERIMETER, MapGen.FLAG_TUNNEL } },
            perimeter_selector = { matches_none = MapGen.FLAG_SOLID } 
        },

        fill_operator = { add = {MapGen.FLAG_TUNNEL, MapGen.FLAG_SEETHROUGH}, remove = MapGen.FLAG_SOLID, content = floor_tile},
        perimeter_operator = { matches_all = MapGen.FLAGS_SOLID, add = {MapGen.FLAG_SOLID, MapGen.FLAG_TUNNEL, MapGen.FLAG_PERIMETER}, content = wall_tile},

        perimeter_width = padding or 1,
        size_range = width_range,
        tunnels_per_room_range = tunnels_per_room
    }
    oper(map, MapGen.ROOT_GROUP, area)
end

function M.simple_random_placement_operator(map, tileset) 
    return MapGen.random_placement_operator {
        child_operator = M.room_carve_operator(tileset.wall, tileset.floor),
        size_range = chance(.5) and {12,15} or {5,8}, amount_of_placements_range = {3,4},
        create_subgroup = false
    } 
end

function M.simple_bsp_operator(map, tileset) 
    return MapGen.bsp_operator {
        child_operator = function (map, group, bbox) 
            return M.simple_random_placement_operator(map, tileset)(map, group,bbox)
        end,
        split_depth = 2, minimum_node_size = {20,20}, randomize_size = false,
        create_subgroup = false
    }
end

return M