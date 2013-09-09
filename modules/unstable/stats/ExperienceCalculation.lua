local M = nilprotect {} -- Submodule

local function base(xp_level)
    return xp_level ^ 2.7
end

local function inverse(xp)
    return math.log(xp) / math.log(2.7)
end

function M.level_experience_needed(xp_level)
    return math.round(base(xp_level) * 75) + 125
end

-- Before adjustments
function M.skill_level_from_experience(multiplier, xp)
    xp = (xp / multiplier / 10) - 50
    return math.round(inverse(xp) * 10) / 10
end

-- Before adjustments
function M.experience_from_skill_level(multiplier, xp_level)
    return math.round(base(xp_level) * multiplier * 10) + 50
end

return M