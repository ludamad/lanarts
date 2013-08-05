
local M = {} -- Submodule

M.basic_items = {
    { item = "Gold",                 chance = 125, amount = {5,15}  }
    -- Projectiles
    , { item = "Arrow",              chance = 15,  amount = {5,20}  }
    , { item = "Stone",              chance = 15,  amount = {3,15}  }
    -- Ranged Weapons
    , { item = "Short Bow",          chance = 10                     }
    -- Melee Weapons
    , { item = "Dagger",             chance = 10                     }
    , { item = "Short Sword",        chance = 10                     }
    , { item = "Mace",               chance = 5                      }
    -- Scrolls
    , { item = "Haste Scroll",       chance = 3                      }
    , { item = "Strength Scroll",    chance = 2                      }
    , { item = "Scroll of Escape",   chance = 3                      }
    , { item = "Magic Scroll",       chance = 2                      }
    , { item = "Defence Scroll",     chance = 2                      }
    , { item = "Will Scroll",        chance = 2                      }
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

return M