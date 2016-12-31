Data.weapon_create {
    name = "Unarmed",
    description = "You are currently fighting with your fists.",
    type = "unarmed",

    spr_item = "hand",

    resist_modifier = 0.75,

    damage = {base = {2, 5}, strength = 0.75},
    power = {base = 0, strength = 0.2},

    cooldown = 30,

    range = 7
}

Data.weapon_create {
    name = "Dagger",
    description = "A short, swift, pointy blade.",
    type = "short blades",

    shop_cost = {0,0},
    spr_item = "dagger",
    randart_sprites = {"spr_weapons.randart_dagger1", "spr_weapons.randart_dagger2"},

    resist_modifier = 0.875,

    damage = {base = {4, 6}, strength = 0.875},
    power = {base = 0, strength = 0.2},

    cooldown = 35,

    range = 7

}

Data.weapon_create {
    name = "Short Sword",
    description = "An easy-to-wield short sword.",
    type = "short blades",

    shop_cost = {20, 35},

    spr_item = "short sword",
    randart_sprites = {"spr_weapons.randart_short_sword1", "spr_weapons.randart_short_sword2"},

    damage = {base = {4, 8}, strength = 1.1},
    power = {base = 1, strength = 0.2},

    cooldown = 40,

    range = 7
}

Data.weapon_create {
    name = "Wizard's Staff",
    description = "A magical staff that gives MP back for every kill.",
    type = "staves",

    shop_cost = {75, 105},

    spr_item = "spr_weapons.staff",
    randart_sprites = {"spr_weapons.staff_mummy"},

    damage = {strength = 0.6, magic = 0.6, base = {5, 9}},
    power = {base = 1, strength = 0.2},
    stat_bonuses = { magic = 1 },

    cooldown = 40,
    range = 7,
    effects_granted = {"ManaGainOnKill"}
}

Data.weapon_create {
    name = "Gallanthor's Crutch",
    description = "The epic staff of crippled archmage",
    type = "staves",

    shop_cost = {125, 155},

    spr_item = "epic staff",

    damage = {strength = 0.6, magic = 0.6, base = {5, 9}},
    power = {base = 1, strength = 0.2},
    stat_bonuses = { magic = 1 },
    spell_cooldown_multiplier = 0.9,

    cooldown = 40,

    range = 7
}


Data.weapon_create {
    name = "Samurai Sabre",
    description = "A swift blade once owned by a powerful warrior.",
    type = "short blades",

    shop_cost = {70, 100},

    spr_item = "samurai sabre",


    damage = {base = {5, 11}, strength = 1.1},
    power = {base = 4, strength = 0.3},

    cooldown = 45,

    range = 7
}

Data.weapon_create {
    name = "Iron Broadsword",
    description = "Long and Heavy, but effective.",
    type = "blades",

    shop_cost = {70, 100},

    spr_item = "iron broadsword",

    resist_modifier = 1,

    damage = {base = {5, 10}, strength = 1.3},
    power = {base = 6, strength = 0.35},

    cooldown = 60,

    range = 7
}

Data.weapon_create {
    name = "Gragh's Club",
    description = "A club owned by Gragh, embued with his life force as he died. Has a chance of knocking back enemies on hit.",
    type = "axes and maces",

    shop_cost = {350, 500},

    spr_item = "spr_weapons.giant_club2",

    resist_modifier = 1.1,

    damage = {base = {8, 13}, strength = 1.25},
    power = {base = {3, 6}, strength = 0.35},

    cooldown = 55,

    range = 7,
    effects_granted = {"KnockbackWeapon"}
}


Data.weapon_create {
    name = "Mace",
    description = "A large, strong mace.",
    type = "axes and maces",

    shop_cost = {35, 50},

    spr_item = "mace",
    randart_sprites = {"spr_weapons.randart_mace1", "spr_weapons.randart_mace2", "spr_weapons.randart_mace3"},

    resist_modifier = 1.1,

    damage = {base = {7, 12}, strength = 1.25},
    power = {base = {2, 5}, strength = 0.35},

    cooldown = 55,

    range = 7
}

Data.weapon_create {
    name = "Magic Sword",
    description = "An enchanted sword that saps your mana reserves, but does greater damage.",
    type = "short blades",

    shop_cost = {75, 95},

    spr_item = "magic sword",

    resist_modifier = 1.1,

    damage = {strength = 1.1, magic = 0.2, base = {5, 9}},
    power = {base = 1, strength = 0.2, magic = 0.2},

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

Data.weapon_create {
    name = "Triple Sword",
    description = "A very slow huge sword of great power.",
    type = "axes and maces",

    shop_cost = {125, 165},

    spr_item = "spr_weapons.triple_sword",
    randart_weight = 5,
    randart_sprites = {"spr_weapons.randart_triple_sword1", "spr_weapons.randart_triple_sword2"},

    resist_modifier = 1.5,

    damage = {base = {10, 15}, strength = 1.6},
    power = {base = 12, strength = 1.0},

    cooldown = 85,

    range = 7
}

Data.weapon_create {
    name = "Battle Axe",
    description = "A slow powerful axe that decimates the weak.",
    type = "axes and maces",

    shop_cost = {85, 105},

    spr_item = "battle axe",

    resist_modifier = 1.4,

    damage = {base = {9, 13}, strength = 1.4},
    power = {base = 6, strength = 0.3},

    cooldown = 65,

    range = 7
}

Data.weapon_create {
    name = "Orc Axe",
    description = "An axe built from orc hide",
    type = "axes and maces",

    shop_cost = {100, 120},

    spr_item = "orc axe",

    resist_modifier = 1.4,

    damage = {base = {10, 14}, strength = 1.4},
    power = {base = 6, strength = 0.3},

    cooldown = 65,

    range = 8
}

Data.weapon_create {
    name = "Iron Scimitar",
    description = "A scimitar of iron, slices quickly and efficiently.",
    type = "blades",

    shop_cost = {100, 150},

    spr_item = "iron scimitar",

    resist_modifier = 0.9,

    damage = {base = {1, 5}, strength = 1.1},
    power = {base = 5, strength = 0.3},

    cooldown = 25,

    range = 5
}

--------------------------
-- BEGIN RANGED WEAPONS --
--------------------------
Data.weapon_create {
    name = "Short Bow",
    description = "A handy small bow.",
    type = "bows",

    shop_cost = {10, 20},

    spr_item = "bow1",

    uses_projectile = true,
    randart_sprites = {"spr_weapons.randart_shortbow"},

    damage = {base = {3, 6}, strength = 1.15},
    power = {base = 3, strength = 0.2},

    cooldown = 50,

    range = 300
}

Data.weapon_create {
    name = "Long Bow",
    description = "A powerful long-bow.",
    type = "bows",

    shop_cost = {95, 125},

    spr_item = "long bow",

    uses_projectile = true,
    randart_sprites = {"spr_weapons.urand_krishna", "spr_weapons.longbow3"},

    damage = {base = {5, 9}, strength = 1.2},
    power = {base = 5, strength = 0.25},

    cooldown = 55,

    range = 300
}

Data.weapon_create {
    name = "Mars Bow",
    description = "A bow begotten of Mars' blessing.",
    type = "bows",

    shop_cost = {205, 245},

    spr_item = "spr_weapons.urand_storm_bow",

    uses_projectile = true,

    damage = {base = {6, 10}, strength = 1.2},
    power = {base = 6, strength = 0.25},

    cooldown = 55,

    range = 300
}

Data.weapon_create {
    name = "The Piercer",
    description = "An artifact bow of great power. Lowers HP and MP.",
    type = "bows",

    shop_cost = {305, 445},

    spr_item = "spr_weapons.urand_piercer",

    uses_projectile = true,

    damage = {base = {7, 11}, strength = 1.2},
    power = {base = 7, strength = 0.25},

    cooldown = 45,
    stat_bonuses = {mp = -30, hp = -30, strength = 2, magic = 2, defence = 1, willpower = 1},

    range = 300
}


Data.weapon_create {
    name = "Mars Bow",
    description = "A bow begotten of Mars' blessing.",
    type = "bows",

    shop_cost = {205, 245},

    spr_item = "mars bow",

    uses_projectile = true,

    damage = {base = {6, 10}, strength = 1.2},
    power = {base = 6, strength = 0.25},

    cooldown = 55,

    range = 300
}

-- BEGIN MONSTER ATTACK TYPES ---

Data.weapon_create {
    name = "Slow Melee",
    type = "unarmed",

    spr_item = "none",

    resist_modifier = 1.6,

    damage = {base = {2, 5}, strength = 1.5},
    power = {base = 2, strength = 0.3},

    cooldown = 70,

    range = 7
}


Data.weapon_create {
    name = "Basic Melee",
    type = "unarmed",

    spr_item = "none",


    damage = {base = {2, 5}, strength = 1},
    power = {base = 0, strength = 0.2},

    cooldown = 40,

    range = 7
}

Data.weapon_create {
    name = "Magic Melee",
    type = "unarmed",

    spr_item = "none",

    damage_type = {magic = 0.5, physical = 0.5},

    damage = {base = {2, 5}, strength = 1},
    power = {base = 0, strength = 0.2},

    cooldown = 40,

    range = 7
}

Data.weapon_create {
    name = "Fast Melee",
    type = "unarmed",

    spr_item = "none",

    resist_modifier = 0.5,

    damage = {base = {2, 5}, strength = 0.5},
    power = {base = 0, strength = 0.2},

    cooldown = 20,

    range = 7
}

Data.weapon_create {
    name = "Fast Magic Melee",
    type = "unarmed",

    spr_item = "none",

    damage_type = {magic = 0.5, physical = 0.5},
    resist_modifier = 0.5,

    damage = {base = {2, 5}, strength = 0.5},
    power = {base = 0, strength = 0.2},

    cooldown = 20,

    range = 7
}

-- END MONSTER ATTACK TYPES --

-- Start the game with 1000 'randarts' -- for now, preconfigured item generations.
require("items.Randarts").define_weapon_randarts()

