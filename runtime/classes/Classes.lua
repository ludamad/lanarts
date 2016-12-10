Data.class_create {
    name = "Mage",
    sprites = {"wizard", "wizard2"},
    available_spells = {
      { spell = "Minor Missile",
    	level_needed = 1 },
      { spell = "Magic Blast",
        level_needed = 1 },
      { spell = "Fire Bolt",
        level_needed = 2 } 
    },
    start_stats = {
        movespeed = 4,
        hp = 100,
        mp = 120,
        hpregen = 0.040,
        mpregen = 0.076,
        strength = 4,
        defence = 4,
        willpower = 7,
        magic = 7,
        equipment = {
          gold = 0,
          inventory = {
             {item = "Mana Potion"},
             {item = "Ring of Spells"},
             {item = "Gallanthor's Ring"} 
          }
        }
    },
    gain_per_level = {
        hp = 15,
        mp = 20,
        hpregen = 0.007,
        mpregen = 0.010,
        magic = 1,
        strength = 1,
        defence = 1,
        willpower = 1
    }
}

Data.class_create {
    name = "Fighter",
    sprites = {"fighter", "fighter2"},
    available_spells = {
      { spell = "Berserk",
        level_needed = 1 },
      { spell = "Power Strike",
        level_needed = 1 }
    },
    start_stats = {
        movespeed = 4,
        hp = 135,
        mp = 50,
        hpregen = 0.065,
        mpregen = 0.051,
        strength = 7,
        defence = 7,
        willpower = 4,
        magic = 4,
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
        strength = 1,
        defence = 1,
        willpower = 1
    }
}

Data.class_create {
    name = "Archer",
    sprites = {"archer"},
    available_spells = {
      { spell = "Magic Arrow",
        level_needed = 1 },
      { spell = "Expedite",
        level_needed = 2 }
    },
    start_stats = {
        movespeed = 4,
        hp = 100,
        mp = 85,
        hpregen = 0.041,
        mpregen = 0.06,
        strength = 6,
        defence = 5,
        willpower = 6,
        magic = 5,
        equipment = {
          weapon = "Short Bow",
          gold = 0,
          projectile = { item = "Arrow", amount = 60 },
          inventory = {
             {item = "Dagger"} 
          }
        }
    },
    gain_per_level = {
        hp = 10,
        mp = 15,
        hpregen = 0.011,
        mpregen = 0.010,
        magic = 1,
        strength = 1,
        defence = 1,
        willpower = 1
    }
}
