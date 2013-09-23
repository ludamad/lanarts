-- Usage: 'SpellType.define { ... attributes ... }', 'SpellType.lookup(<name or ID>)'
local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"
local Actions = import "@Actions"

local M = nilprotect {} -- Submodule

local DEFAULT_GLOBAL_COOLDOWN = 40
local DEFAULT_RANGE = 300

local schema = {
    lookup_key = S.STRING,
    traits = S.TABLE,

    target_type = S.one_of(Actions.TARGET_TYPES, --[[default]] Actions.TARGET_HOSTILE_POSITION),
    range = S.defaulted(S.NUMBER, DEFAULT_RANGE),
    mp_cost = S.defaulted(S.NUMBER, 0),

	cooldown_self = S.defaulted(S.NUMBER, 0),
	cooldown_offensive = S.NUMBER,
	cooldown = S.NUMBER,
	cooldown_global = S.defaulted(S.NUMBER, DEFAULT_GLOBAL_COOLDOWN),

	recommended_stats = S.TABLE_OR_NIL,

    on_create = S.FUNCTION,
    on_use = S.FUNCTION,
    on_prerequisite = S.FUNCTION_OR_NIL,
    on_autotarget = S.FUNCTION_OR_NIL,

    ai_score_hint = S.FUNCTION_OR_NIL
}

local function create(t)
    t.lookup_key = t.lookup_key or t.name
    S.enforce(schema, t)
    return t
end

local SpellType = ResourceTypes.type_create(create, nil, nil, --[[Lookup key]] "lookup_key")

M.define = SpellType.define
M.lookup = SpellType.lookup

return M