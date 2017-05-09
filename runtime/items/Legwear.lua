Data.equipment_create {
    name = "Platelegs",
    description = "Protecting legwear that interferes with ranged and magical combat.",
    type = "legwear",

    shop_cost = { 50, 70 },

    spr_item = "spr_legwear.platelegs",

    spell_cooldown_multiplier = 1.35,

    ranged_cooldown_multiplier = 1.35,

    stat_bonuses = { 
        defence = 2,
    },
}

Data.equipment_create {
    name = "Magic Skirt",
    description = "A magical skirt. Grants improved magic ability.",
    type = "legwear",

    shop_cost = { 50, 70 },

    spr_item = "spr_legwear.skirt",
    stat_bonuses = {magic = 1},
}

Data.equipment_create {
    name = "Gallanthor's Skirt",
    description = "A magical skirt. Grants improved magic ability and the ability to cast spells faster.",
    type = "legwear",

    shop_cost = { 150, 300 },

    spr_item = "spr_legwear.gallanthorskirt",
    stat_bonuses = {magic = 1},
    spell_cooldown_multiplier = 0.91
}

