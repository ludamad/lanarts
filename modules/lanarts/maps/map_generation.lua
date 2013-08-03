local utils = import "core.utils"
import "@tiles.tilesets"
local MapGen = import "core.map_generation"
local Maps = import "core.maps"
local GameObject = import "core.GameObject"

local M = {} -- Submodule

local function room_carve_operator(tileset)
    return MapGen.rectangle_operator { 
        area_query = MapGen.rectangle_criteria { 
                        fill_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_PERIMETER }, 
                        perimeter_width = 1, perimeter_selector = { matches_all = MapGen.FLAG_SOLID }
        },
        perimeter_width = 1, perimeter_operator = { add = {MapGen.FLAG_SOLID, MapGen.FLAG_PERIMETER}, content = tileset.wall },
        fill_operator = { add = MapGen.FLAG_SEETHROUGH, remove = MapGen.FLAG_SOLID, content = tileset.floor }
    }
end

local function simple_random_placement_operator(map, tileset) 
    print "SIMPLE_RANDOM_PLACEMENT_OPERATOR"
    return MapGen.random_placement_operator {
        child_operator = room_carve_operator(tileset),
        size_range = chance(.5) and {15,15} or {5,5}, amount_of_placements_range = {3,4},
        create_subgroup = false
    } 
end

local function simple_bsp_operator(map, tileset) 
    return MapGen.bsp_operator {
        child_operator = function (map, group, bbox) 
            return simple_random_placement_operator(map, tileset)(map, group,bbox)
        end,
        split_depth = 2, minimum_node_size = {20,20}, randomize_size = false,
        create_subgroup = false
    }
end

local function simple_tunnels(map, tileset) 
    local oper = MapGen.tunnel_operator {
        validity_selector = { 
            fill_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_TUNNEL },
            perimeter_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_TUNNEL }
        },

        completion_selector = {
            fill_selector = { matches_none = { MapGen.FLAG_SOLID, MapGen.FLAG_PERIMETER, MapGen.FLAG_TUNNEL } },
            perimeter_selector = { matches_none = MapGen.FLAG_SOLID } 
        },

        fill_operator = { add = {MapGen.FLAG_TUNNEL, MapGen.FLAG_SEETHROUGH}, remove = MapGen.FLAG_SOLID, content = tileset.floor},
        perimeter_operator = { matches_all = MapGen.FLAGS_SOLID, add = {MapGen.FLAG_SOLID, MapGen.FLAG_TUNNEL, MapGen.FLAG_PERIMETER}, content = tileset.wall},

        perimeter_width = 1,
        size_range = {1,4},
        tunnels_per_room_range = {1,2}
    }
    oper(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))
end

local function random_enemy(map, type) 
    local sqr = MapGen.find_random_square { 
        map = map, 
        selector = { matches_none = {MapGen.FLAG_SOLID, MapGen.FLAG_HAS_OBJECT} },
        operator = { add = MapGen.FLAG_HAS_OBJECT } 
    }
    table.insert(map.instances, GameObject.enemy_create {
        do_init = false,
        xy = {sqr[1]*32+16, sqr[2]*32+16},
        type = type
    })
end

local function area_temp_apply()
    local tileset = TileSets.grass
    local area_temp = MapGen.area_template_create {
        data_file = path_resolve "test-template.txt",
        legend = { ['x'] = { add = {MapGen.FLAG_SEETHROUGH, MapGen.FLAG_SOLID}, content = tileset.wall }, 
                   ['.'] = { add = MapGen.FLAG_SEETHROUGH, content = tileset.floor } }
    }
    local map = MapGen.map_create { size = area_temp.size, content = tileset.wall, instances = {} }
    area_temp:apply { map = map }
    for i=1,10 do random_enemy(map, "Giant Bat") end
    for i=1,10 do random_enemy(map, "Giant Rat") end
    local map_id = Maps.create { map = map, instances = map.instances }
    World.players_spawn(map_id)
    return map_id
end

M.first_map_create = area_temp_apply

local function sfirst_map_create() 
    local tileset = TileSets.grass
    local map = MapGen.map_create { size = {80,80}, flags = MapGen.FLAG_SOLID, content = tileset.wall, instances = {} }

    simple_bsp_operator(map, tileset)(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))
    simple_tunnels(map, tileset)

    local map_id = World.map_create { map = map, instances = map.instances }
    World.players_spawn(map_id)
    return map_id
end

return M
