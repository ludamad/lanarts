local DungeonFeatures = require "objects.DungeonFeatures"

local GameObject = require "core.GameObject"
local SourceMap = require "core.SourceMap"
local Map = require "core.Map"
local World = require "core.World"
local mtwist = require "mtwist"
local HealingSquare = require("objects.HealingSquare").HealingSquare

local M = {} -- Submodule

function M.from_tile_xy(xy)
    return {xy[1]*32+16, xy[2]*32+16}
end

function M.random_square(map, area, --[[Optional]] selector, --[[Optional]] operator, --[[Optional]] max_attempts) 
    assert(map)
    assert(not area or #area > 0)
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

function M.random_enemy(map, type, --[[Optional]] area, --[[Optional]] selector) 
    local sqr = M.random_square(map, area, selector)
    if not sqr then return nil end
    return M.spawn_enemy(map, type, sqr)
end

function M.spawn_healing_square(map, sqr)
    local object = HealingSquare.create { do_init = false, xy = M.from_tile_xy(sqr) }
    table.insert(map.instances, object)
    return object
end

M.lanarts_door_sprite = tosprite("spr_doors.sealed_door")
function M.spawn_lanarts_door(map, sqr)
    local object = DungeonFeatures.Door.create { do_init = false, xy = M.from_tile_xy(sqr), closed_sprite = M.lanarts_door_sprite, lanarts_needed = 3}
    table.insert(map.instances, object)
    return object
end


function M.spawn_door(map, sqr, --[[Optional]] open_sprite, --[[Optional]] closed_sprite, --[[Optional]] required_key)
    local object = DungeonFeatures.Door.create { do_init = false, xy = M.from_tile_xy(sqr), open_sprite = open_sprite, closed_sprite = closed_sprite, required_key = required_key }
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
    local Vaults = require "maps.Vaults"
    local sqr = M.random_square(map, area,
            --[[Selector]] {matches_none = {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT} })
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

function M.pick_player_squares(map, positions) 
    local picked = {}
    assert(#positions > 0, "No player positions!")
    for i=1,World.player_amount do
        local sqr = map.rng:random_choice(positions)
        table.remove_occurrences(positions, sqr)
        append(picked, sqr)
    end
    if #picked < World.player_amount then
        return nil
    end
    return picked
end
function M.find_player_positions(map, --[[Optional]] flags) 
    local positions = {}
    local map_area = bbox_create({0,0},map.size)
    for i=1,World.player_amount do
        local sqr = M.random_square(map, map_area, 
            --[[Selector]] { matches_all = flags, matches_none = {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT} },
            --[[Operator]] nil, 
            --[[Max attempts]] 10000
        )
        if not sqr then error("Could not find player spawn position for player " .. i .. "!") end
        positions[i] = {(sqr[1]+.5) * Map.TILE_SIZE, (sqr[2]+.5) * Map.TILE_SIZE}
    end
    return positions
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
    local wall_flags = {SourceMap.FLAG_SOLID, SourceMap.FLAG_TUNNEL, SourceMap.FLAG_PERIMETER}
    local remove_flags = {}
    if wall_seethrough then
        append(wall_flags, SourceMap.FLAG_SEETHROUGH)
    else
        append(remove_flags, SourceMap.FLAG_SEETHROUGH)
    end
    return SourceMap.tunnel_operator {
        validity_selector = { 
            fill_selector = { matches_all = SourceMap.FLAG_SOLID, matches_none = SourceMap.FLAG_TUNNEL },
            perimeter_selector = { matches_all = SourceMap.FLAG_SOLID, matches_none = SourceMap.FLAG_TUNNEL }
        },

        completion_selector = {
            fill_selector = { matches_none = {SourceMap.FLAG_SOLID, SourceMap.FLAG_PERIMETER, SourceMap.FLAG_TUNNEL} },
            perimeter_selector = { matches_none = SourceMap.FLAG_SOLID } 
        },
        fill_operator = { add = {SourceMap.FLAG_SEETHROUGH, SourceMap.FLAG_TUNNEL}, remove = SourceMap.FLAG_SOLID, content = floor},
        perimeter_operator = { matches_all = SourceMap.FLAG_SOLID, add = wall_flags, remove = remove_flags, content = wall},

        rng = rng,
        perimeter_width = 1,
        size_range = {1,2},
        tunnels_per_room_range = {1,2}
    }
end

function M.make_rectangle_criteria()
        return SourceMap.rectangle_criteria { 
                fill_selector = { matches_all = SourceMap.FLAG_SOLID, matches_none = SourceMap.FLAG_PERIMETER }, 
                perimeter_width = 1, 
                perimeter_selector = { matches_all = SourceMap.FLAG_SOLID }
        }
end

function M.make_rectangle_oper(floor, wall, wall_seethrough, --[[Optional]] area_query)
    local wall_flags = {SourceMap.FLAG_SOLID}
    local remove_wall_flags = {SourceMap.FLAG_SEETHROUGH}
    if wall_seethrough then
        append(wall_flags, SourceMap.FLAG_SEETHROUGH)
        remove_wall_flags = {}
    end
    return SourceMap.rectangle_operator { 
        area_query = area_query,
        perimeter_width = 1,
       fill_operator = { add = {SourceMap.FLAG_CUSTOM5, SourceMap.FLAG_SEETHROUGH}, remove = {SourceMap.FLAG_SOLID}, content = floor},
        perimeter_operator = { add = {SourceMap.FLAG_PERIMETER}, remove = remove_wall_flags, content = wall },
    }
end



return M
