Data.equipment_create {
    name = "Belt of Protection",
    description = "A belt with a protective aura.",
    type = "belt",

    shop_cost = { 120, 140 },

    spr_item = "spr_belts.protection",

    reduction = { base = 1 },
    resistance = { base = 1 },
    magic_reduction = { base = 1 },
    magic_resistance = { base = 1 }
}

Data.equipment_create {
    name = "Belt of Slaying",
    description = "A belt infused with ogre strength.",
    type = "belt",

    shop_cost = { 120, 140 },

    spr_item = "spr_belts.slaying",

    stat_bonuses = {strength = 1}
}

Data.equipment_create {
    name = "Spiked Belt",
--    description = "Your can cast 'Call Spikes'.",
    description = "You gain +1 defence, and do 25% damage back to melee attackers.",
    type = "belt",

    shop_cost = { 200, 250 },

    spr_item = "spr_belts.spike",

    stat_bonuses = {defence = 1},
    effects_granted = {"Spiky"}
}

Data.equipment_create {
    name = "Dank Belt",
    description = "Your melee attacks have a 10% chance of poisoning targets.",
    type = "belt",

    shop_cost = { 200, 250 },

    spr_item = "spr_belts.poison",

    effects_granted = {"PoisonedWeapon"}
}

Data.equipment_create {
    name = "Lifesteal Belt",
    description = "A belt that grants extra health on every kill.",
    type = "belt",

    shop_cost = { 200, 250 },

    spr_item = "spr_belts.life-belt",
    effects_granted = {"HealthGainOnKill"}
}

Data.equipment_create {
    name = "Warped Belt",
    description = "A belt that decreases your HP and MP reserves significantly, but provides significant magical power.",
    type = "belt",

    shop_cost = { 200, 250 },

    spr_item = "spr_belts.warped",

    stat_bonuses = {magic = 4, mp = -40, hp = -50}
}

