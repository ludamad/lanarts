local M = nilprotect {} -- Submodule

M.SKILL_POINT_START_AMOUNT = 500
M.SKILL_POINT_COST_RATE = 100

M.EXPERIENCE_EXPONENT = 2.7
M.EXPERIENCE_COST_RATE = 75
M.EXPERIENCE_COST_BASE = 125

function M.level_experience_needed(xp_level)
    return math.round(xp_level ^ M.EXPERIENCE_EXPONENT * M.EXPERIENCE_COST_RATE) + M.EXPERIENCE_COST_BASE
end

local function skill_cost_increment(level)
    return level * M.SKILL_POINT_COST_RATE
end

-- Determines the amount of skill points received at each level-up
function M.skill_points_at_level_up(xp_level)
    return skill_cost_increment(xp_level+1)
end

local function skill_point_cost(multiplier, xp_level)
    return skill_cost_increment(math.floor(xp_level * (xp_level + 1) / 2 * multiplier))
end

function M.cost_from_skill_level(multiplier, xp_level)
    local f = math.floor(xp_level)
    local rem = xp_level - f
    local xp = (1-rem) * skill_point_cost(multiplier, f) + rem * skill_cost_increment(f+1);
    return math.ceil(xp)
end

function M.skill_level_from_cost(multiplier, xp)
    for lvl=0,math.huge do
        local xp_base = skill_point_cost(multiplier, lvl)
        local incr = skill_cost_increment(lvl + 1) 
        if xp_base + incr > xp then
           local true_level = lvl + (xp - xp_base) / incr
           return math.floor(true_level * 10) / 10
        end
    end
    assert(false)
end

return M