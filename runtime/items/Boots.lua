Data.equipment_create {
    name = "Leather Boots",
    description = "Sturdy leather boots that provide feet with basic, but much needed, protection.",
    type = "boots",

    shop_cost = { 15, 25 },

    spr_item = "spr_boots.boots1_brown",
    randart_sprites = {"spr_boots.randart_leather_boots"},

    resistance = { base = 1 }
}

Data.equipment_create {
    name = "Iron Shoes",
    description = "Heavy shoes suitable for war.",
    type = "boots",

    shop_cost = { 35, 45 },

    spr_item = "iron boots",

    resistance = { base = 2 }
}

Data.equipment_create {
    name = "Iron Boots",
    description = "Heavy boots suitable for war.",
    type = "boots",

    shop_cost = { 60, 90 },

    spr_item = "spr_boots.boots_iron2",

    reduction = { base = 1 },
    resistance = { base = 1 }
}

Data.equipment_create {
    name = "Troll Boots",
    description = "Heavy boots that enchant the user which regenerative properties and extra strength, but significantly reduce movement speed.",
    type = "boots",

    shop_cost = { 120, 200 },

    spr_item = "spr_boots.boots4_green",

    reduction = { base = 1 },
    resistance = { base = 1 },
    stat_bonuses = { hpregen = 2 / 60, strength = 1},
    effects_granted = {"Encumbered"}
}

Data.equipment_create {
    name = "Jack Boots",
    description = "Beast skin boots that increases the user's lifeforce.",
    type = "boots",

    shop_cost = { 120, 200 },

    spr_item = "spr_boots.boots2_jackboots",

    reduction = { base = 1 },
    resistance = { base = 1 },
    stat_bonuses = { hp = 30 }
}
Data.equipment_create {
    name = "Stripe Boots",
    description = "Slick, sturdy boots that grant the user significantly faster spell volleys.",
    type = "boots",

    shop_cost = { 120, 200 },

    spr_item = "spr_boots.boots3_stripe",

    reduction = { base = 1 },
    resistance = { base = 1 },
    stat_bonuses = { spell_velocity_multiplier = 1.35 }
}
