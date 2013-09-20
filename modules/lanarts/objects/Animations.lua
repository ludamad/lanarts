local GameObject = import "core.GameObject"
local GameMap = import "core.GameMap"

local ObjectUtils = import ".ObjectUtils"

local M = nilprotect {} -- Submodule

local ANIMATION_TRAIT = "animation"
local ANIMATION_DEPTH = -100

function M.fadeout_create(args)
    assert(args.duration and args.sprite)
    args.velocity = args.velocity or {0,0}
    args.time_elapsed = 0

    args.depth = args.depth or ANIMATION_DEPTH
    args.traits = {ANIMATION_TRAIT}

    function args:on_step()
        self.xy = vector_add(self.xy, self.velocity)
        self.time_elapsed = self.time_elapsed + 1
        if self.time_elapsed > self.duration then
            GameObject.destroy(self)
        end
    end

    function args:on_draw()
        self.sprite:draw(self.xy)
--    
--        local alpha = (self.duration - self.time_elapsed) / self.duration
--        ObjectUtils.draw_if_seen(self, self.sprite, alpha)
    end

    return GameObject.object_create(args)
end

function M.is_animation(obj)
    return table.contains(obj.traits, ANIMATION_TRAIT)
end

return M