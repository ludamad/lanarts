local M = nilprotect {} -- Submodule

local function resolve_multipliers(multipliers)
    if type(multipliers) == "string" then return {[multipliers] = 1} end
    if #multipliers == 0 then return multipliers end
    -- Else
    local ret = {}
    for type in values(multipliers) do ret[type] = 1 end
    return ret
end

function M.item_proficiency_type_create(multipliers)
   return resolve_multipliers(multipliers) 
end

function M.item_proficiency_requirement_create(multipliers, requirement)
    return {multipliers=multipliers, requirement=requirement}
end

function M.calculate_proficiency(p, stats)
    return table.dot_product(p, stats.derived.aptitudes.effectiveness)
end

function M.calculate_proficiency_modifier(proficiencies, stats)
    local failure_total, requirement_total = 0,0
    for p in values(proficiencies) do
        local prof = M.calculate_proficiency(p.multipliers, stats)
        print("Proficiency for " .. pretty_tostring(p,0,true) .. ": ".. prof)
        requirement_total = requirement_total + p.requirement
        failure_total = failure_total + math.max(0, p.requirement - prof)
    end
    return 1.0 - math.min(0.25, failure_total / requirement_total)
end

return M