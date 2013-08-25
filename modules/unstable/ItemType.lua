-- Usage: 'ItemType.define { ... attributes ... }', 'ItemType.lookup(<name or ID>)'
local Schemas = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local ITEM_TYPE_SCHEMA = {
    name = Schemas.STRING,
    traits = Schemas.TABLE,
    on_prerequisite = Schemas.FUNCTION_OR_NIL
}

return ResourceTypes.type_create(Schemas.checker_create(ITEM_TYPE_SCHEMA))