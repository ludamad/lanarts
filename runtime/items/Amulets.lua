Data.equipment_create {
    name = "Amulet of the Wall",
    description = "An amulet that enables the user to create walls temporarily.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.blue",
    spells_granted = {"Wallanthor"},

    stat_bonuses = { defence = 2 }
}

-- Data.equipment_create {
--     name = "Amulet of Protection",
--     description = "Grants the user increased defences against magical attacks.",
--     type = "amulet",
-- 
--     shop_cost = { 180, 240 },
-- 
--     stat_bonuses = { willpower = 4 },
--     spr_item = "spr_amulets.protect",
-- }

Data.equipment_create {
    name = "Amulet of Fortification",
    description = "The wearer becomes sturdier when hit in melee.",
    type = "amulet",

    shop_cost = { 220, 440 },

    effects_granted = {"Fortification"},
    spr_item = "spr_amulets.protect",
}

Data.equipment_create {
    name = "Amulet of Greed",
    description = "Grants the user the ability to wear two additional amulets.",
    type = "amulet",

    shop_cost = { 220, 440 },

    spr_item = "spr_amulets.greed",
    -- Implemented in C++
}


-- name = "Amulet of Festivity",
Data.equipment_create {
    name = "Amulet of Fear",
    description = "A fearsome symbol.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.fear_strike",
    --spells_granted = {"Fear Strike"},
    effects_granted = {"FearWeapon"},
    stat_bonuses = {strength = 1}
}

Data.equipment_create {
    name = "Amulet of Healing",
    description = "Heals HP every time you kill an enemy.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.healing",
    effects_granted = {"AmuletHealthGainOnKill"},
}


Data.equipment_create {
    name = "Amulet of Trepidation",
    description = "A terrible amulet that causes permanent goosebumps.",
    type = "amulet",

    shop_cost = { 120, 140},

    spr_item = "spr_amulets.skull",
    spells_granted = {"Trepidize"},
    stat_bonuses = {magic = 1}
}

Data.equipment_create {
    name = "Amulet of Mephitization",
    description = "A noxious amulet of great power.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.cabochon",
    spells_granted = {"Mephitize"},
    effects_granted = {
        {"PoisonedWeapon", {poison_percentage = 0.10}}
    }
}

Data.equipment_create {
    name = "Amulet of Regeneration",
--    description = "Grants the user the spell 'Regeneration'.",--, but the user has -25HP.",
    description = "Grants the user extra health regeneration.",--, but the user has -25HP.",
    type = "amulet",

    shop_cost = { 220, 340 },
--    stat_bonuses = { hp = -25},

    spr_item = "spr_amulets.regeneration",
    stat_bonuses = {hpregen = 2 / 60},
    --spells_granted = {"Regeneration"},
}

Data.equipment_create {
    name = "Amulet of Greater Fire",
    description = "Grants the user multidirectional Fire Bolt casts, and makes all projectile spells move faster.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.greaterfireball",
    effects_granted = {"AmuletGreaterFire"},
    --spells_granted = {"Fireball"},
    stat_bonuses = {spell_velocity_multiplier = 1.15}
}

--Data.equipment_create {
--    name = "Amulet of Fire",
--    description = "Grants the user the spell 'Fire Bolt'.",
--    type = "amulet",
--
--    shop_cost = { 70, 100 },
--
--    spr_item = "spr_amulets.fireball",
--    spells_granted = {"Fire Bolt"},
--}

Data.equipment_create {
    name = "Amulet of the Berserker",
    description = "Berserked user regenerates 4HP a second, and has 5% chance of summoning a centaur on kill.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.berserk",
    effects_granted = {"AmuletBerserker"}
}

Data.equipment_create {
    name = "Amulet of Great Pain",
    description = "Grants the user double healing power from necromancy spells.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.greaterpain",
    effects_granted = {"AmuletGreatPain"},
    stat_bonuses = {willpower = 1}
}

Data.equipment_create {
    name = "Amulet of Ice Form",
    description = "Grants the user the spell 'Ice Form'.",
    type = "amulet",

    shop_cost = { 200, 250 },

    spr_item = "spr_amulets.iceform",
    spells_granted = {"Ice Form"}
}

Data.equipment_create {
    name = "Amulet of Light",
    description = "Grants the wearer potent crowd dispersal powers.",
    type = "amulet",

    shop_cost = { 320, 440 },

    spr_item = "spr_amulets.light",
    spells_granted = {"Luminos"}
}

--Data.equipment_create {
--    name = "Amulet of Pain",
--    description = "Grants the user the spell 'Pain' and +1 willpower.",
--    type = "amulet",
--
--    shop_cost = { 120, 140 },
--
--    spr_item = "spr_amulets.pain",
--    spells_granted = {"Pain"},
--    stat_bonuses = {willpower = 1}
--}

Data.equipment_create {
    name = "Amulet of Ringholding",
    description = "Grants user the ability to wear an additional ring.",
    type = "amulet",

    shop_cost = { 120, 140 },

    spr_item = "spr_amulets.urand_bloodlust",
}

Data.equipment_create {
    name = "Snake Lanart",
    description = "Helps open Pixullochia. When equipped, summons a snake on every kill.",
    type = "lanart",

    -- TODO implement
    -- TODO use
    spr_item = "spr_runes.rune_snake",
    sellable = false
}

Data.equipment_create {
    name = "Tomb Lanart",
    description = "Helps open Pixullochia. When equipped, summons a zombie on every kill.",
    type = "lanart",

    -- TODO implement
    -- TODO use
    spr_item = "spr_runes.rune_tomb",
    sellable = false,
    effects_granted = {"SummonMummyOnKill"}
}


Data.equipment_create {
    name = "Abyssal Lanart",
    description = "Helps open Pixullochia. When equipped, summons a zombie on every kill.",
    type = "lanart",

    -- TODO implement
    -- TODO use
    spr_item = "spr_runes.rune_abyss",
    sellable = false,
    effects_granted = {"SummonMummyOnKill"}
}

Data.equipment_create {
    name = "Swarm Lanart",
    description = "Helps open Pixullochia. When equipped, grants +5 Green Power.",
    type = "lanart",

    -- TODO implement
    -- TODO use
    spr_item = "spr_runes.rune_swamp",
    sellable = false,
    effects_granted = {{"GreenPower", {power = 5}}}
}

Data.equipment_create {
    name = "Dragon Lanart",
    description = "Helps open Pixullochia. When equipped, grants +5 Red Resistance.",
    type = "lanart",

    -- TODO implement
    -- TODO use
    spr_item = "spr_runes.rune_demonic_4",
    sellable = false,
    effects_granted = {{"RedResist", {resist = 5}}}
}

Data.equipment_create {
    name = "Rage Lanart",
    description = "Helps open Pixullochia. When equipped, grants +1 willpower, and +8% melee rate.",
    type = "lanart",

    -- TODO implement
    -- TODO use
    spr_item = "spr_runes.rune_tartarus",
    sellable = false,
    melee_cooldown_multiplier = 1/1.08,
    stat_bonuses = {willpower=1}
}

Data.equipment_create {
    name = "Obliteration Lanart",
    description = "Helps open Pixullochia. When equipped, grants +50 spell velocity, and +5% casting rate.",
    type = "lanart",

    -- TODO implement
    -- TODO use
    spr_item = "spr_runes.rune_cerebov",
    sellable = false,
    spell_cooldown_multiplier = 1/1.05,
    spell_velocity_multiplier = 1.5,
}

