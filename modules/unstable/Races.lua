-- Usage: 'Races.define { ... attributes ... }', 'Races.lookup(<name or ID>)'
local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local Races = ResourceTypes.type_create(
    S.enforce_function_create {
        name = S.STRING,
        description = S.STRING,
        traits = S.defaulted(S.TABLE, {}),
        on_create = S.FUNCTION
    }
)

return Races