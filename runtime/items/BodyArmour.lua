Data.equipment_create {
    name = "Leather Armour",
    description = "A durable, flexible armour.",
    type = "armour",

    shop_cost = {15,25},

    spr_item = "leather armour",
    randart_weight = 100,


    stat_bonuses = { 
        spell_cooldown_multiplier = 1.1,
        defence = 2,
    },
}

Data.equipment_create {
    name = "Chainmail",
    description = "A strong armour with limited flexibility.",
    type = "armour",

    shop_cost = {45,60},
    randart_weight = 20,

    spr_item = "spr_armour.chain_mail1",

    stat_bonuses = { 
        defence = 3,
        spell_cooldown_multiplier = 1.15,
        ranged_cooldown_multiplier = 1.15,
    },
}

Data.equipment_create {
    name = "Crystalline Plate",
    description = "An armour made of heavy but magically unburdensome crystals. Slows down movement speed considerably.",
    type = "armour",

    shop_cost = {295,305},

    spr_item = "spr_armour.crystal_plate2",

    stat_bonuses = { 
        defence = 3,
        willpower = 3,
    },
    effects_granted = {"Encumbered"}
}

Data.equipment_create {
    name = "Platemail",
    description = "A strong, heavy armour.",
    type = "armour",

    shop_cost = {95,105},

    spr_item = "platemail",

    stat_bonuses = { 
        defence = 3,
        spell_cooldown_multiplier = 1.35,
        ranged_cooldown_multiplier = 1.35,
    },
}

Data.equipment_create {
    name = "Green Robe",
    description = "The magic robes of a beginner green mage.",
    type = "armour",

    shop_cost = {10,20},

    spr_item = "spr_armour.green_robe",

    stat_bonuses = { 
        willpower = 1,
    },
    effects_granted = {
        {"RedResist", {resist = -1}},
        {"RedPower", {power = -1}},
        {"GreenResist", {resist = 1}},
        {"GreenPower", {power = 1}},
    }
}

Data.equipment_create {
     name = "Robe",
     description = "A lightly protective, uninhibited robe.",
     type = "armour",
 
     shop_cost = {10,20},
 
     spr_item = "robe",
 
     stat_bonuses = { 
         willpower = 1,
     },
}

Data.equipment_create {
    name = "White Robe",
    description = "The magic robes of a beginner white mage.",
    type = "armour",

    shop_cost = {40,100},

    spr_item = "spr_armour.white_robe",

    stat_bonuses = { 
        willpower = 1,
    },
    effects_granted = {
        {"BlackResist", {resist = -4}},
        {"BlackPower", {power= -4}},
        {"WhiteResist", {resist = 4}},
        {"WhitePower", {power = 4}},
    }
}

Data.equipment_create {
    name = "Red Robe",
    description = "The magic robes of a beginner red mage.",
    type = "armour",

    shop_cost = {40,100},

    spr_item = "spr_armour.robe_ego1",

    stat_bonuses = { 
        willpower = 1,
    },
    effects_granted = {
        {"BlueResist", {resist = -4}},
        {"BluePower", {power = -4}},
        {"RedResist", {resist = 4}},
        {"RedPower", {power = 4}},
    }
}


Data.equipment_create {
    name = "Magician's Robe",
    description = "An enchanted robe of protection and magical power.",
    type = "armour",

    shop_cost = {75,90},

    spr_item = "enchanted robe",

    stat_bonuses = {
        magic = 1,
        willpower = 1,
        defence = 1
    }
}

Data.equipment_create {
    name = "Runed Robe",
    description = "A heavily enchanted robe of protection and magical power.",
    type = "armour",

    shop_cost = {115,135},

    spr_item = "runed robe",

    stat_bonuses = {
        defence = 2, 
        willpower = 2, 
        magic = 1,
    }
}

Data.equipment_create {
    name = "Robe of Mana",
    description = "A robe of great magical capacity.",
    type = "armour",

    shop_cost = {95,155},

    spr_item = "robe of mana",

    stat_bonuses = {
        mp = 35, 
        willpower = 2, 
    }
}

Data.equipment_create {
    name = "Robe of Vitality",
    description = "A robe that magically grants prolonged life to the user.",
    type = "armour",

    shop_cost = {125,155},

    spr_item = "spr_armour.robe_of_health",

    stat_bonuses = {
        hp = 35, 
        willpower = 2, 
    }
}

Data.equipment_create {
    name = "Red Dragonplate",
    description = "A great armour crafted from dragon scales. Grants mana on every kill.",
    type = "armour",

    shop_cost = {395,605},

    spr_item = "spr_armour.red_dragon_scale_mail",
    stat_bonuses = {
        hp = 10,
        mp = 10,
        defence = 3,
        willpower = 3,
        spell_cooldown_multiplier = 1.05,
        ranged_cooldown_multiplier = 1.05,
    },
    effects_granted = {"ArmourManaGainOnKill"}
}
