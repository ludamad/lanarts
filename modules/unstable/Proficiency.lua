local type, math, table = type, math, table -- Cache
local StatMultiplierUtils = import "@StatMultiplierUtils"

local M = nilprotect {} -- Submodule

function M.proficiency_type_create(multiplier)
   return StatMultiplierUtils.resolve_multiplier(multiplier) 
end

function M.proficiency_requirement_create(multipliers, requirement)
    if type(multipliers) ~= "table" or #multipliers > 0 then
        multipliers = M.proficiency_type_create(multipliers)
    end
    return {multipliers=multipliers, requirement=requirement}
end

function M.calculate_proficiency(p, stats)
    return table.dot_product(p, stats.derived.aptitudes.effectiveness)
end

function M.resolve_proficiency_requirements(proficiencies, stats)
    local failure_total, requirement_total = 0,0
    for i=1,#proficiencies do
        local p = proficiencies[i]
        local prof = M.calculate_proficiency(p.multipliers, stats)
        requirement_total = requirement_total + p.requirement
        failure_total = failure_total + math.max(0, p.requirement - prof)
    end
    return failure_total, requirement_total
end

return M