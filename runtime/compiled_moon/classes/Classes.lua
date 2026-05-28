local MAGE_START_STATS = {
  movespeed = 4,
  hp = 100,
  mp = 120,
  hpregen = 2 / 60,
  mpregen = 4 / 60,
  strength = 4,
  defence = 4,
  willpower = 7,
  magic = 7,
  equipment = {
    gold = 0,
    weapon = "Apprentice Staff",
    inventory = {
      {
        item = "Mana Potion"
      },
      {
        item = "Health Potion"
      }
    }
  }
}
local MAGE_GAIN_STATS = {
  hp = 15,
  mp = 20,
  hpregen = 0.5 / 60,
  mpregen = 1 / 60,
  magic = 2,
  strength = 1,
  defence = 1,
  willpower = 2
}
local STORMCALLER_START_STATS = table.merge(MAGE_START_STATS, {
  hp = 130,
  defence = 6
})
Data.class_create({
  name = "Pyrocaster",
  sprites = ((function()
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, 2 do
      _accum_0[_len_0] = "spr_classes.red_mage" .. tostring(i)
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()),
  available_spells = {
    {
      spell = "Fire Bolt",
      level_needed = 1
    },
    {
      spell = "Ring of Flames",
      level_needed = 2
    },
    {
      spell = "Dragonform",
      level_needed = 3
    }
  },
  start_stats = MAGE_START_STATS,
  gain_per_level = MAGE_GAIN_STATS
})
Data.class_create({
  name = "Blue Mage",
  sprites = ((function()
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, 2 do
      _accum_0[_len_0] = "spr_classes.blue_mage" .. tostring(i)
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()),
  available_spells = {
    {
      spell = "Water Bolt",
      level_needed = 1
    }
  },
  start_stats = MAGE_START_STATS,
  gain_per_level = MAGE_GAIN_STATS
})
Data.class_create({
  name = "Stormcaller",
  sprites = ((function()
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, 2 do
      _accum_0[_len_0] = "spr_classes.white_mage" .. tostring(i)
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()),
  available_spells = {
    {
      spell = "Energy Spear",
      level_needed = 1
    },
    {
      spell = "Chain Lightning",
      level_needed = 2
    },
    {
      spell = "Tornado Storm",
      level_needed = 3
    }
  },
  start_stats = STORMCALLER_START_STATS,
  gain_per_level = MAGE_GAIN_STATS
})
Data.class_create({
  name = "Templar",
  sprites = {
    "spr_classes.templar_stand"
  },
  available_spells = {
    {
      spell = "Power Strike",
      level_needed = 1
    },
    {
      spell = "Dash Attack",
      level_needed = 2
    }
  },
  start_stats = {
    movespeed = 4,
    hp = 135,
    mp = 60,
    hpregen = 4 / 60,
    mpregen = 2 / 60,
    strength = 7,
    defence = 7,
    willpower = 4,
    magic = 4,
    equipment = {
      weapon = "Short Sword",
      gold = 0,
      inventory = {
        {
          item = "Health Potion"
        }
      }
    }
  },
  gain_per_level = {
    hp = 20,
    mp = 15,
    hpregen = 1 / 60,
    mpregen = 0.5 / 60,
    magic = 1,
    willpower = 1,
    defence = 2,
    strength = 2
  }
})
Data.class_create({
  name = "Fighter",
  sprites = {
    "fighter",
    "fighter2",
    "spr_enemies.humanoid.dwarf"
  },
  available_spells = {
    {
      spell = "Berserk",
      level_needed = 1
    },
    {
      spell = "Dash Attack",
      level_needed = 2
    },
    {
      spell = "Power Strike",
      level_needed = 3
    }
  },
  start_stats = {
    movespeed = 4,
    hp = 135,
    mp = 60,
    hpregen = 4 / 60,
    mpregen = 2 / 60,
    strength = 6,
    defence = 7,
    willpower = 4,
    magic = 4,
    equipment = {
      weapon = "Mace",
      gold = 0,
      inventory = {
        {
          item = "Health Potion"
        }
      }
    }
  },
  gain_per_level = {
    hp = 20,
    mp = 15,
    hpregen = 1 / 60,
    mpregen = 0.5 / 60,
    magic = 1,
    willpower = 1,
    defence = 2,
    strength = 2
  }
})
Data.class_create({
  name = "Rogue",
  sprites = ((function()
    local _accum_0 = { }
    local _len_0 = 1
    for i = 1, 2 do
      _accum_0[_len_0] = "spr_classes.rogue" .. tostring(i)
      _len_0 = _len_0 + 1
    end
    return _accum_0
  end)()),
  available_spells = {
    {
      spell = "Dash Attack",
      level_needed = 1
    }
  },
  start_stats = {
    movespeed = 4,
    hp = 100,
    mp = 60,
    hpregen = 6 / 60,
    mpregen = 2 / 60,
    strength = 7,
    defence = 7,
    willpower = 4,
    magic = 4,
    equipment = {
      weapon = "Dagger",
      gold = 0,
      inventory = {
        {
          item = "Health Potion"
        }
      }
    }
  },
  gain_per_level = {
    hp = 15,
    mp = 15,
    hpregen = 1.5 / 60,
    mpregen = 0.5 / 60,
    magic = 1,
    willpower = 1,
    defence = 2,
    strength = 2
  }
})
return Data.class_create({
  name = "Necromancer",
  sprites = {
    "spr_enemies.humanoid.necromancer"
  },
  available_spells = {
    {
      spell = "Baleful Regeneration",
      level_needed = 1
    },
    {
      spell = "Summon Dark Aspect",
      level_needed = 2
    },
    {
      spell = "Skeleton Army",
      level_needed = 3
    }
  },
  start_stats = {
    movespeed = 4,
    hp = 115,
    mp = 50,
    hpregen = 3 / 60,
    mpregen = 0 / 60,
    strength = 0,
    defence = 4,
    willpower = 7,
    magic = 7,
    equipment = {
      gold = 0,
      inventory = {
        {
          item = "Mana Potion",
          amount = 1
        }
      }
    }
  },
  gain_per_level = {
    hp = 15,
    mp = 5,
    hpregen = 1 / 60,
    mpregen = 0 / 60,
    magic = 2,
    willpower = 1,
    defence = 2,
    strength = 1
  }
})
