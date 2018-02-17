MAGE_START_STATS = {
    movespeed: 4,
    hp: 100,
    mp: 120,
    hpregen: 2 / 60, -- Per 60 frames
    mpregen: 4 / 60, -- Per 60 frames 
    strength: 4,
    defence: 4,
    willpower: 7,
    magic: 7,
    equipment: {
        gold: 0
        inventory: {
            {item: "Mana Potion"},
            {item: "Stripe Boots", equipped: true}
        }
    }
}
MAGE_GAIN_STATS = {
    hp: 15,
    mp: 20,
    hpregen: 0.5 / 60, -- Per 60 frames
    mpregen: 1 / 60, -- Per 60 frames
    magic: 2,
    strength: 1,
    defence: 1,
    willpower: 2
}

Data.class_create {
    name: "Red Mage",
    sprites: (for i=1,2 do "spr_classes.red_mage#{i}")
    available_spells: {
        {spell: "Minor Missile", level_needed: 1},
        {spell: "Fire Bolt", level_needed: 1},
        {spell: "Ring of Flames", level_needed: 2},
        {spell: "Inner Fire", level_needed: 3},
    },
    start_stats: MAGE_START_STATS
    gain_per_level: MAGE_GAIN_STATS
}

Data.class_create {
    name: "Blue Mage",
    sprites: (for i=1,2 do "spr_classes.blue_mage#{i}")
    available_spells: {
        {spell: "Minor Missile", level_needed: 1},
        {spell: "Water Bolt", level_needed: 1},
    },
    start_stats: MAGE_START_STATS
    gain_per_level: MAGE_GAIN_STATS
}

Data.class_create {
    name: "White Mage",
    sprites: (for i=1,2 do "spr_classes.white_mage#{i}")
    available_spells: {
        {spell: "Minor Missile", level_needed: 1},
        {spell: "Energy Spear", level_needed: 1},
        {spell: "Chain Lightning", level_needed: 2},
        {spell: "Tornado Storm", level_needed: 3},
    },
    start_stats: MAGE_START_STATS
    gain_per_level: MAGE_GAIN_STATS
}

--Data.class_create {
--    name: "Scoundrel",
--    sprites: {"fighter", "fighter2", "spr_enemies.humanoid.dwarf"},
--    available_spells: {
--        { spell: "Berserk", level_needed: 1 },
--        { spell: "Power Strike", level_needed: 2 },
--        { spell: "Dash Attack", level_needed: 3 },
--    },
--    start_stats: {
--        movespeed: 4,
--        hp: 135,
--        mp: 60,
--        hpregen: 4 / 60, -- Per 60 frames
--        mpregen: 2 / 60, -- Per 60 frames
--        strength: 7,
--        defence: 7,
--        willpower: 4,
--        magic: 4,
--        equipment: {
--          weapon: "Mace",
--          gold: 0,
--          inventory: {{item: "Health Potion"}},
--        }
--    },
--    gain_per_level: {
--        hp: 20,
--        mp: 15,
--        hpregen: 1 / 60, -- Per 60 frames
--        mpregen: 0.5 / 60, -- Per 60 frames
--        magic: 1,
--        willpower: 1,
--        defence: 2,
--        strength: 2
--    }
--}

Data.class_create {
    name: "Fighter",
    sprites: {"fighter", "fighter2", "spr_enemies.humanoid.dwarf"},
    available_spells: {
        { spell: "Berserk", level_needed: 1 },
        { spell: "Power Strike", level_needed: 2 },
        { spell: "Dash Attack", level_needed: 3 },
    },
    start_stats: {
        movespeed: 4,
        hp: 135,
        mp: 60,
        hpregen: 4 / 60, -- Per 60 frames
        mpregen: 2 / 60, -- Per 60 frames
        strength: 7,
        defence: 7,
        willpower: 4,
        magic: 4,
        equipment: {
          weapon: "Mace",
          gold: 0,
          inventory: {{item: "Health Potion"}},
        }
    },
    gain_per_level: {
        hp: 20,
        mp: 15,
        hpregen: 1 / 60, -- Per 60 frames
        mpregen: 0.5 / 60, -- Per 60 frames
        magic: 1,
        willpower: 1,
        defence: 2,
        strength: 2
    }
}

Data.class_create {
    name: "Rogue",
    sprites: (for i=1,2 do "spr_classes.rogue#{i}"),
    available_spells: {
        { spell: "Dash Attack", level_needed: 1 },
--        { spell: "Hide in Sight", level_needed: 1 },
    },
    start_stats: {
        movespeed: 4,
        hp: 100,
        mp: 60,
        hpregen: 6 / 60, -- Per 60 frames
        mpregen: 2 / 60, -- Per 60 frames
        strength: 7,
        defence: 7,
        willpower: 4,
        magic: 4,
        equipment: {
          weapon: "Dagger",
          gold: 0,
          inventory: {{item: "Health Potion"}},
        }
    },
    gain_per_level: {
        hp: 15,
        mp: 15,
        hpregen: 1.5 / 60, -- Per 60 frames
        mpregen: 0.5 / 60, -- Per 60 frames
        magic: 1,
        willpower: 1,
        defence: 2,
        strength: 2
    }
}
Data.class_create {
    name: "Necromancer",
    sprites: {"spr_enemies.humanoid.necromancer"},
    available_spells: {
      { spell: "Pain",
        level_needed: 1 },
      { spell: "Baleful Regeneration",
        level_needed: 1 },
      { spell: "Summon Dark Aspect",
        level_needed: 2 },
      { spell: "Greater Pain",
        level_needed: 3 },
      -- { spell: "",
      --   level_needed: 2 },
      -- { spell: "Ludaze",
      --   level_needed: 3 },
      -- { spell: "Call Spikes",
      --   level_needed: 4 },
    },
    start_stats: {
        movespeed: 4,
        hp: 115,
        mp: 50,
        hpregen: 3 / 60, -- Per 60 frames
        mpregen: 0 / 60, -- Per 60 frames
        strength: 0,
        defence: 4,
        willpower: 7,
        magic: 7,
        equipment: {
          weapon: "Dagger",
          gold: 0,
          inventory: {{item: "Mana Potion", amount: 1}}
        }
    },
    gain_per_level: {
        hp: 15,
        mp: 5,
        hpregen: 1 / 60, -- Per 60 frames
        mpregen: 0 / 60, -- Per 60 frames
        magic: 2,
        willpower: 1,
        defence: 2,
        strength: 1
    }
}
