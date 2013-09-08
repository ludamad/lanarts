-- Usage: 'ItemType.define { ... attributes ... }', 'ItemType.lookup(<name or ID>)'
local S = import "@Schemas"
local ResourceTypes = import "@ResourceTypes"

local ItemType = ResourceTypes.type_create(
    S.enforce_function_create {
        name = S.STRING,
        description = S.STRING,
        sprite = S.NOT_NIL, -- Any 'drawable' type, eg has :draw(xy) method
        traits = S.TABLE, -- Determines if it is equipment, etc
        gold_worth = S.defaulted(S.NUMBER, 0), -- Determines shop cost and placement weight 

        on_prerequisite = S.FUNCTION_OR_NIL,
        on_use = S.FUNCTION,

        equip_bonuses = S.TABLE_OR_NIL,
        -- Takes an approximate character-level appropriateness
        on_variation = S.FUNCTION_OR_NIL,

        -- For equipment (all require item to be equippable and already equippeds
        on_step = S.FUNCTION_OR_NIL,
        on_evoke = S.FUNCTION_OR_NIL,
        on_wield = S.FUNCTION_OR_NIL,
        on_calculate = S.FUNCTION_OR_NIL
    }
)

ItemType.EQUIPMENT_TRAIT = "equipment_trait"

return ItemType