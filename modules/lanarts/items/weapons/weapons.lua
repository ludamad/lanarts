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

    spr_item = "dagger",

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


    damage = {base = {4, 8}, strength = 1.1},
    power = {base = 1, strength = 0.2},

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

    range = 10
}

Data.weapon_create {
    name = "Mace",
    description = "An large, strong mace.",
    type = "axes and maces",

    shop_cost = {35, 50},

    spr_item = "mace",

    resist_modifier = 1.1,

    damage = {base = {5, 10}, strength = 1.25},
    power = {base = {2, 5}, strength = 0.5},

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

    damage = {strength = 1.1, magic = 0.2, base = {4, 8}},
    power = {base = 1, strength = 0.2},

    cooldown = 42,

    range = 7,

    on_hit_func = function(user, target) 
        local mpcost = rand_range(4,6)
        if user.stats.mp >= mpcost then
       	    -- Essentially 1/4th extra damage:
            target:damage(user.stats.strength / 4 + rand_range(1,2), user.stats.strength / 5 + 1, 0.0, 0.25)
            user.stats.mp = user.stats.mp - mpcost
        end
    end
      
}

Data.weapon_create {
    name = "Battle Axe",
    description = "A slow powerful axe that decimates the weak.",
    type = "axes and maces",

    shop_cost = {85, 105},

    spr_item = "battle axe",

    resist_modifier = 1.4,

    damage = {base = {8, 12}, strength = 1.4},
    power = {base = 6, strength = 0.25},

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

Data.weapon_create {
    name = "Short Bow",
    description = "A handy small bow.",
    type = "bows",

    shop_cost = {10, 20},

    spr_item = "bow1",

    uses_projectile = true,

    damage = {base = {3, 6}, strength = 1.15},
    power = {base = 3, strength = 0.2},

    cooldown = 50,

    range = 300
}

Data.weapon_create {
    name = "Long Bow",
    description = "A powerful long-bow.",
    type = "bows",

    shop_cost = {75, 85},

    spr_item = "long bow",

    uses_projectile = true,

    damage = {base = {5, 9}, strength = 1.2},
    power = {base = 5, strength = 0.25},

    cooldown = 55,

    range = 300
}

Data.weapon_create {
    name = "Wand of Minor Missile",
    description = "A powerful long-bow.",
    type = "wands",

    shop_cost = {75, 85},

    spr_item = "missle wand",

    uses_projectile = true,

    damage = {base = {5, 9}, strength = 1.2},
    power = {base = 5, strength = 0.25},

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
