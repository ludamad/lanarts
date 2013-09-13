local Display = import "core.Display"

local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"

local M = nilprotect {} -- Submodule

-- TODO: Make each GameObject take a metatable that operates like a class
-- Per-object diversification can still be provided via alternate per-object hooks.
-- on_step is fairly 'low-level' which is why it never worked well for enemy customization

local AVOID_SETTING = {xy=1, on_draw=1, on_step=1, radius=1, solid=1, do_init=1}

-- Create an object, copying over any unneeded arguments
-- TODO: Merge this functionality with GameObject.object_create so that it is no longer necessary
function M.object_create(args)
    local object = GameObject.object_create(args)
    for k,v in pairs(args) do
        -- Avoid setting anything we already used
        if not AVOID_SETTING[k] then
            object[k] = v
        end
    end
    return object
end

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

    function type._base_create(real_type, args)
        if base_type then 
            if base_type._create then
                return base_type._create(real_type, base_type.on_base_create, args)
            end
            args.type = args.type or real_type
            return base_type.create(args)
        else
            args.type = args.type or real_type
            local object = M.object_create(args)
            return object
        end
    end

    function type.create(...)
        return type._create(type, type._base_create, ...)
    end

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