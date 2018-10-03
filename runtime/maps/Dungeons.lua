local SourceMap = require "core.SourceMap"
local GameObject = require "core.GameObject"

local TileSets = require "tiles.Tilesets"

local map_utils = require "maps.MapUtils"
local PortalSet = require "maps.PortalSet"

local M = {} -- Submodule

function M.room_carve_operator(wall_tile, floor_tile, --[[Optional]] padding)
    return SourceMap.rectangle_operator {
        area_query = SourceMap.rectangle_criteria {
                        fill_selector = { matches_all = SourceMap.FLAG_SOLID, matches_none = SourceMap.FLAG_PERIMETER },
                        perimeter_width = padding or 1, perimeter_selector = { matches_all = SourceMap.FLAG_SOLID }
        },
        perimeter_width = padding or 1, perimeter_operator = {
            add = {SourceMap.FLAG_SOLID, SourceMap.FLAG_PERIMETER},
            remove = SourceMap.FLAG_SEETHROUGH,
            content = wall_tile
    },
        fill_operator = { add = SourceMap.FLAG_SEETHROUGH, remove = SourceMap.FLAG_SOLID, content = floor_tile }
    }
end

function M.simple_tunnels(map, width_range, tunnels_per_room, wall_tile, floor_tile, --[[Optional]] area, --[[Optional]] padding)
    area = area or bbox_create({0,0}, map.size)
    event_log("(RNG #%d) pre simple tunnels", map.rng:amount_generated())
    local oper = SourceMap.tunnel_operator {
        rng = map.rng,
        validity_selector = {
            fill_selector = { matches_all = SourceMap.FLAG_SOLID, matches_none = SourceMap.FLAG_TUNNEL },
            perimeter_selector = { matches_all = SourceMap.FLAG_SOLID, matches_none = SourceMap.FLAG_TUNNEL }
        },

        completion_selector = {
            fill_selector = { matches_none = { SourceMap.FLAG_SOLID, SourceMap.FLAG_PERIMETER, SourceMap.FLAG_TUNNEL } },
            perimeter_selector = { matches_none = SourceMap.FLAG_SOLID }
        },

        fill_operator = { add = {SourceMap.FLAG_TUNNEL, SourceMap.FLAG_SEETHROUGH}, remove = SourceMap.FLAG_SOLID, content = floor_tile},
        perimeter_operator = {
            matches_all = SourceMap.FLAG_SOLID, remove = SourceMap.FLAG_SEETHROUGH,
            add = {SourceMap.FLAG_SOLID, SourceMap.FLAG_TUNNEL, SourceMap.FLAG_PERIMETER}, content = wall_tile},

        perimeter_width = padding or 1,
        size_range = width_range,
        tunnels_per_room_range = tunnels_per_room
    }
    event_log("(RNG #%d) post simple tunnels", map.rng:amount_generated())
    oper(map, SourceMap.ROOT_GROUP, area)
end

function M.simple_random_placement_operator(map, tileset)
    event_log("(RNG #%d) pre random placement oper", map.rng:amount_generated())
    return SourceMap.random_placement_operator {
        rng = map.rng,
        child_operator = M.room_carve_operator(tileset.wall, tileset.floor),
        size_range = chance(.5) and {12,15} or {5,8}, amount_of_placements_range = {3,4},
        create_subgroup = false
    }
end

function M.simple_bsp_operator(map, tileset)
    return SourceMap.bsp_operator {
        rng = map.rng,
        child_operator = function (map, group, bbox)
            return M.simple_random_placement_operator(map, tileset)(map, group,bbox)
        end,
        split_depth = 2, minimum_node_size = {20,20}, randomize_size = false,
        create_subgroup = false
    }
end

return M
