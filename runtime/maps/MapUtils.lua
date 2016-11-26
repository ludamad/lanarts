local DungeonFeatures = require "objects.DungeonFeatures"

local GameObject = require "core.GameObject"
local SourceMap = require "core.SourceMap"
local Map = require "core.Map"
local mtwist = require "mtwist"

local M = {} -- Submodule

function M.from_tile_xy(xy)
    return {xy[1]*32+16, xy[2]*32+16}
end

function M.random_square(map, area, --[[Optional]] selector, --[[Optional]] operator, --[[Optional]] max_attempts) 
    return SourceMap.find_random_square { 
        rng = map.rng, map = map, area = area, 
        selector = selector or { matches_none = {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT} },
        operator = operator or { add = SourceMap.FLAG_HAS_OBJECT },
        max_attempts = max_attempts
    }
end

function M.spawn_enemy(map, type, tile_xy)
    local object = GameObject.enemy_create {
        do_init = false,
        xy = M.from_tile_xy(tile_xy),
        type = type,
    }
    table.insert(map.instances, object)
    return object

end

function M.spawn_store(map, items, tile_xy)
    local object = GameObject.store_create {
        xy = M.from_tile_xy(tile_xy),
        items = items,
        do_init = false,
        sprite = "store",
        frame = random(0,4)
    }
    table.insert(map.instances, object)
end

function M.spawn_item(map, type, amount, tile_xy)
    local object = GameObject.item_create {
        do_init = false,
        xy = M.from_tile_xy(tile_xy),
        type = type,
        amount = amount,
        type = type,
    }
    table.insert(map.instances, object)
    return object
end

function M.random_store(map, items, area) 
    local sqr = M.random_square(map, area)
    if not sqr then return nil end
    return M.spawn_store(map, items, sqr)
end

function M.random_item(map, type, amount, area) 
    local sqr = M.random_square(map, area)
    if not sqr then return nil end
    return M.spawn_item(map, type, amount, sqr)
end

function M.random_enemy(map, type, area) 
    local sqr = M.random_square(map, area)
    if not sqr then return nil end
    return M.spawn_enemy(map, type, sqr)
end

function M.spawn_door(map, sqr)
    local object = DungeonFeatures.Door.create { do_init = false, xy = M.from_tile_xy(sqr) }
    table.insert(map.instances, object)
    return object
end

function M.spawn_decoration(map, sprite, sqr, frame, solid)
    if solid == nil then solid = true end 
    local object = DungeonFeatures.Decoration.create {
        do_init = false,
        xy = M.from_tile_xy(sqr),
        type = DungeonFeatures.Decoration,
        sprite = sprite,
        frame = frame
    } 
    table.insert(map.instances, object)
    return object
end

function M.spawn_portal(map, sqr, sprite, --[[Optional]] callback, --[[Optional]] frame)
    local object = GameObject.feature_create {
        do_init = false,
        xy = M.from_tile_xy(sqr),
        type = GameObject.PORTAL,
        sprite = sprite,
        on_player_interact = callback,
        frame = frame
    }
    table.insert(map.instances, object)
    return object
end

function M.random_portal(map, area, sprite, callback, --[[Optional]] frame) 
    local sqr = M.random_square(map, area)
    if not sqr then return nil end
    return M.spawn_portal(map, sqr, sprite, callback, frame)
end

function M.map_create(label, size, content, --[[Optional]] flags)
    return SourceMap.map_create { 
        rng = mtwist.create(random(0, 2 ^ 31)),
    	label = label, 
    	size = size, 
    	flags = flags or SourceMap.FLAG_SOLID, 
    	content = content,
    	instances = {} 
    }
end

function M.game_map_create(map, wandering_enabled) 
    if wandering_enabled == nil then wandering_enabled = false end
    return Map.create { map = map, label = map.label, instances = map.instances, wandering_enabled = wandering_enabled }
end

function M.area_template_apply(map, area, filename, legend)
    local area_temp = SourceMap.area_template_create {data_file = filename, legend = legend}
    area_temp:apply{ map = map, area = area }
end

function M.area_template_to_map(label, filename, padding, legend)
	local area_temp = SourceMap.area_template_create {data_file = filename, legend = legend}

    local orient = random_choice {
        SourceMap.ORIENT_DEFAULT, SourceMap.ORIENT_FLIP_X, SourceMap.ORIENT_FLIP_Y,
        SourceMap.ORIENT_TURN_90, SourceMap.ORIENT_TURN_180, SourceMap.ORIENT_TURN_270
    }
    local size = vector_add(area_temp.size, {padding*2,padding*2})
    if orient == SourceMap.ORIENT_TURN_90 or orient == SourceMap.ORIENT_TURN_270 then
        size[1], size[2] = size[2], size[1]
    end

    local map = M.map_create(label, size, legend['x'].content)
    area_temp:apply{ map = map, top_left_xy = {padding,padding}, orientation = orient }
    return map
end

function M.make_tunnel_oper(rng, floor, wall, wall_seethrough) 
    wall_flags = {TileMap.FLAG_SOLID, TileMap.FLAG_TUNNEL, TileMap.FLAG_PERIMETER}
    remove_flags = {}
    if wall_seethrough then
        append(wall_flags, TileMap.FLAG_SEETHROUGH)
    else
        append(remove_flags, TileMap.FLAG_SEETHROUGH)
    end
    return TileMap.tunnel_operator {
        validity_selector = { 
            fill_selector = { matches_all = TileMap.FLAG_SOLID, matches_none = TileMap.FLAG_TUNNEL },
            perimeter_selector = { matches_all = TileMap.FLAG_SOLID, matches_none = TileMap.FLAG_TUNNEL }
        },

        completion_selector = {
            fill_selector = { matches_none = {TileMap.FLAG_SOLID, TileMap.FLAG_PERIMETER, TileMap.FLAG_TUNNEL} },
            perimeter_selector = { matches_none = TileMap.FLAG_SOLID } 
        },
        fill_operator = { add = {TileMap.FLAG_SEETHROUGH, TileMap.FLAG_TUNNEL}, remove = TileMap.FLAG_SOLID, content = floor},
        perimeter_operator = { matches_all = TileMap.FLAG_SOLID, add = wall_flags, remove = remove_flags, content = wall},

        rng = rng,
        perimeter_width = 1,
        size_range = {1,2},
        tunnels_per_room_range = {1,2}
    }
end

function M.make_rectangle_criteria()
        return TileMap.rectangle_criteria { 
                fill_selector = { matches_all = TileMap.FLAG_SOLID, matches_none = TileMap.FLAG_PERIMETER }, 
                perimeter_width = 1, 
                perimeter_selector = { matches_all = TileMap.FLAG_SOLID }
        }
end

function M.make_rectangle_oper(floor, wall, wall_seethrough, --[[Optional]] area_query)
    wall_flags = {TileMap.FLAG_SOLID}
    remove_wall_flags = {TileMap.FLAG_SEETHROUGH}
    if wall_seethrough then
        append(wall_flags, TileMap.FLAG_SEETHROUGH)
        remove_wall_flags = {}
    end
    return TileMap.rectangle_operator { 
        area_query = area_query,
        perimeter_width = 1,
       fill_operator = { add = {TileMap.FLAG_CUSTOM5, TileMap.FLAG_SEETHROUGH}, remove = {TileMap.FLAG_SOLID}, content = floor},
        perimeter_operator = { add = {TileMap.FLAG_PERIMETER}, remove = remove_wall_flags, content = wall },
    }
end



return M
