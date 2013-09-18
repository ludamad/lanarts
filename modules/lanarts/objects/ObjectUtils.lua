local Display = import "core.Display"

local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"

local M = nilprotect {} -- Submodule

function M.draw_if_seen(obj, sprite, --[[Optional]] alpha)
    if GameMap.object_visible(obj) then 
        M.screen_draw(sprite, obj.xy, alpha)
    end
end

-- Draw's obj.sprite. Useful on_draw method for static-sprite objects.
function M.draw_sprite_member_if_seen(obj)
    M.draw_if_seen(obj, obj.sprite)
end

function M.screen_draw(sprite, xy, --[[Optional]] alpha, --[[Optional]] frame)
    local color = alpha and with_alpha(COL_WHITE, alpha) or COL_WHITE
    sprite:draw(
        {origin=Display.CENTER, color=color, frame=frame},
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

function M.tile_xy(self)
    return vector_scale(self.xy, 1/32.0, true)
end

return M