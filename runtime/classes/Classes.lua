Data.class_create {
    name = "Mage",
    sprites = {"wizard", "wizard2"},
    available_spells = {
      { spell = "Minor Missile",
    	level_needed = 1 },
      { spell = "Magic Blast",
        level_needed = 2 },
      { spell = "Fire Bolt",
        level_needed = 1 },
      { spell = "Mephitize",
        level_needed = 3 },
      { spell = "Wallanthor",
        level_needed = 4 },
    },
    start_stats = {
        movespeed = 4,
        hp = 100,
        mp = 120,
        hpregen = 2 / 60, -- Per 60 frames
        mpregen = 4 / 60, -- Per 60 frames 
        strength = 4,
        defence = 4,
        willpower = 7,
        magic = 7,
        equipment = {
          gold = 0,
          inventory = {
             {item = "Mana Potion"},
             --For testing legwear:
             {item = "Platelegs", amount = 1},
             {item = "Magic Skirt", amount = 1},
             -- For testing belts:
             {item = "Belt of Slaying", amount = 1},
             {item = "Belt of Protection", amount = 1},
             -- For testing spell amulets:
             {item = "Amulet of Fear", amount = 1}, 
             {item = "Amulet of Trepidation", amount = 1},
             {item = "Amulet of Regeneration", amount = 1},
             {item = "Amulet of Fire", amount = 1},
             {item = "Amulet of Greater Fire", amount = 1},
             {item = "Amulet of Protection", amount = 1},
             {item = "Amulet of Pain", amount = 1},
             {item = "Amulet of Great Pain", amount = 1},
          }
        }
    },
    gain_per_level = {
        hp = 15,
        mp = 20,
        hpregen = 0.5 / 60, -- Per 60 frames
        mpregen = 0.65 / 60, -- Per 60 frames
        magic = 1,
        strength = 1,
  --      defence = 1,
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
        level_needed = 2 },
    },
    start_stats = {
        movespeed = 4,
        hp = 135,
        mp = 50,
        hpregen = 4 / 60, -- Per 60 frames
        mpregen = 2 / 60, -- Per 60 frames
        strength = 7,
        defence = 7,
        willpower = 4,
        magic = 4,
        equipment = {
          weapon = "Dagger",
          gold = 0,
          inventory = {
             {item = "Health Potion", amount = 1},
             -- For testing spell amulets:
             -- {item = "Amulet of the Wall", amount = 1},
             -- {item = "Amulet of Fear", amount = 1}, 
             -- {item = "Amulet of Trepidation", amount = 1},
             -- {item = "Amulet of Regeneration", amount = 1},
             -- {item = "Amulet of Fire", amount = 1},
             -- {item = "Amulet of Greater Fire", amount = 1},
             -- {item = "Amulet of Protection", amount = 1},
          }
        }
    },
    gain_per_level = {
        hp = 25,
        mp = 10,
        hpregen = 0.65 / 60, -- Per 60 frames
        mpregen = 0.5 / 60, -- Per 60 frames
 --       magic = 1,
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
        hp = 115,
        mp = 85,
        hpregen = 4 / 60, -- Per 60 frames
        mpregen = 2 / 60, -- Per 60 frames
        strength = 7,
        defence = 4,
        willpower = 7,
        magic = 4,
        equipment = {
          weapon = "Short Bow",
          gold = 0,
          inventory = {
             {item = "Arrow", amount = 50} 
          }
        }
    },
    gain_per_level = {
        hp = 25,
        mp = 10,
        hpregen = 0.65 / 60, -- Per 60 frames
        mpregen = 0.5 / 60, -- Per 60 frames
        magic = 1,
        strength = 1,
        defence = 1,
--        willpower = 1
    }
}
