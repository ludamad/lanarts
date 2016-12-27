
M = {} -- Submodule
M.RANDART = (type) -> nilprotect {:type}

-- Rings
rings = (chance, args) -> {
    :chance
    args.ignore_strong and { item: "Magician's Ring",    chance: 2                      }
    args.ignore_strong and { item: "Moonstone Ring",     chance: 2                      }
    args.ignore_strong and { item: "Ogre's Ring",        chance: 2                      }
    args.ignore_strong and { item: "Hydra Ring",         chance: 2                      }
    args.ignore_strong and{  item: "Gallanthor's Ring",  chance: 1                      }
    { item: "Ring of Spells",     chance: 2                      }
    { item: "Ring of Vitality",   chance: 2                      }
    { item: "Ring of Stone",      chance: 1                      }
--    { item: RANDART("Ring"),      chance: 1                      }
}

-- Amulets
amulets = (chance) -> { 
    :chance
    { item: "Amulet of the Wall", chance: 1 }
    { item: "Amulet of Fear", chance: 1 }
    { item: "Amulet of Trepidation", chance: 1 }
    { item: "Amulet of Mephitization", chance: 1 }
    { item: "Amulet of Regeneration", chance: 1 }
    { item: "Amulet of Fire", chance: 1 }
    { item: "Amulet of Greater Fire", chance: 1 }
    { item: "Amulet of Protection", chance: 1 }
    { item: "Amulet of the Berserker", chance: 1 }
    { item: "Amulet of Pain", chance: 1 }
    { item: "Amulet of Great Pain", chance: 1 }
--    { item: RANDART("Amulet"),      chance: 1 }
}

-- Consumables
consumables = (chance) -> { 
    :chance
    {
        chance: 95
        { item: "Arrow",              chance: 8,  amount: {5,20}  }
        { item: "Silver Arrow",       chance: 8,  amount: {2,6}   }
    --  Ludamad: Remove stones for now. Part of: Try to reduce the projectile actions for non-archer a lot.
    --    { item: "Stone",              chance: 15,  amount: {3,15}  }
        -- Scrolls
        { item: "Haste Scroll",       chance: 2                      }
        -- Potions
        { item: "Health Potion",      chance: 4                     }
        { item: "Mana Potion",        chance: 4                     }
    }

    -- Permanent enchantments
    { 
        chance: 5
        { item: "Strength Scroll",    chance: 1                      }
        { item: "Magic Scroll",       chance: 1                      }
        { item: "Defence Scroll",     chance: 1                      }
        { item: "Will Scroll",        chance: 1                      }
        { item: "Scroll of Experience",        chance: 1             }
    }
}

-- Headgear
helmets = (chance, args) -> { 
    :chance
    -- Helmets
    args.ignore_weak and { item: "Iron Helmet",        chance: 3                      }
    args.ignore_weak and { item: "Wizard's Hat",       chance: 3                      }
    { item: "Runed Helmet",       chance: 1                      }
    { item: "Crown of Mars",      chance: 1                      }
    { item: "Crown of Orcus",     chance: 1                      }
    { item: "Diana's Diadem",     chance: 1                      }
}

-- Belts
belts = (chance) -> { 
    :chance
    { item: "Belt of Protection",      chance: 10                      }
    { item: "Belt of Slaying",        chance: 10                      }
    { item: "Warped Belt",        chance: 1                      }
}

-- Legwear
legwear  = (chance) -> { 
    :chance
    { item: "Platelegs",      chance: 1                      }
    { item: "Magic Skirt",        chance: 1                      }
}

_filter = (group) -> 
    -- Remove objects dummied out with 'true':
    table.remove_occurrences(group, true)
    for _, v in ipairs(group)
        _filter(v)
    return group

bows  = (chance, args) -> _filter {
    :chance 
    {
        chance: 10
        args.ignore_weak and {
            chance: 10
            { item: "Short Bow",          chance: 2                      }
        } 
        args.ignore_medium and {
            chance: 10
            -- Bows
            { item: "Long Bow",           chance: 2                      }
        }
        args.ignore_strong and {
            chance: 10
            -- Bows
            { item: "Mars Bow",           chance: 90                      }
            { item: "The Piercer",           chance: 1                      }
        }
    }
--    args.ignore_randarts and { item: RANDART("Bow"), chance: 1}
}

armour = (chance, args) -> _filter {
    :chance,
    -- Body Armour
    args.ignore_weak and { item: "Leather Armour",     chance: 4                      }
    args.ignore_weak and { item: "Robe",               chance: 4                      }

    args.ignore_medium and{ item: "Chainmail",          chance: 2                      }
    args.ignore_medium and { item: "Magician's Robe",    chance: 2                      }
    args.ignore_medium and { item: "Robe of Mana",       chance: 2                      }

    args.ignore_strong and { item: "Platemail",          chance: 1                      }
    args.ignore_strong and { item: "Runed Robe",         chance: 1                      }
}

boots = (chance, args) -> _filter {
    :chance 
    {
        chance: 10
        args.ignore_weak and {
            chance: 100
            { item: "Leather Boots",      chance: 1                      }
        } 
        args.ignore_medium and {
            chance: 20
            { item: "Iron Boots",           chance: 2                      }
        }
        args.ignore_strong and {
            chance: 5
            { item: "Troll Boots",           chance: 90                      }
        }
    }
--    args.ignore_randarts and { item: RANDART("Boots"), chance: 1}
}

gloves = (chance, args) -> _filter {
    :chance 
    {
        chance: 10
        args.ignore_weak and {
            chance: 100
            { item: "Leather Gloves",      chance: 1                      }
        } 
        args.ignore_medium and {
            chance: 20
            -- Bows
            { item: "Steel Gloves",           chance: 2                      }
        }
        args.ignore_strong and {
            chance: 5
            -- Bows
            { item: "Runed Gloves",           chance: 90                      }
        }
    }
--    args.ignore_randarts and { item: RANDART("Gloves"), chance: 1}
}

weapons = (chance, args) -> _filter {
    :chance 
    {
        chance: 10
        args.ignore_weak and {
            chance: 100
            { item: "Dagger",      chance: 1                      }
            { item: "Short Sword",      chance: 1                      }
        } 
        args.ignore_medium and {
            chance: 20
            { item: "Devious Staff",                 chance: 1           }
            { item: "Mace",                          chance: 1           }
        }
        args.ignore_strong and {
            chance: 5
            { item: "Iron Broadsword",                          chance: 1           }
            { item: "Gallanthor's Crutch",                 chance: 1     }
            { item: "Samurai Sabre",      chance: 1                      }
            { item: "Magic Sword",        chance: 1                      }
            { item: "Triple Sword",        chance: 1                      }
            { item: "Battle Axe",        chance: 1                      }
            { item: "Iron Scimitar",        chance: 1                      }
            { item: "Orc Axe",        chance: 1                      }
        }
    }
--    args.ignore_randarts and { item: RANDART("Weapon"), chance: 1}
}

M.store_items = {
    consumables(22, ignore_weak: true)
    rings(8, ignore_weak: true)
    helmets(5, ignore_weak: true)
    armour(5, ignore_weak: true)
    belts(2, ignore_weak: true)
    legwear(2, ignore_weak: true)
    boots(1, ignore_weak: true)
    bows(5, ignore_weak: true)
    gloves(5, ignore_weak: true)
    weapons(8, ignore_weak: true, ignore_medium: true)
}

M.enchanted_items = {
    chance: 10 -- For basic_items entry
    { item: "Gold",                 chance: 85, amount: {15,45}  }
    consumables(40, ignore_weak: true)
    rings(8, ignore_weak: true)
    helmets(5, ignore_weak: true)
    armour(5, ignore_weak: true)
    belts(1, ignore_weak: true)
    legwear(5, ignore_weak: true)
    boots(1, ignore_weak: true)
    bows(5, ignore_weak: true)
    gloves(5, ignore_weak: true)
    weapons(12, ignore_weak: true, ignore_medium: true)
--  Ludamad: Remove stones for now. Part of: Try to reduce the projectile actions for non-archer a lot.
--    { item: "Stone",              chance: 5,  amount: {3,15}  }
}

M.basic_items = {
    { item: "Gold",                 chance: 100, amount: {5,15}  }
    consumables(40, ignore_strong: true)
    rings(2, ignore_strong: true)
    helmets(5, ignore_strong: true)
    armour(12, ignore_strong: true)
    belts(1, ignore_strong: true)
    legwear(2, ignore_strong: true)
    boots(1, ignore_strong: true)
    bows(5, ignore_strong: true)
    gloves(5, ignore_strong: true)
    weapons(12, ignore_strong: true)
    M.enchanted_items -- Chance of being moved up a category. See enchanted_items for the weight.
}

return M
