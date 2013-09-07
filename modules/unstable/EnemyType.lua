-- Usage: 'EnemyType.define { ... attributes ... }', 'EnemyType.lookup(<name or ID>)'

local Stats = import "@Stats"
local Item = import "@Item"
local Schemas = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local EnemyType = ResourceTypes.type_create(
    Schemas.enforce_function_create {
        name = Schemas.STRING,
        description = Schemas.STRING,
        -- TODO sprites, radius, appear message, defeat message
        traits = Schemas.TABLE,
        on_prerequisite = Schemas.FUNCTION_OR_NIL
    }
)

return EnemyType