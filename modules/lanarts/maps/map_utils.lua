local GameObject = import "core.GameObject"
local MapGen = import "core.map_generation"
local Maps = import "core.maps"

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
        area = area
    }
    table.insert(map.instances, object)
    return object

end

function M.random_enemy(map, type, area) 
    local sqr = M.random_square(map, area)
    if not sqr then return nil end
    return M.spawn_enemy(map, type, sqr)
end

function M.spawn_decoration(map, sprite, sqr) 
    local object = GameObject.feature_create {
        do_init = false,
        solid = true,
        xy = {sqr[1]*32+16, sqr[2]*32+16},
        type = GameObject.OTHER,
        sprite = sprite,
    }
    table.insert(map.instances, object)
    return object
end

function M.spawn_portal(map, sqr, sprite, callback, --[[Optional]] frame)
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
--
--function M.random_portal_connect(map, area, sprite, portals, direction, portal_key, next_map)
--    local portal = M.random_portal(map, sprite, function(self, user)
--            if not self.connected_map then
--                if type(next_map) == "function" then
--                    self.connected_map = next_map(self, user)
--                else 
--                    self.connected_map = next_map
--                end
--            end
--            local methodname = direction == "start" and "get_end" or "get_start" -- Go to opposite end
--            Maps.transfer(user, self.connected_map, portals[methodname](portals, portal_key))
--    end)
--    local methodname = direction == "start" and "set_start" or "set_end"
--    portals[methodname](portals, portal_key, portal.xy)
--    return portal
--end

function M.map_create(size, content, --[[Optional]] flags)
    return MapGen.map_create { size = size, flags = flags or MapGen.FLAG_SOLID, content = content, instances = {} }
end

function M.game_map_create(map, wandering_enabled) 
    if wandering_enabled == nil then wandering_enabled = false end
    return Maps.create { map = map, instances = map.instances, wandering_enabled = wandering_enabled }
end

function M.area_template_apply(map, area, filename, legend)
    local area_temp = MapGen.area_template_create {data_file = filename, legend = legend}
    area_temp:apply{ map = map, area = area }
end

function M.area_template_to_map(filename, legend)
	local area_temp = MapGen.area_template_create {data_file = filename, legend = legend}

	local map = M.map_create(area_temp.size, 0)
	area_temp:apply{ map = map }
	return map
end

return M