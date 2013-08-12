local GameObject = import "core.GameObject"
local MapGen = import "core.map_generation"
local GameMap = import "core.GameMap"

local M = {} -- Submodule

function M.random_square(map, area, --[[Optional]] selector, --[[Optional]] operator) 
    return MapGen.find_random_square { 
        map = map, 
        selector = selector or { matches_none = {MapGen.FLAG_SOLID, MapGen.FLAG_HAS_OBJECT} },
        operator = operator or { add = MapGen.FLAG_HAS_OBJECT },
        area = area
    }
end

function M.spawn_enemy(map, type, tile_xy)
    local object = GameObject.enemy_create {
        do_init = false,
        xy = {tile_xy[1]*32+16, tile_xy[2]*32+16},
        type = type,
    }
    table.insert(map.instances, object)
    return object

end

function M.spawn_store(map, items, tile_xy)
    local object = GameObject.store_create {
        xy = {tile_xy[1]*32+16, tile_xy[2]*32+16},
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
        xy = {tile_xy[1]*32+16, tile_xy[2]*32+16},
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
    local object = GameObject.feature_create {
        do_init = false,
        xy = {sqr[1]*32+16, sqr[2]*32+16},
        type = GameObject.DOOR_CLOSED,
        sprite = "closed door",
    }
    table.insert(map.instances, object)
    return object
end

function M.spawn_decoration(map, sprite, sqr, frame, solid)
    if solid == nil then solid = true end 
    local object = GameObject.feature_create {
        do_init = false,
        solid = solid,
        xy = {sqr[1]*32+16, sqr[2]*32+16},
        type = GameObject.OTHER,
        sprite = sprite,
        frame = frame
    }
    table.insert(map.instances, object)
    return object
end

function M.spawn_portal(map, sqr, sprite, --[[Optional]] callback, --[[Optional]] frame)
    local object = GameObject.feature_create {
        do_init = false,
        xy = {sqr[1]*32+16, sqr[2]*32+16},
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
    return MapGen.map_create { 
    	label = label, 
    	size = size, 
    	flags = flags or MapGen.FLAG_SOLID, 
    	content = content,
    	instances = {} 
    }
end

function M.game_map_create(map, wandering_enabled) 
    if wandering_enabled == nil then wandering_enabled = false end
    return GameMap.create { map = map, label = map.label, instances = map.instances, wandering_enabled = wandering_enabled }
end

function M.area_template_apply(map, area, filename, legend)
    local area_temp = MapGen.area_template_create {data_file = filename, legend = legend}
    area_temp:apply{ map = map, area = area }
end

function M.area_template_to_map(label, filename, padding, legend)
	local area_temp = MapGen.area_template_create {data_file = filename, legend = legend}

    local orient = random_choice {
        MapGen.ORIENT_DEFAULT, MapGen.ORIENT_FLIP_X, MapGen.ORIENT_FLIP_Y,
        MapGen.ORIENT_TURN_90, MapGen.ORIENT_TURN_180, MapGen.ORIENT_TURN_270
    }
    local size = vector_add(area_temp.size, {padding*2,padding*2})
    if orient == MapGen.ORIENT_TURN_90 or orient == MapGen.ORIENT_TURN_270 then
        size[1], size[2] = size[2], size[1]
    end

    local map = M.map_create(label, size, legend['x'].content)
    area_temp:apply{ map = map, top_left_xy = {padding,padding}, orientation = orient }
    return map
end

return M
