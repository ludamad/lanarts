local Apts = import "@stats.AptitudeTypes"
local weapon_define = (import ".ItemDefineUtils").weapon_define

weapon_define {
    name = "Dagger",
    description = "A small but sharp blade, adept at stabbing purposes.",

    gold_worth = 15, difficulty = 0,
    aptitude_types = {Apts.MELEE, Apts.PIERCING, Apts.BLADE},
    effectiveness = 6, damage = 4, delay = 1.0
}

weapon_define {
    name = "Short Sword",
    description = "A small, light sword.",

    gold_worth = 35, difficulty = 1,
    aptitude_types = {Apts.MELEE, Apts.PIERCING, Apts.BLADE},
    effectiveness = 4, damage = 6, delay = 1.1
}

weapon_define {
    name = "Long Sword",
    description = "A large trusty sword.",

    gold_worth = 80, difficulty = 3,
    aptitude_types = {Apts.MELEE, Apts.SLASHING, Apts.BLADE},
    effectiveness = 1, damage = 10, delay = 1.4
}

weapon_define {
    name = "Great Sword",
    description = "An oversized brutish sword.",

    gold_worth = 120, difficulty = 5,
    aptitude_types = {Apts.MELEE, Apts.SLASHING, Apts.BLADE},
    effectiveness = -3, damage = 16, delay = 1.6
}

weapon_define {
    name = "Hand Axe",
    description = "A light, small and sturdy axe.",

    gold_worth = 20, difficulty = 0,
    aptitude_types = {Apts.MELEE, Apts.SLASHING, Apts.AXE},
    effectiveness = 3, damage = 7, delay = 1.3
}