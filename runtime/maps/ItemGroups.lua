
local M = {} -- Submodule

M.store_items = {
    { item = "Arrow",              chance = 15,  amount = {5,20}  }
    , { item = "Silver Arrow",       chance = 10,  amount = {2,6}   }
    , { item = "Stone",              chance = 15,  amount = {3,15}  }
    -- Scrolls
    , { item = "Haste Scroll",       chance = 7                      }
    -- Potions
    , { item = "Health Potion",      chance = 10                     }
    , { item = "Mana Potion",        chance = 10                     }
    -- Rings
    , { item = "Ring of Spells",     chance = 3                      }
    , { item = "Ring of Vitality",   chance = 3                      }
    , { item = "Ring of Stone",      chance = 3                      }
    -- Helmets
    , { item = "Runed Helmet",       chance = 1                      }
    , { item = "Crown of Mars",      chance = 1                      }
    , { item = "Crown of Orcus",     chance = 1                      }
    , { item = "Diana's Diadem",     chance = 1                      }
    -- Rings
    , { item = "Magician's Ring",    chance = 1                      }
    , { item = "Moonstone Ring",     chance = 1                      }
    , { item = "Ogre's Ring",        chance = 1                      }
    , { item = "Hydra Ring",         chance = 1                      }
    , { item = "Gallanthor's Ring",         chance = 1               }
    -- Bows
    , { item = "Mars Bow",           chance = 1                      }
    -- Boots
    , { item = "Iron Boots",         chance = 1                      }
    -- Gloves
    , { item = "Steel Gloves",       chance = 1                      }
    , { item = "Runed Gloves",       chance = 1                      }
    -- Saves
    , { item = "Devious Staff",                 chance = 1           }
    , { item = "Gallanthor's Crutch",                 chance = 1     }
    -- Permanent enchantments
    , { item = "Strength Scroll",    chance = 1                      }
    , { item = "Magic Scroll",       chance = 1                      }
    , { item = "Defence Scroll",     chance = 1                      }
    , { item = "Will Scroll",        chance = 1                      }
    , { item = "Scroll of Experience",        chance = 1             }
}

M.basic_items = {
    { item = "Gold",                 chance = 125, amount = {5,15}  }
    -- Projectiles
    , { item = "Arrow",              chance = 15,  amount = {5,20}  }
    , { item = "Silver Arrow",       chance = 10,  amount = {2,6}   }
    , { item = "Stone",              chance = 15,  amount = {3,15}  }
    -- Ranged Weapons
    , { item = "Short Bow",          chance = 10                     }
    -- Melee Weapons
    , { item = "Dagger",             chance = 10                     }
    , { item = "Short Sword",        chance = 10                     }
    , { item = "Mace",               chance = 5                      }
    -- Scrolls
    , { item = "Haste Scroll",       chance = 7                      }
    -- Potions
    , { item = "Health Potion",      chance = 10                     }
    , { item = "Mana Potion",        chance = 10                     }
    -- Body Armour
    , { item = "Leather Armour",     chance = 4                      }
    , { item = "Robe",               chance = 4                      }
    -- Helmets
    , { item = "Iron Helmet",        chance = 4                      }
    , { item = "Wizard's Hat",       chance = 4                      }
    -- Rings
    , { item = "Ring of Spells",     chance = 3                      }
    , { item = "Ring of Vitality",   chance = 3                      }
    , { item = "Ring of Stone",      chance = 3                      }
    -- Boots
    , { item = "Leather Boots",      chance = 5                      }
    -- Gloves
    , { item = "Leather Gloves",     chance = 5                      }
}

M.enchanted_items = {
    { item = "Stone",              chance = 5,  amount = {3,15}  }
    -- Melee Weapons
    , { item = "Mace",               chance = 3                      }
    -- Body Armour
    , { item = "Leather Armour",     chance = 3                      }
    , { item = "Robe",               chance = 3                      }
    -- Helmets
    , { item = "Iron Helmet",        chance = 3                      }
    , { item = "Wizard's Hat",       chance = 3                      }
    -- Boot
    , { item = "Leather Boots",      chance = 3                      }
    -- Gloves
    , { item = "Leather Gloves",     chance = 3                      }
    , { item = "Silver Arrow",       chance = 3,  amount = {2,8}   }
    -- TODO REMOVE ABOVE REDUNDANCIES^
    -- Ranged Weapons
    , { item = "Long Bow",           chance = 2                      }
    , { item = "Mars Bow",           chance = 1                      }
    -- Melee Weapons
    , { item = "Samurai Sabre",      chance = 1                      }
    , { item = "Magic Sword",        chance = 2                      }
    -- Potions
    , { item = "Health Potion",      chance = 10                     }
    , { item = "Mana Potion",        chance = 10                     }
    -- Body Armour
    , { item = "Chainmail",          chance = 2                      }
    , { item = "Platemail",          chance = 1                      }
    , { item = "Magician's Robe",    chance = 2                      }
    , { item = "Runed Robe",         chance = 1                      }
    , { item = "Robe of Mana",         chance = 1                      }
    -- Helmets
    , { item = "Runed Helmet",       chance = 1                      }
    , { item = "Crown of Mars",      chance = 1                      }
    , { item = "Crown of Orcus",     chance = 1                      }
    , { item = "Diana's Diadem",     chance = 1                      }
    -- Rings
    , { item = "Magician's Ring",    chance = 1                      }
    , { item = "Moonstone Ring",     chance = 1                      }
    , { item = "Ogre's Ring",        chance = 1                      }
    , { item = "Hydra Ring",         chance = 1                      }
    , { item = "Gallanthor's Ring",         chance = 1               }
    -- Boots
    , { item = "Iron Boots",         chance = 1                      }
    -- Gloves
    , { item = "Steel Gloves",       chance = 1                      }
    , { item = "Runed Gloves",       chance = 1                      }
    -- Saves
    , { item = "Devious Staff",                 chance = 1           }
    , { item = "Gallanthor's Crutch",                 chance = 1     }
    -- Permanent enchantments
    , { item = "Strength Scroll",    chance = 1                      }
    , { item = "Magic Scroll",       chance = 1                      }
    , { item = "Defence Scroll",     chance = 1                      }
    , { item = "Will Scroll",        chance = 1                      }
    , { item = "Scroll of Experience",        chance = 1             }
}

return M
