local ResourceTypes = import "@ResourceTypes"

local M = ResourceTypes.type_create() -- Submodule

-- Create a skill table with the given values. Anything unset is assumed to be 0.
function M.skills_create(--[[Optional]] params)
    local skills = {}
    if params ~= nil then
        table.deep_copy(params, skills)
    end
    return skills
end

return M