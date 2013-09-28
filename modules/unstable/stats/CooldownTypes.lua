local StatContext = import "@StatContext"

local M = nilprotect {} -- Submodule

-- Default cooldown amounts
M.default_cooldown_table = {
    ALL_ACTIONS = 45,
    OFFENSIVE_ACTIONS = 45,
    MELEE_ACTIONS = 45,
    SPELL_ACTIONS = 45,
    ITEM_ACTIONS = 45,
    ABILITY_ACTIONS = 45,
    REST_ACTION = 20
}

-- Cooldown types
for type,amount in pairs(M.default_cooldown_table) do
    M[type] = type
end

function M.cooldown_needed_message(type)
    if type == M.ITEM_ACTIONS then
        return "You cannot use an item yet."
    elseif type == M.MELEE_ACTIONS or type == M.OFFENSIVE_ACTIONS or type == M.SPELL_ACTIONS then
        return "You cannot use an attack yet."
    end
end

M.parent_cooldown_types = {
    [M.ALL_ACTIONS] = {},
    [M.OFFENSIVE_ACTIONS] = {M.ALL_ACTIONS},
    [M.MELEE_ACTIONS] = {M.ALL_ACTIONS, M.OFFENSIVE_ACTIONS},
    [M.SPELL_ACTIONS] = {M.ALL_ACTIONS, M.OFFENSIVE_ACTIONS},
    [M.ITEM_ACTIONS] = {M.ALL_ACTIONS},
    [M.ABILITY_ACTIONS] = {M.ALL_ACTIONS},
    [M.REST_ACTION] = {M.ALL_ACTIONS}
}

-- The following make cooldowns easier to define:
M.cooldown_to_field_map = {
    [M.ALL_ACTIONS] = "cooldown",
    [M.OFFENSIVE_ACTIONS] = "cooldown_offensive",
    [M.MELEE_ACTIONS] = "cooldown_melee",
    [M.SPELL_ACTIONS] = "cooldown_spell",
    [M.ITEM_ACTIONS] = "cooldown_item",
    [M.ABILITY_ACTIONS] = "cooldown_ability",
}

M.field_to_cooldown_map = table.value_key_invert(M.cooldown_to_field_map)

M.cooldown_fields = table.key_list(M.field_to_cooldown_map)
table.sort(M.cooldown_fields)

function M.cooldown_table(types, --[[Optional]] scale)
    scale = scale or 1
    local ret = {}
    for type in values(types) do
        ret[type] = M.default_cooldown_table[type] * scale
    end
    return ret
end

-- Function for applying to all cooldowns
function M.multiply_all_cooldown_rates(stats, rate, --[[Optional]] permanent)
    for type,_cooldown in pairs(M.default_cooldown_table) do
        StatContext.multiply_cooldown_rate(stats, type, rate, permanent)
    end
end

function M.reset_rest_cooldown(stats)
    local cooldown = M.default_cooldown_table[M.REST_ACTION]
    StatContext.apply_cooldown(stats, M.REST_ACTION, cooldown)
end

return M