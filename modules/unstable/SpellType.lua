-- Usage: 'SpellType.define { ... attributes ... }', 'SpellType.lookup(<name or ID>)'
local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local M = nilprotect {} -- Submodule

-- Useful for monster AI and auto-targetting.
-- The on_autotarget method provides better functionality.
M.TARGET_NONE = "target_none"

M.TARGET_HOSTILE_POSITION = "target_hostile_position"
M.TARGET_FRIENDLY_POSITION = "target_friendly_position"

M.TARGET_HOSTILE = "target_hostile"
M.TARGET_FRIENDLY = "target_friendly"

local TARGET_TYPES = {
    M.TARGET_NONE, M.TARGET_HOSTILE_POSITION, M.TARGET_FRIENDLY_POSITION,
    M.TARGET_HOSTILE, M.TARGET_FRIENDLY
}

local DEFAULT_GLOBAL_COOLDOWN = 40
local DEFAULT_RANGE = 300

local schema = {
    lookup_key = S.STRING,
    traits = S.TABLE,

    target_type = S.one_of(TARGET_TYPES, --[[default]] M.TARGET_HOSTILE_POSITION),
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