local TEST_INVENTORY = os.getenv("LANARTS_TESTITEMS") and {
     {item = "Mana Potion"},
     --For testing legwear:
     {item = "Platelegs", amount = 1},
     {item = "Magic Skirt", amount = 1},
     {item = "Leather Armour", amount = 1},
     {item = "Leather Armour", amount = 1},
     -- For testing belts:
     {item = "Belt of Slaying", amount = 1},
     {item = "Dank Belt", amount = 1},
     {item = "Belt of Protection", amount = 1},
     {item = "Spiked Belt", amount = 1},
     -- For testing spell amulets:
     --{item = "Amulet of Fear", amount = 1}, 
     {item = "Amulet of Trepidation", amount = 1},
     --{item = "Amulet of Regeneration", amount = 1},
     --{item = "Amulet of Fire", amount = 1},
     --{item = "Amulet of Greater Fire", amount = 1},
     --{item = "Amulet of Protection", amount = 1},
     --{item = "Amulet of Pain", amount = 1},
     --{item = "Amulet of Ringholding", amount = 1},
     { item = "Amulet of Mephitization", amount = 1 },
     { item = "Amulet of Greed", amount = 1 },
     { item = "Amulet of Healing", amount = 1 },
     {item = "Amulet of Great Pain", amount = 1},
     {item = "Amulet of Ice Form", amount = 1},
     {item = "Amulet of Light", amount = 1},
     {item = "Ring of Spells", amount = 1},
     {item = "Ring of Ethereal Armour", amount = 1},
     {item = "Ring of Vampirism", amount = 1},
     {item = "Ring of Stone", amount = 1},
     {item = "Gloves of Confusion", amount = 1},
     {item = "Gloves of Fear", amount = 1},
     {item = "Gallanthor's Ring", amount = 1},
     {item = "Magic Map", amount = 1},
     {item = "Scroll of Fear", amount = 10},
} or {}

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
        level_needed = 3 }
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
          inventory = table.tconcat({
             {item = "Mana Potion"},
          }, TEST_INVENTORY)
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
        level_needed = 1 },
      { spell = "Call Spikes",
        level_needed = 2 },
    },
    start_stats = {
        movespeed = 4,
        hp = 115,
        mp = 85,
        hpregen = 4 / 60, -- Per 60 frames
        mpregen = 2 / 60, -- Per 60 frames
        strength = 5,
        defence = 4,
        willpower = 7,
        magic = 6,
        equipment = {
          weapon = "Short Bow",
          gold = 0,
          inventory = {
             {item = "Arrow", amount = 50, equip = true},
             {item = "Health Potion", amount = 1},
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
