local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local SkillType = ResourceTypes.type_create(
    S.enforce_function_create {
        name = S.STRING,
        description = S.STRING,
--        sprite = S.NOT_NIL, --TODO: stat drawing thingies

        on_create = S.FUNCTION,
        on_step = S.FUNCTION_OR_NIL,
        on_attack = S.FUNCTION_OR_NIL,
        on_calculate = S.FUNCTION_OR_NIL,
        on_spend_skill_points = S.FUNCTION,
        cost_multiplier = S.defaulted(S.NUMBER, 1)
    }
)
-- Find a skill type from a list
function SkillType.get_skill_slot(list, type)
    local name = _G.type(type) == "string" and type or type.name
    for s in values(list) do 
        if s.type.name == name then
            return s
        end
    end
    assert(false) -- Skill type doesn't exist for some reason
end

return SkillType