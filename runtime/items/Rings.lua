Data.equipment_create {
    name = "Ring of Spells",
    description = "A ring bound with magical energies, giving the user increased mana reserves and making spell projectiles move faster.",
    type = "ring",

    shop_cost = { 35, 55 },

    spr_item = "ring of spells",

    stat_bonuses = { mp = 15, spell_velocity_multiplier = 1.2 }
}

Data.equipment_create {
    name = "Ring of Ethereal Armour",
    description = "You may only have this ring equipped, of all your rings. You may equip an additional piece of body armour.",
    type = "ring",

    shop_cost = { 225, 405 },

    spr_item = "spr_rings.ethereal",
}

Data.equipment_create {
    name = "Wizard's Ring",
    description = "A ring that gives the user +1 magic, and enables the user to regain mana on every kill.",
    type = "ring",

    shop_cost = { 155, 205 },

    spr_item = "spr_rings.wizardsring",
    effects_granted = {"RingManaGainOnKill"},
    stat_bonuses = {magic = 1}
}

Data.equipment_create {
    name = "Ring of Vampirism",
    description = "A ring that passively sucks the life of those you physically strike.",
    type = "ring",

    shop_cost = { 95, 125 },

    spr_item = "spr_rings.vampirism",

    effects_granted = {"VampiricWeapon"}
}

Data.equipment_create {
    name = "Abolishment Ring",
    description = "Abolishes the user's magic, granting them 1 strength point for every 3 points of magic.",
    type = "ring",

    shop_cost = { 155, 205 },

    spr_item = "spr_rings.abolishment",
    effects_granted = {"Abolishment"}
}

Data.equipment_create {
    name = "Ogre's Ring",
    description = "A large ring that grants the user brutish force, at the cost of physical defences.",
    type = "ring",

    shop_cost = { 95, 125 },

    spr_item = "ogres ring",

    stat_bonuses = { defence = -1, strength = 2}
}

Data.equipment_create {
    name = "Hydra Ring",
    description = "An enchanted ring of hydra skin; grants the user greater healing over time.",
    type = "ring",

    shop_cost = { 95, 125 },
    stat_bonuses = { hpregen = 0.0333333 },
    spr_item = "hydra ring",
}

Data.equipment_create {
    name = "Moonstone Ring",
    description = "A ring said to be forged from celestial material; grants the user magical replenishment.",
    type = "ring",

    shop_cost = { 95, 125 },

    stat_bonuses = { mpregen = 0.0333333 },
    spr_item = "moonstone ring",
}

Data.equipment_create {
    name = "Ring of Vitality",
    description = "A ring bound with life-extending mantras.",
    type = "ring",

    shop_cost = { 35, 55 },

    spr_item = "ring of vitality",

    stat_bonuses = { hp = 20 }
}

Data.equipment_create {
    name = "Ring of Stone",
    description = "A ring that causes thickening of the skin but reduced life.",
    type = "ring",

    shop_cost = { 55, 75 },

    spr_item = "ring of stone",

    stat_bonuses = { defence = 2, hp = -10 }
}

Data.equipment_create {
    name = "Gallanthor's Ring",
    description = "The ring of an old archmage.",
    type = "ring",

    shop_cost = { 115, 145 },

    spell_cooldown_multiplier = 0.9,
    spr_item = "gallanthors ring",

    stat_bonuses = {magic = 1, willpower = -1, mp = 10 }
}

Data.equipment_create {
    name = "Stara's Ring",
    description = "Grants the user +35MP, +2 willpower, -2 defence.",
    type = "ring",

    shop_cost = { 225, 325 },

    spr_item = "magicians ring",

    stat_bonuses = { defence = -2, willpower = 3, mp = 35}
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
require("items.Randarts").define_equipment_randarts()

