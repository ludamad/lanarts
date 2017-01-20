Data.equipment_create {
    name = "Leather Armour",
    description = "A durable, flexible armour.",
    type = "armour",

    shop_cost = {15,25},

    spr_item = "leather armour",

    spell_cooldown_multiplier = 1.1,

    --reduction = {base = 1},
    resistance = {base = 2}
}

Data.equipment_create {
    name = "Chainmail",
    description = "A strong armour with limited flexibility.",
    type = "armour",

    shop_cost = {45,60},

    spr_item = "chainmail",

    spell_cooldown_multiplier = 1.15,

    ranged_cooldown_multiplier = 1.15,

    reduction = {base = 1},
    resistance = {base = 2}
}

Data.equipment_create {
    name = "Crystalline Plate",
    description = "An armour made of heavy but magically unburdensome crystals. Slows down movement speed considerably.",
    type = "armour",

    randart_sprite = {"spr_armour.crystal_plate3"},
    shop_cost = {295,305},

    spr_item = "spr_armour.crystal_plate2",

    reduction = {base = 3},
    resistance = {base = 3},
    magic_reduction = {base = 1},
    magic_resistance = {base = 1},
    effects_granted = {"Encumbered"}
}

Data.equipment_create {
    name = "Platemail",
    description = "A strong, heavy armour.",
    type = "armour",

    randart_sprite = {"spr_armour.randart_plate"},
    shop_cost = {95,105},

    spr_item = "platemail",

    spell_cooldown_multiplier = 1.35,

    ranged_cooldown_multiplier = 1.35,

    reduction = {base = 2},
    resistance = {base = 2}
}

Data.equipment_create {
    name = "Robe",
    description = "A lightly protective, uninhibited robe.",
    type = "armour",

    shop_cost = {10,20},
    randart_sprites = {"spr_armour.randart_robe1", "spr_armour.randart_robe2"},

    spr_item = "robe",

    --reduction = {base = 1},
    magic_reduction = {base = 1},

    --resistance = {base = 1},
    magic_resistance = {base = 1}
}

Data.equipment_create {
    name = "Magician's Robe",
    description = "An enchanted robe of protection and magical power.",
    type = "armour",

    shop_cost = {75,90},

    spr_item = "enchanted robe",

    reduction = {base = 1},
    magic_reduction = {base = 1},

    resistance = {base = 1},
    magic_resistance = {base = 1},

    stat_bonuses = {magic = 1}
}

Data.equipment_create {
    name = "Runed Robe",
    description = "A heavily enchanted robe of protection and magical power.",
    type = "armour",

    shop_cost = {115,135},

    spr_item = "runed robe",

    reduction = {base = 1},
    magic_reduction = {base = 2},

    resistance = {base = 1},
    magic_resistance = {base = 2},

    stat_bonuses = {defence = 1, magic = 1}
}

Data.equipment_create {
    name = "Robe of Mana",
    description = "A robe of great magical capacity.",
    type = "armour",

    shop_cost = {95,105},

    spr_item = "robe of mana",

    stat_bonuses = {mp = 35}
}

Data.equipment_create {
    name = "Robe of Vitality",
    description = "A robe that magically grants prolonged life to the user.",
    type = "armour",

    shop_cost = {125,155},

    spr_item = "spr_armour.robe_of_health",

    stat_bonuses = {hp = 35}
}

Data.equipment_create {
    name = "Red Dragonplate",
    description = "A great armour crafted from dragon scales. Grants mana on every kill.",
    type = "armour",

    shop_cost = {395,605},

    spr_item = "spr_armour.red_dragon_scale_mail",

    spell_cooldown_multiplier = 1.05,
    ranged_cooldown_multiplier = 1.05,

    stat_bonuses = {mp = 10, hp = 10},
    reduction = {base = 2},
    resistance = {base = 2},
    effects_granted = {"ManaGainOnKill"}
}
