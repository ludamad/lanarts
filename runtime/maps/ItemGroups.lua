
local M = {} -- Submodule

M.store_items = {
    { item = "Arrow",              chance = 8,  amount = {5,20}  }
    , { item = "Silver Arrow",       chance = 8,  amount = {2,6}   }
--  Ludamad: Remove stones for now. Part of: Try to reduce the projectile actions for non-archer a lot.
--    , { item = "Stone",              chance = 15,  amount = {3,15}  }
    -- Scrolls
    , { item = "Haste Scroll",       chance = 2                      }
    -- Potions
    , { item = "Health Potion",      chance = 4                     }
    , { item = "Mana Potion",        chance = 4                     }
    -- Amulets
    , { item = "Amulet of the Wall", chance = 1 }
    , { item = "Amulet of Fear", chance = 1 }
    , { item = "Amulet of Trepidation", chance = 1 }
    , { item = "Amulet of Mephitization", chance = 1 }
    , { item = "Amulet of Regeneration", chance = 1 }
    , { item = "Amulet of Fire", chance = 1 }
    , { item = "Amulet of Greater Fire", chance = 1 }
    , { item = "Amulet of Protection", chance = 1 }
    -- Rings
    , { item = "Ring of Spells",     chance = 1                      }
    , { item = "Ring of Vitality",   chance = 1                      }
    , { item = "Ring of Stone",      chance = 1                      }
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
    -- Belts
    , { item = "Belt of Protection",      chance = 1                      }
    , { item = "Belt of Slaying",        chance = 1                      }
    -- Legwear
    , { item = "Platelegs",      chance = 1                      }
    , { item = "Magic Skirt",        chance = 1                      }
    -- Bows
    , { item = "Mars Bow",           chance = 1                      }
    -- Boots
    , { item = "Iron Boots",         chance = 1                      }
    , { item = "Troll Boots",         chance = 1                      }
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
    { item = "Gold",                 chance = 100, amount = {5,15}  }
    -- Projectiles
    , { item = "Arrow",              chance = 15,  amount = {5,20}  }
    , { item = "Silver Arrow",       chance = 10,  amount = {2,6}   }
--  Ludamad: Remove stones for now. Part of: Try to reduce the projectile actions for non-archer a lot.
--    , { item = "Stone",              chance = 15,  amount = {3,15}  }
    -- Ranged Weapons
    , { item = "Short Bow",          chance = 4                     }
    -- Melee Weapons
    , { item = "Dagger",             chance = 4                     }
    , { item = "Short Sword",        chance = 4                     }
    , { item = "Mace",               chance = 4                      }
    -- Scrolls
    , { item = "Haste Scroll",       chance = 2                      }
    -- Potions
    , { item = "Health Potion",      chance = 8                     }
    , { item = "Mana Potion",        chance = 8                     }
    -- Body Armour
    , { item = "Leather Armour",     chance = 4                      }
    , { item = "Robe",               chance = 4                      }
    -- Helmets
    , { item = "Iron Helmet",        chance = 4                      }
    , { item = "Wizard's Hat",       chance = 4                      }
    -- Rings
    , { chance = 2 
        , { item = "Ring of Spells",     chance = 1                      }
        , { item = "Ring of Vitality",   chance = 1                      }
        , { item = "Ring of Stone",      chance = 1                      }
    }
    -- Boots
    , { item = "Leather Boots",      chance = 4                      }
    , { item = "Iron Boots",        chance = 1                      }
    -- Belts
    , { item = "Belt of Protection",      chance = 1                      }
    , { item = "Belt of Slaying",        chance = 1                      }
    -- Legwear
    , { item = "Platelegs",      chance = 1                      }
    , { item = "Magic Skirt",        chance = 1                      }
    -- Gloves
    , { item = "Leather Gloves",     chance = 4                      }
    -- Amulets
    , { chance = 2 
        , { item = "Amulet of the Wall", chance = 1 }
        , { item = "Amulet of Fear", chance = 1 }
        , { item = "Amulet of Trepidation", chance = 1 }
        , { item = "Amulet of Mephitization", chance = 1 }
        , { item = "Amulet of Regeneration", chance = 1 }
        , { item = "Amulet of Fire", chance = 1 }
        , { item = "Amulet of Greater Fire", chance = 1 }
        , { item = "Amulet of Protection", chance = 1 }
    }
}

M.enchanted_items = {
    { item = "Gold",                 chance = 85, amount = {15,45}  }
--  Ludamad: Remove stones for now. Part of: Try to reduce the projectile actions for non-archer a lot.
--    { item = "Stone",              chance = 5,  amount = {3,15}  }
    -- Melee Weapons
    , { item = "Mace",               chance = 3                      }
    -- Body Armour
    , { item = "Leather Armour",     chance = 3                      }
    , { item = "Robe",               chance = 3                      }
    -- Helmets
    , { item = "Iron Helmet",        chance = 3                      }
    , { item = "Wizard's Hat",       chance = 3                      }
    -- Boot
    , { item = "Leather Boots",      chance = 1                      }
    -- Gloves
    , { item = "Leather Gloves",     chance = 1                      }
    , { item = "Arrow",              chance = 4,  amount = {5,20}  }
    , { item = "Silver Arrow",       chance = 4,  amount = {2,8}   }
    -- TODO REMOVE ABOVE REDUNDANCIES^
    -- Ranged Weapons
    -- Ludamad: Remove non-artifact bows as attainable items for now. Part of: Try to reduce the projectile actions for non-archer a lot.
    , { item = "Long Bow",           chance = 2                      }
    , { item = "Mars Bow",           chance = 1                      }
    -- Melee Weapons
    , { item = "Samurai Sabre",      chance = 1                      }
    , { item = "Magic Sword",        chance = 1                      }
    , { item = "Triple Sword",        chance = 1                      }
    , { item = "Battle Axe",        chance = 1                      }
    , { item = "Iron Scimitar",        chance = 1                      }
    -- Potions
    , { item = "Health Potion",      chance = 4                     }
    , { item = "Mana Potion",        chance = 4                     }
    -- Body Armour
    , { item = "Chainmail",          chance = 2                      }
    , { item = "Platemail",          chance = 1                      }
    , { item = "Magician's Robe",    chance = 2                      }
    , { item = "Runed Robe",         chance = 1                      }
    , { item = "Robe of Mana",         chance = 1                      }
    -- Belts
    , { item = "Belt of Protection",      chance = 1                      }
    , { item = "Belt of Slaying",        chance = 1                      }
    -- Legwear
    , { item = "Platelegs",      chance = 1                      }
    , { item = "Magic Skirt",        chance = 1                      }
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
    , { item = "Troll Boots",         chance = 1                      }
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
    -- Amulets
    , { item = "Amulet of the Wall", chance = 1 }
    , { item = "Amulet of Fear", chance = 1 }
    , { item = "Amulet of Trepidation", chance = 1 }
    , { item = "Amulet of Mephitization", chance = 1 }
    , { item = "Amulet of Regeneration", chance = 1 }
    , { item = "Amulet of Fire", chance = 1 }
    , { item = "Amulet of Greater Fire", chance = 1 }
    , { item = "Amulet of Protection", chance = 1 }
}

return M
