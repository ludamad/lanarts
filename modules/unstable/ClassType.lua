-- Usage: 'ClassType.define { ... attributes ... }', 'ClassType.lookup(<name or ID>)'
local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local class_schema = {
    name = S.NOT_NIL, description = S.NOT_NIL,
    traits = S.TABLE_OR_NIL,
    on_create = S.FUNCTION    
}

local function define_class(args)
    return S.enforce(class_schema, args)
end

local ClassType = ResourceTypes.type_create(define_class)
return ClassType