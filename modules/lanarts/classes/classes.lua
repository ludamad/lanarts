Data.class_create {
    name = "Mage",
    sprites = {"wizard", "wizard2"},
    available_spells = {
      { spell = "Fire Bolt",
        level_needed = 1 }, 
      { spell = "Minor Missile",
    	level_needed = 1 },
      { spell = "Blink",
        level_needed = 2 }, 
      { spell = "Poison Cloud",
        level_needed = 2 }, 
      { spell = "Magic Blast",
        level_needed = 3 }
    },
    start_stats = {
        movespeed = 4,
        hp = 85,
        mp = 100,
        hpregen = 0.033,
        mpregen = 0.066,
        strength = 3,
        defence = 3,
        willpower = 5,
        magic = 5,
        equipment = {
          gold = 0,
          inventory = {
             {item = "Mana Potion"} 
          }
        }
    },
    gain_per_level = {
        hp = 15,
        mp = 20,
        hpregen = 0.007,
        mpregen = 0.010,
        magic = 2,
        strength = 1,
        defence = 1,
        willpower = 2
    }
}

Data.class_create {
    name = "Fighter",
    sprites = {"fighter", "fighter2"},
    available_spells = {
      { spell = "Berserk",
        level_needed = 1 },
      { spell = "Power Strike",
        level_needed = 3 }
    },
    start_stats = {
        movespeed = 4,
        hp = 110,
        mp = 40,
        hpregen = 0.055,
        mpregen = 0.044,
        strength = 6,
        defence = 6,
        willpower = 3,
        magic = 2,
        equipment = {
          weapon = "Dagger",
          gold = 0,
          inventory = {
             {item = "Stone", amount = 10} 
          }
        }
    },
    gain_per_level = {
        hp = 25,
        mp = 10,
        hpregen = 0.010,
        mpregen = 0.007,
        magic = 1,
        strength = 2,
        defence = 2,
        willpower = 1
    }
}

Data.class_create {
    name = "Archer",
    sprites = {"archer"},
    available_spells = {
      { spell = "Expedite",
        level_needed = 1 },
      { spell = "Magic Arrow",
        level_needed = 2 }
    },
    start_stats = {
        movespeed = 4,
        hp = 85,
        mp = 70,
        hpregen = 0.05,
        mpregen = 0.05,
        strength = 4,
        defence = 4,
        willpower = 4,
        magic = 4,
        equipment = {
          weapon = "Short Bow",
          gold = 0,
          projectile = { item = "Arrow", amount = 40 },
          inventory = {
             {item = "Dagger"} 
          }
        }
    },
    gain_per_level = {
        hp = 15,
        mp = 20,
        hpregen = 0.010,
        mpregen = 0.010,
        magic = 2,
        strength = 1,
        defence = 1,
        willpower = 2
    }
}
