Data.equipment_create {
    name = "Platelegs",
    description = "Protecting legwear that interferes with ranged and magical combat.",
    type = "legwear",

    shop_cost = { 50, 70 },

    spr_item = "spr_legwear.platelegs",

    spell_cooldown_multiplier = 1.35,

    ranged_cooldown_multiplier = 1.35,

    reduction = {base = 1},
    resistance = {base = 1}
}

Data.equipment_create {
    name = "Magic Skirt",
    description = "A magically protected skirt. Impedes physical combat.",
    type = "legwear",

    shop_cost = { 50, 70 },

    spr_item = "spr_legwear.skirt",

    melee_cooldown_multiplier = 1.35,

    magic_reduction = {base = 1},
    magic_resistance = {base = 1}
}

