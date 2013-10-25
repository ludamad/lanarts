-- Usage: 'ClassType.define { ... attributes ... }', 'ClassType.lookup(<name or ID>)'
local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local class_schema = {
    lookup_key = S.NOT_NIL, 
    traits = S.TABLE_OR_NIL,
    on_spend_skill_points = S.FUNCTION, -- Takes (self, skill_points_to_spend)
    on_create = S.FUNCTION, -- Takes (configuration)
    on_map_init = S.FUNCTION, -- Takes (self, stats)
}

local function define_class(args)
    return S.enforce(class_schema, args)
end

local ClassType = ResourceTypes.type_create(define_class, nil, nil, --[[Lookup key]] "lookup_key")
return ClassType