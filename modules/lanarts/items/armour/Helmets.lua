Data.equipment_create {
    name = "Iron Helmet",
    description = "A basic, durable iron helmet.",
    type = "helmet",

    shop_cost = { 15, 25 },

    spr_item = "iron helmet",

    spell_cooldown_multiplier = 1.05,

    reduction = { base = 1 }
}

Data.equipment_create {
    name = "Runed Helmet",
    description = "An enchanted helmet enscribed with ancient, magical text.",
    type = "helmet",

    shop_cost = { 45, 65 },

    spr_item = "runed helmet",

    spell_cooldown_multiplier = 1.05,

    reduction = { base = 1 },
    magic_reduction = { base = 1 },

    resistance = { base = 1 }
}

Data.equipment_create {
    name = "Wizard's Hat",
    description = "A basic, mildly magical, wizards hat.",
    type = "helmet",

    shop_cost = { 15, 25 },

    spr_item = "wizard hat",

    magic_reduction = { base = 1 }

}

Data.equipment_create {
    name = "Crown of Mars",
    description = "An artifact covered in runes of battle.",
    type = "helmet",

    shop_cost = { 45, 55 },

    spr_item = "mars crown",

    reduction = { base = 1 },
    stat_bonuses = { magic = 1, strength = 1 }
}

Data.equipment_create {
    name = "Diana's Diadem",
    description = "An artifact gifted by the god Diana, embued with defensive magic.",
    type = "helmet",

    shop_cost = { 45, 55 },

    spr_item = "dianas crown",

    reduction = { base = 1 },
    magic_reduction = { base = 1 },

    resistance = { base = 1 },
    magic_resistance = { base = 1 }
}

Data.equipment_create {
    name = "Crown of Orcus",
    description = "A runed crown of bone carved in the name of Orcus, granting life regeneration.",
    type = "helmet",

    shop_cost = { 45, 55 },

    spr_item = "orcus crown",

    reduction = { base = 1 },
    resistance = { base = 1 },
    stat_bonuses = { hpregen = 0.01666 }
}