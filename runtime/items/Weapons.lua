local DataW = require "DataWrapped"

-- Projectiles:
-- This entry is used as the basic 'nothing' equipment slot.
Data.projectile_create {
    name = "Nothing", -- This must be named nothing.
    weapon_class = "none",
    spr_item = "none"
}

DataW.projectile_create {
    name = "Arrow",
    shop_cost = {2,3},
    description = "A trusty arrow, for use with a bow.",
    weapon_class = "bows",
    types = {"Piercing"},
    spr_item = "arrow_item",
    spr_attack = "arrow_projectile",
    -- Stats
    drop_chance = 90, -- % chance 
    speed = 6.75
}

DataW.projectile_create {
    name = "Silver Arrow",
    shop_cost = {3,4},
    description = "A deadly silver arrow.",
    weapon_class = "bows",
    types = {"Piercing"},
    spr_item = "stone_arrow_item",
    spr_attack = "stone_arrow_projectile",
    -- Stats
    damage_bonuses = {
      damage = {base = {4,6}},
      power = {base = 2}
    },
    drop_chance = 95, -- % chance
    speed = 7
}

DataW.projectile_create {
    name = "Stone",
    shop_cost = {2,2},
    description = "A fist sized stone, good for throwing.",
    weapon_class = "unarmed",
    types = {"Bludgeon"},
    spr_item = "stone",
    drop_chance = 90, -- % chance
    -- Stats
    range = 300,
    speed = 6,
    cooldown = 45,
    damage = {base = {3,4}},
    power = {base = {0,1}, strength = 1.0}
}

DataW.weapon_create {
    name = "Unarmed",
    description = "You are currently fighting with your fists.",
    type = "unarmed",
    spr_item = "hand",
    cooldown = 30,
}

DataW.weapon_create {
    name = "Pain",
    description = "Instantly damage a nearby enemy, but hurt yourself in the process. If you kill the enemy, you gain back one third its life.",
    type = "unarmed",
    types = {"Black"},
    spr_item = "spr_spells.pain",
    cooldown = 30,

    alt_action = function(caster, xy, target)
        caster:try_use_spell("Pain", xy, target)
    end,

    range = 50
}


DataW.weapon_create {
    name = "Apprentice Staff",
    description = "A staff of endless minor bolts.",
    type = "staves",

    types = {
    },
    shop_cost = {75, 105},

    spr_item = "spr_weapons.staff-of-minor",
    alt_action = function(caster, xy, target)
        caster:try_use_spell("Minor Missile", xy, target)
    end,

    cooldown = 200,
    range = 400
}

DataW.weapon_create {
    name = "Dagger",
    description = "A short, swift, pointy blade.",
    type = "short blades",
    types = {"Slashing"},

    shop_cost = {0,0},
    spr_item = "spr_weapons.dagger",

    cooldown = 35,

    range = 7

}

DataW.weapon_create {
    name = "Short Sword",
    description = "An easy-to-wield short sword.",
    type = "short blades",
    types = {"Slashing"},

    shop_cost = {20, 35},

    spr_item = "spr_weapons.short_sword1",

--stat_bonuses = {
    --    strength = 1
    --},
    cooldown = 40,

    range = 7
}

--DataW.weapon_create {
--    name = "Standard Sword",
--    description = "A fairly easy-to-wield standard sword.",
--    type = "short blades",
--
--    shop_cost = {40, 55},
--
--    spr_item = "spr_weapons.sword",
--
--    --stat_bonuses = {
--    --    strength = 2
--    --},
--
--    cooldown = 40,
--    range = 7
--}
DataW.weapon_create {
    name = "Wizard's Staff",
    description = "A magical staff that gives MP back for every kill.",
    type = "staves",

    types = {
        "Bludgeon"
    },
    shop_cost = {75, 105},

    spr_item = "spr_weapons.staff",

    cooldown = 40,
    range = 7,
    effects_granted = {"WeaponManaGainOnKill"}
}

DataW.weapon_create {
    name = "Vine Staff",
    description = "A magical vine staff.",
    type = "staves",

    types = {
        "Green", "Bludgeon"
    },
    shop_cost = {75, 105},

    spr_item = "spr_weapons.spwpn_staff_of_olgreb",

    cooldown = 40,
    range = 7,
    effects_granted = {"WeaponManaGainOnKill"}
}

DataW.weapon_create {
    name = "Gallanthor's Crutch",
    description = "The epic staff of crippled archmage Gallanthor. Used in the first 200 years of his life.",
    type = "staves",

    shop_cost = {125, 255},
    types = {
        "Bludgeon"
    },

    spr_item = "epic staff",
    

    cooldown = 40,
    stat_bonuses = {magic = 2, spell_cooldown_multiplier = 0.9},

    range = 7
}

DataW.weapon_create {
    name = "Gallanthor's 5-Colour Staff",
    description = "The epic staff of crippled archmage's later perfection of the chromatic arts. Used in the first 500 years of his life.",
    type = "staves",

    shop_cost = {525, 2055},
    types = {
        "Bludgeon"
    },

    spr_item = "epic staff",
    

    cooldown = 40,
    stat_bonuses = {magic = 3, spell_cooldown_multiplier = 0.85},

    range = 7,
    effects_granted = {
        {"RedPower", {power=1}},
        {"BluePower", {power=1}},
        {"BlackPower", {power=1}},
        {"GreenPower", {power=1}},
        {"WhitePower", {power=1}},
    }
}

DataW.weapon_create {
    name = "Serated Sword",
    description = "A cruel jagged sword.",
    type = "short blades",
    types = {"Slashing"},

    shop_cost = {100, 160},

    spr_item = "spr_weapons.double_sword",

    damage_multiplier = 1.25,
    cooldown = 45,

    range = 7
}

DataW.weapon_create {
    name = "Iron Broadsword",
    description = "Long and heavy, but effective.",
    type = "blades",
    types = {"Slashing"},

    shop_cost = {70, 100},

    spr_item = "iron broadsword",

    --stat_bonuses = {strength = 2},
    damage_multiplier = 1.10,

    cooldown = 60,

    range = 7
}

DataW.weapon_create {
    name = "Gragh's Club",
    description = "A club owned by Gragh, embued with his life force as he died. Has a chance of knocking back enemies on hit. Regains health on kill.",
    type = "axes and maces",

    types = {
        "Bludgeon"
    },
    shop_cost = {350, 500},

    spr_item = "spr_weapons.giant_club2",
    
    damage_multiplier = 1.25,
    stat_bonuses = {strength = 3},

    cooldown = 55,

    range = 7,
    effects_granted = {"KnockbackWeapon", "WeaponHealthGainOnKill"}
}


DataW.weapon_create {
    name = "Mace of Light",
    description = "A large, strong mace.",
    type = "axes and maces",
    types = {"White", "Bludgeon"},

    shop_cost = {35, 50},

    spr_item = "spr_weapons.urand_brilliance",

    cooldown = 55,

    range = 7
}


DataW.weapon_create {
    name = "Mace",
    description = "A large, strong mace.",
    type = "axes and maces",
    types = {"Bludgeon"},

    shop_cost = {35, 50},

    spr_item = "mace",

    cooldown = 55,

    range = 7
}

DataW.weapon_create {
    name = "Magic Sword",
    description = "An enchanted sword that saps your mana reserves, but does greater damage.",
    type = "short blades",
    types = {"Slashing"},

    shop_cost = {75, 95},

    spr_item = "magic sword",

    cooldown = 42,

    range = 7,

    -- Damage bonus implemented in C++, here is the MP cost:
    on_hit_func = function(user, target) 
        local mpcost = 20
        if user.stats.mp >= mpcost then
            user.stats.mp = user.stats.mp - mpcost
        end
    end
      
}

--DataW.weapon_create {
--    name = "Triple Sword",
--    description = "A very slow huge sword of great power.",
--    type = "axes and maces",
--
--    shop_cost = {125, 165},
--
--    spr_item = "spr_weapons.triple_sword",
--    randart_weight = 5,
--    randart_sprites = {"spr_weapons.randart_triple_sword1", "spr_weapons.randart_triple_sword2"},
--
--    damage = {base = {10, 15}, strength = 1.6},
--    power = {base = 12, strength = 1.0},
--
--    cooldown = 85,
--
--    range = 7
--}
--
--DataW.weapon_create {
--    name = "Battle Axe",
--    description = "A slow powerful axe that decimates the weak.",
--    type = "axes and maces",
--
--    shop_cost = {85, 105},
--
--    spr_item = "battle axe",
--
--    damage = {base = {9, 13}, strength = 1.4},
--    power = {base = 6, strength = 0.3},
--
--    cooldown = 65,
--
--    range = 7
--}

DataW.weapon_create {
    name = "Ice Axe",
    description = "An axe built by ice orcs.",
    type = "axes and maces",

    types = {
        "Blue", "Bludgeon"
    },
    shop_cost = {100, 120},
    spr_item = "spr_weapons.urand_frostbite",

    cooldown = 65,

    range = 8
}


DataW.weapon_create {
    name = "Orc Axe",
    description = "An axe built by fierce orcs.",
    type = "axes and maces",

    types = {
        "Bludgeon"
    },
    shop_cost = {100, 120},

    spr_item = "orc axe",

    cooldown = 65,

    range = 8
}

DataW.weapon_create {
    name = "Dark Scythe",
    description = "A darkly scythe.",
    type = "blades",
    types = {"Black", "Slashing"},

    shop_cost = {100, 150},

    spr_item = "spr_weapons.spwpn_scythe_of_curses",

    damage_multiplier = 1.25,
    cooldown = 25,

    range = 5
}


DataW.weapon_create {
    name = "Flaming Scimitar",
    description = "A scimitar of red magic, attacks with sharp fire.",
    type = "blades",
    types = {"Red", "Slashing"},

    shop_cost = {100, 150},

    spr_item = "spr_weapons.urand_flaming_death",

    damage_multiplier = 1.25,
    cooldown = 25,

    range = 5
}

DataW.weapon_create {
    name = "Iron Scimitar",
    description = "A scimitar of iron, slices quickly and efficiently.",
    type = "blades",
    types = {"Slashing"},

    shop_cost = {100, 150},

    spr_item = "iron scimitar",

    damage_multiplier = 1.25,
    cooldown = 25,

    range = 5
}

--------------------------
-- BEGIN RANGED WEAPONS --
--------------------------
DataW.weapon_create {
    name = "Short Bow",
    description = "A handy small bow.",
    type = "bows",

    -- Arrows are piercing, no types here
    shop_cost = {10, 20},

    spr_item = "bow1",

    uses_projectile = true,

    cooldown = 50,

    range = 300
}

DataW.weapon_create {
    name = "Long Bow",
    description = "A powerful long-bow.",
    type = "bows",

    -- Arrows are piercing, no types here
    shop_cost = {95, 125},

    spr_item = "long bow",

    uses_projectile = true,

    cooldown = 55,

    range = 300
}

DataW.weapon_create {
    name = "Mars Bow",
    description = "A bow begotten of Mars' blessing.",
    type = "bows",

    shop_cost = {205, 245},

    spr_item = "spr_weapons.urand_storm_bow",

    uses_projectile = true,

    damage_multiplier = 1.25, -- +25% damage bonus

    cooldown = 55,

    range = 300
}

DataW.weapon_create {
    name = "The Piercer",
    description = "An artifact bow of great power. Lowers HP and MP.",
    type = "bows",

    shop_cost = {305, 445},

    spr_item = "spr_weapons.urand_piercer",

    uses_projectile = true,

    damage_multiplier = 1.50, -- +50% damage bonus

    cooldown = 45,
    stat_bonuses = {mp = -30, hp = -30, strength = 2, magic = 2, defence = 3, willpower = 3},

    range = 300
}

-- BEGIN MONSTER ATTACK TYPES ---

DataW.weapon_create {
    name = "Slow Melee",
    type = "unarmed",

    spr_item = "none",

    types = {"Bludgeon"}, 
    damage = {base = {2, 5}, strength = 1.5},
    power = {base = 2, strength = 0.3},

    cooldown = 70,

    range = 7
}


DataW.weapon_create {
    name = "Basic Melee",
    type = "unarmed",

    spr_item = "none",


    types = {"Bludgeon"}, 
    damage = {base = {2, 5}, strength = 1},
    power = {base = 0, strength = 0.2},

    cooldown = 40,

    range = 7
}

DataW.weapon_create {
    name = "Magic Melee",
    type = "unarmed",

    spr_item = "none",

    types = {"Bludgeon"}, 
    damage_type = {magic = 0.5, physical = 0.5},

    damage = {base = {2, 5}, strength = 1},
    power = {base = 0, strength = 0.2},

    cooldown = 40,

    range = 7
}

DataW.weapon_create {
    name = "Fast Halfmagic Melee",
    type = "unarmed",

    spr_item = "none",

    types = {"Bludgeon"}, 
    damage_type = {magic = 0.5, physical = 0.5},

    damage = {base = {2, 5}, strength = 0.5},
    power = {base = 0, strength = 0.2},

    cooldown = 20,

    range = 7
}


DataW.weapon_create {
    name = "Fast Melee",
    type = "unarmed",

    spr_item = "none",

    types = {"Bludgeon"}, 
    damage = {base = {2, 5}, strength = 0.5},
    power = {base = 0, strength = 0.2},

    cooldown = 20,

    range = 7
}

DataW.weapon_create {
    name = "Fast Magic Melee",
    type = "unarmed",

    spr_item = "none",

    types = {"Bludgeon"}, 
    damage_type = {magic = 0.5, physical = 0.5},
    damage = {base = {2, 5}, strength = 0.5},
    power = {base = 0, strength = 0.2},

    cooldown = 20,

    range = 7
}

-- END MONSTER ATTACK TYPES --

