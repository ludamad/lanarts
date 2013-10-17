local ActionProjectileObject = import "@objects.ActionProjectileObject"
local Actions = import "@Actions"
local GameMap = import "core.Map"
local Apts = import "@stats.AptitudeTypes"

local M = nilprotect {} -- Submodule

-- Returns the amount of summons, and the XP cap of a summon
function M.get_summoner_capacity(obj)
    local D = obj:stat_context().derived
    local summon_apt = D.aptitudes.effectiveness[Apts.SUMMONING]
    
end

function M.get_summoned_objects(obj)
    local ret = {}
    for o in GameMap.objects(obj.map) do
        if o.summoner == obj then
            table.insert(ret, o)
        end
    end
    return ret
end

function M.get_summoned_objects_worth(summoned_objects)
    local gain = 0
    for s in values(summoned_objects) do
        gain = gain + s.xp_gain
    end
    -- Far cheaper to have few, similarly powered 
    return math.sqrt(gain * (#summoned_objects) ^ 1.25)
end

M.SummonPrereq = newtype()
function M.SummonPrereq:init(args)
    self.xp_cap_func = args
end

M.SummonEffect = newtype()
function M.SummonEffect:init(args)
end
function M.SummonEffect:apply(user, target)
    local user_xy = user.obj.xy
    -- Assumption: Target is either a position or a StatContext
    local target_xy = is_position(target) and target or target.obj.xy

    return ActionProjectileObject.create {
        map = user.obj.map,
        xy = user_xy,
        stats = user.obj:stat_context_copy(),
        velocity = vnorm(vsub(target_xy, user_xy), self.speed),
        -- Projectile configuration:
        sprite = self.sprite,
        action = self.action,
        radius = self.radius
    }
end


return M