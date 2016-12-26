Data.equipment_create {
    name = "Leather Boots",
    description = "Sturdy leather boots that provide feet with basic, but much needed, protection.",
    type = "boots",

    shop_cost = { 15, 25 },

    spr_item = "leather boots",
    randart_sprites = {"spr_armour.randart_leather_boots"},

    resistance = { base = 1 }
}

Data.equipment_create {
    name = "Iron Boots",
    description = "Heavy boots suitable for war.",
    type = "boots",

    shop_cost = { 35, 45 },

    spr_item = "iron boots",

    reduction = { base = 1 },
    resistance = { base = 2 }
}

Data.equipment_create {
    name = "Troll Boots",
    description = "Heavy boots suitable for war.",
    type = "boots",

    shop_cost = { 35, 45 },

    spr_item = "troll boots",

    reduction = { base = 1 },
    resistance = { base = 2 },
    stat_bonuses = { movespeed = -2 }
}
