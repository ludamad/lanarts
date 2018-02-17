Data.equipment_create {
    name = "Leather Boots",
    description = "Sturdy leather boots that provide feet with basic, but much needed, protection.",
    type = "boots",

    shop_cost = { 15, 25 },

    spr_item = "spr_boots.boots1_brown",

    stat_bonuses = {
        defence = 1,
    },
}

Data.equipment_create {
    name = "Iron Shoes",
    description = "Heavy shoes suitable for war.",
    type = "boots",

    shop_cost = { 35, 45 },

    spr_item = "iron boots",

    stat_bonuses = {
        defence = 2,
    },
}

--Data.equipment_create {
--    name = "Iron Boots",
--    description = "Heavy boots suitable for war.",
--    type = "boots",
--
--    shop_cost = { 60, 90 },
--
--    spr_item = "spr_boots.boots_iron2",
--
--    stat_bonuses = {
--        willpower = 2,
--        defence = 3
--    }
--}

Data.equipment_create {
    name = "Troll Boots",
    description = "Heavy boots that enchant the user which regenerative properties and extra strength, but significantly reduce movement speed.",
    type = "boots",

    shop_cost = { 120, 200 },

    spr_item = "spr_boots.boots4_green",

    stat_bonuses = {
        hpregen = 2 / 60,
        strength = 1,
        defence = 3,
    },
    effects_granted = {"Encumbered"}
}

Data.equipment_create {
    name = "Jack Boots",
    description = "Beast skin boots that increases the user's lifeforce.",
    type = "boots",

    shop_cost = { 120, 200 },

    spr_item = "spr_boots.boots2_jackboots",

    stat_bonuses = {
        hp = 30,
        defence = 2,
    },
}
Data.equipment_create {
    name = "Stripe Boots",
    description = "Slick, sturdy boots that grant the user significantly faster spell volleys.",
    type = "boots",

    shop_cost = { 120, 200 },

    spr_item = "spr_boots.boots3_stripe",

    stat_bonuses = {
        --defence = 2,
        spell_velocity_multiplier = 1.50
    }
}

