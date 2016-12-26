Data.equipment_create {
    name = "Amulet of the Wall",
    description = "Grants the user the spell 'Wallanthor'.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.blue",
    spells_granted = {"Wallanthor"},

    --stat_bonuses = { mp = -25}
}

-- name = "Amulet of Festivity",
Data.equipment_create {
    name = "Amulet of Fear",
    description = "Grants the user the ability to instill the fear of death in enemies with a 'Fear Strike'.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.fear_strike",
    spells_granted = {"Fear Strike"},
}

Data.equipment_create {
    name = "Amulet of Trepidation",
    description = "Grants the user the spell 'Trepidize'.",
    type = "amulet",

    shop_cost = { 120, 140},

    spr_item = "spr_amulets.skull",
    spells_granted = {"Trepidize"},
}

Data.equipment_create {
    name = "Amulet of Mephitization",
    description = "Grants the user the spell 'Mephitize'.",
    type = "amulet",

    shop_cost = { 120, 140 },
    stat_bonuses = { hp = -25},

    spr_item = "spr_amulets.cabochon",
    spells_granted = {"Mephitize"},
}

Data.equipment_create {
    name = "Amulet of Regeneration",
    description = "Grants the user the spell 'Regeneration', but the user has -25HP.",
    type = "amulet",

    shop_cost = { 120, 140 },
    stat_bonuses = { hp = -25},

    spr_item = "spr_amulets.regeneration",
    spells_granted = {"Regeneration"},
}

