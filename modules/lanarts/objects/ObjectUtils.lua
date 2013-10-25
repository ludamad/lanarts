local Display = import "core.Display"

local GameObject = import "core.GameObject"
local Map = import "core.Map"
local Relations = import ".Relations"

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

function M.type_create(--[[Optional]] base_type)
    local type = {}
    if base_type then table.copy(base_type, type) end
    type.base = base_type

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

function M.find_closest_hostile(self)
    perf.timing_begin("find_closest_hostile")
    local x,y = self.x,self.y
    local min_dist,min_obj = math.huge,nil
    for _, obj in ipairs(self.map.combat_objects) do
        if not obj.destroyed and Relations.is_hostile(self, obj) then
            local dist = math.max(math.abs(obj.x-x), math.abs(obj.y-y))
            if dist < min_dist then 
                min_dist,min_obj = dist,obj
            end
        end
    end
    perf.timing_end("find_closest_hostile")
    return min_obj
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

return M
