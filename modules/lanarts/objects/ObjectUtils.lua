local Display = import "core.Display"

local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"
local Relations = import ".Relations"

local M = nilprotect {} -- Submodule

function M.draw_if_seen(obj, sprite, --[[Optional]] alpha, --[[Optional]] frame, --[[Optional]] direction)
    if GameMap.object_visible(obj) then 
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
    local min_dist,min_obj = math.huge,nil
    for obj in GameMap.objects(self.map) do
        if obj.team and Relations.is_hostile(self, obj) then
            local dist = vector_distance(obj.xy, self.xy)
            if dist < min_dist then 
                min_dist,min_obj = dist,obj
            end
        end
    end
    return min_obj
end

function M.tile_xy(self)
    return vector_scale(self.xy, 1/32.0, true)
end

return M