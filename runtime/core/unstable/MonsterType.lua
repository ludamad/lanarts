-- Usage: 'MonsterType.define { ... attributes ... }', 'MonsterType.lookup(<name or ID>)'
local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local monster_schema = {
    name = S.STRING,
    traits = S.TABLE,
    description = S.STRING,
    challenge_rating = S.NUMBER,
    base_stats = S.TABLE,
    unarmed_action = S.TABLE,
  
    -- Object controlling functions
    radius = S.defaulted(S.NUMBER, 14),
    init = S.FUNCTION_OR_NIL,
    on_step = S.FUNCTION_OR_NIL,
    on_draw = S.FUNCTION_OR_NIL,
    on_die = S.FUNCTION_OR_NIL,
    on_first_appear = S.FUNCTION_OR_NIL
}

local function define_monster(args)
    return S.enforce(monster_schema, args)
end

local MonsterType = ResourceTypes.type_create(define_monster)
return MonsterType