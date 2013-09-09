local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local SkillType = ResourceTypes.type_create(
    S.enforce_function_create {
        name = S.STRING,
        description = S.STRING,
--        sprite = S.NOT_NIL, --TODO: stat drawing thingies

        on_step = S.FUNCTION_OR_NIL,
        on_attack = S.FUNCTION_OR_NIL,
        on_calculate = S.FUNCTION_OR_NIL,
        cost_multiplier = S.defaulted(S.NUMBER, 1)
    }
)

return SkillType