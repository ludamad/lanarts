Data.equipment_create {
    name = "Leather Gloves",
    description = "Gloves, unmistakenably made from gnoll leather.",
    type = "gloves",

    shop_cost = { 15, 25 },

    spr_item = "leather gloves",

    stat_bonuses = { 
        defence = 1,
    },
}

Data.equipment_create {
    name = "Steel Gloves",
    description = "Gloves made from steel, slightly unwieldy.",
    type = "gloves",

    shop_cost = { 55, 75 },

    spr_item = "steel gloves",
    randart_sprites = {"spr_armour.randart_glove1", "spr_armour.randart_glove2", "spr_armour.randart_glove3", "spr_armour.randart_glove4"},
    stat_bonuses = { 
        defence = 2,
    },
}

Data.equipment_create {
    name = "Gloves of Confusion",
    description = "Grants 10% chance of confusing enemies on hit, sending them wandering.",
    type = "gloves",

    shop_cost = { 95, 125 },

    spr_item = "spr_armour.gloves_confusion",

    stat_bonuses = { 
        defence = 1,
    },
    --randart_sprites = {"spr_armour.gloves_confusion_randart"},
    effects_granted = {"ConfusingWeapon"}
}

Data.equipment_create {
    name = "Gloves of Fear",
    description = "Grants 10% chance of striking fear into enemies.",
    type = "gloves",

    shop_cost = { 95, 125 },

    stat_bonuses = { 
        defence = 1,
    },
    spr_item = "spr_armour.gloves_fear",
    --randart_sprites = {"spr_armour.gloves_fear_randart"},
    effects_granted = {"FearWeapon"}
}

Data.equipment_create {
    name = "Runed Gloves",
    description = "Gloves made of a unnaturally strong cloth material, covered in magic runes.",
    type = "gloves",

    shop_cost = { 95, 125 },

    spr_item = "runed gloves",

    stat_bonuses = { 
        strength = 1,
        defence = 2,
        willpower = 1,
    },
}
