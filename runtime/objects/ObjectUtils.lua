local Display = require "core.Display"

local GameObject = require "core.GameObject"
local Map = require "core.Map"

local M = nilprotect {} -- Submodule

function M.draw_if_seen(obj, sprite, --[[Optional]] alpha, --[[Optional]] frame, --[[Optional]] direction)
    if Map.object_visible(obj) then 
        M.screen_draw(sprite, obj.xy, alpha, frame, direction)
    end
end

-- Draw's obj.sprite. Useful on_draw method for static-sprite objects.
function M.draw_sprite_member_if_seen(obj)
    M.draw_if_seen(obj, obj.sprite, obj.alpha, obj.frame, obj.direction)
end

function M.screen_draw(sprite, xy, --[[Optional]] alpha, --[[Optional]] frame, --[[Optional]] direction, --[[Optiona]] color)
    local color = color or COL_WHITE
    color = alpha and with_alpha(color, alpha) or color
    sprite:draw(
        {origin=Display.CENTER, color=color, frame=frame, angle = direction},
        Display.to_screen_xy(xy)
    )
end

function M.type_create(type)
    type = type or {}
    local base = type.base
    if base then table.copy(type.base, type) end
    type.base = base

    function type.base_create(args)
        args.type = args.type or type
        if base_type then return base_type.create(args)
        else return GameObject.object_create(args) end
    end
    -- Default create
    type.create = type.base_crate

    return type
end

function M.object_callback(object, methodname, ...)
    local method = object[methodname]
    if method == nil and object.type ~= nil then
        method = object.type[methodname]
    end
    if method ~= nil then
        method(object, ...)
    end
end

function M.tile_xy(self)
    return vector_scale(self.xy, 1/32.0, true)
end

function M.find_free_position(self, dir)
    local dx,dy = unpack(dir)
    local check, new_xy, R = Map.radius_tile_check, {}, self.radius

    new_xy[1], new_xy[2] = self.x+dx, self.y+dy
    if not check(self.map, new_xy, R) then return new_xy end

    new_xy[1], new_xy[2] = self.x+dx, self.y
    if not check(self.map, new_xy, R) then return new_xy end

    new_xy[1], new_xy[2] = self.x, self.y+dy
    if not check(self.map, new_xy, R) then return new_xy end

    return nil
end


local function item_filter(obj) 
    return GameObject.get_type(obj) == 'item'
end

local function viable_item_square(obj, tx, ty, avoid_obj)
    if Map.tile_is_solid(obj.map, {tx, ty}) then
        return false
    end
    local x, y = tx * 32 + 16, ty * 32 + 16
    if avoid_obj and x == obj.x and y == obj.y then
        return false
    end
    local collisions = Map.rectangle_collision_check(obj.map, {x - 16, y - 16, x+16, y+16}, obj)
    collisions = table.filter(collisions, item_filter)
    if #collisions > 0 then
        return false
    end
    return true
end

local function spiral_iterate(f)
    local x, y = 0, 0
    local dx, dy = 0, -1
    while true do
        if f(x, y) then
            return
        end
        if x == y or (x < 0 and x == -y) or (x > 0 and x == 1-y) then
            dx, dy = -dy, dx
        end
        x, y = x+dx, y+dy
    end
end

local function try_spawn_item(obj, item, amount, tx, ty, avoid_obj)
    if not viable_item_square(obj, tx, ty, avoid_obj) then
        return false
    end
    GameObject.item_create {type = item, amount = amount, xy = {tx*32+16, ty*32+16}, do_init = true}
    return true
end

function M.spawn_item_near(obj, item, amount, --[[Optional]] avoid_obj, --[[Optional]] objX, --[[Optional]] objY)
    local x, y = math.floor((objX or obj.x) / 32), math.floor((objY or obj.y) / 32)
    spiral_iterate(function(dx, dy)
        return try_spawn_item(obj, item, amount, x + dx, y + dy, avoid_obj)
    end)
end

return M
