local M = nilprotect {} -- Submodule

local function base(xp_level)
    return xp_level ^ 2.7
end

function M.level_experience_needed(xp_level)
    return math.round(base(xp_level) * 75) + 125
end

-- Determines the amount of skill points received at each level-up
function M.skill_points_at_level_up(xp_level)
    return (xp_level+1) * 100
end

local function skill_point_cost(multiplier, xp_level)
    return math.floor(xp_level * (xp_level + 1) / 2 * multiplier) * 100
--    return math.round(base(xp_level) * multiplier * 25) + xp_level* 100
end

function M.cost_from_skill_level(multiplier, xp_level)
    local f = math.floor(xp_level)
    local rem = xp_level - f
    local xp = (1-rem) * skill_point_cost(multiplier, f) + (rem) * skill_point_cost(multiplier, f+1)
    return math.ceil(xp)
end

function M.skill_level_from_cost(multiplier, xp)
    local lvl = 1
    local xp_last = 0
    while true do
        local xp_need = skill_point_cost(multiplier, lvl) 
        if xp_need > xp then
           local ratio = (xp - xp_last) / (xp_need - xp_last)
           local true_level = (1-ratio) * (lvl-1) + ratio * (lvl)
           return math.floor(true_level * 10) / 10
        end
        lvl, xp_last = lvl+1, xp_need
    end
    assert(false)
end

return M