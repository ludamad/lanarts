Data.equipment_create {
    name = "Ring of Spells",
    description = "A ring bound with magical energies, giving the user increased mana reserves and making spell projectiles move faster.",
    type = "ring",

    shop_cost = { 35, 55 },

    spr_item = "ring of spells",

    stat_bonuses = { mp = 15, spell_velocity_multiplier = 1.2 }
}

Data.equipment_create {
    name = "Ogre's Ring",
    description = "A large ring that grants the user brutish force, at the cost of physical defences.",
    type = "ring",

    shop_cost = { 95, 125 },

    spr_item = "ogres ring",

    stat_bonuses = { defence = -1, strength = 2, hp = 10 }
}

Data.equipment_create {
    name = "Hydra Ring",
    description = "An enchanted ring of hydra skin; grants the user greater healing over time.",
    type = "ring",

    shop_cost = { 95, 125 },

    spr_item = "hydra ring",

    stat_bonuses = { hpregen = 0.01666 }
}

Data.equipment_create {
    name = "Moonstone Ring",
    description = "A ring said to be forged from celestial material; grants the user magical replenishment.",
    type = "ring",

    shop_cost = { 95, 125 },

    spr_item = "moonstone ring",

    stat_bonuses = { mpregen = 0.01666 }
}

Data.equipment_create {
    name = "Ring of Vitality",
    description = "A ring bound with life-extending mantras.",
    type = "ring",

    shop_cost = { 35, 55 },

    spr_item = "ring of vitality",

    stat_bonuses = { hp = 15 }
}

Data.equipment_create {
    name = "Ring of Stone",
    description = "A ring that causes thickening of the skin.",
    type = "ring",

    shop_cost = { 55, 75 },

    spr_item = "ring of stone",

    stat_bonuses = { defence = 2 }
}

Data.equipment_create {
    name = "Gallanthor's Ring",
    description = "The ring of an old archmage.",
    type = "ring",

    shop_cost = { 115, 145 },

    spell_cooldown_multiplier = 0.9,
    spr_item = "gallanthors ring",

    stat_bonuses = { magic = 1, willpower = -1, mp = 10 }
}

Data.equipment_create {
    name = "Magician's Ring",
    description = "A glowing ring that grants the user magical power, at the cost of magical defences.",
    type = "ring",

    shop_cost = { 95, 125 },

    spr_item = "magicians ring",

    stat_bonuses = { magic = 2, willpower = -1, mp = 10}
}

-- Start the game with 1000 'randarts' -- for now, preconfigured item generations.
require("items.Randarts").define_randarts()
