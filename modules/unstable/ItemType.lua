-- Usage: 'ItemType.define { ... attributes ... }', 'ItemType.lookup(<name or ID>)'
local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"
local StatContext

local function default_on_calculate(self, user)
    StatContext = StatContext or import "@StatContext" -- Lazy import
    if self.equip_bonuses then
        StatContext.temporary_add(user, self.equip_bonuses or {})
    end
    if self.base_equip_bonuses then
        StatContext.temporary_add(user, self.base_equip_bonuses)
    end
end

local schema = {
    sprite = S.NOT_NIL, -- Any 'drawable' type, eg has :draw(xy) method
    traits = S.TABLE, -- Determines if it is equipment, etc
    gold_worth = S.defaulted(S.NOT_NIL, 0), -- Determines shop cost and placement weight
    stackable = S.NOT_NIL, -- Is it stackable ?

    base_equip_bonuses = S.TABLE_OR_NIL,
    equip_bonuses = S.NIL, -- Dont accidentally set
    -- Takes an approximate character-level appropriateness
    on_variation = S.FUNCTION_OR_NIL,

    on_init = S.FUNCTION_OR_NIL,
    -- For equipment (all require item to be equippable and already equipped)
    on_step = S.FUNCTION_OR_NIL,
    on_evoke = S.FUNCTION_OR_NIL,
    action_use = S.TABLE,
    action_wield = S.TABLE_OR_NIL,
    on_calculate = S.defaulted(S.FUNCTION, default_on_calculate)
}

local function create(t)
    t.lookup_key = t.lookup_key or t.name
    S.enforce(schema, t)
    return t
end

local ItemType = ResourceTypes.type_create(create, nil, nil, --[[Lookup key]] "lookup_key")

ItemType.default_on_calculate = default_on_calculate
ItemType.EQUIPMENT_TRAIT = "equipment_trait"

return ItemType