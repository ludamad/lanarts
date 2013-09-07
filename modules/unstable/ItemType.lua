-- Usage: 'ItemType.define { ... attributes ... }', 'ItemType.lookup(<name or ID>)'
local Schemas = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local ItemType = ResourceTypes.type_create(
    Schemas.enforce_function_create {
        name = Schemas.STRING,
        description = Schemas.STRING,
        traits = Schemas.TABLE,
        on_prerequisite = Schemas.FUNCTION_OR_NIL,
        on_use = Schemas.FUNCTION,
        equip_bonuses = Schemas.TABLE_OR_NIL
    }
)

return ItemType