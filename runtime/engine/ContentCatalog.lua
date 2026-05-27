local M = {}

M.classes = {
    {
        id = "Fighter",
        label = "Fighter",
        role = "close melee",
        range = "close",
        balance_focus = "Checks whether melee gear and enemy contact damage are too forgiving.",
    },
    {
        id = "Necromancer",
        label = "Necromancer",
        role = "regen attrition",
        range = "close",
        balance_focus = "Uses Baleful Regeneration while taking damage; gross incoming damage is misleading.",
    },
    {
        id = "Pyrocaster",
        label = "Pyrocaster",
        role = "ranged red magic",
        range = "far",
        balance_focus = "Checks fire conversion, cast cadence, and enemy red resistance pressure.",
    },
    {
        id = "Stormcaller",
        label = "Stormcaller",
        role = "ranged white magic",
        range = "far",
        balance_focus = "Primary comparison point for ranged caster overperformance.",
    },
    {
        id = "Templar",
        label = "Templar",
        role = "defensive melee",
        range = "close",
        balance_focus = "Useful for separating melee survivability from Fighter burst.",
    },
    {
        id = "Rogue",
        label = "Rogue",
        role = "mobile melee",
        range = "close",
        balance_focus = "Useful for testing contact windows and pursuit pressure.",
    },
}

M.levels = {
    {value = 1, label = "1", phase = "opening"},
    {value = 2, label = "2", phase = "early"},
    {value = 3, label = "3", phase = "mid"},
    {value = 4, label = "4", phase = "mid"},
    {value = 5, label = "5 late", phase = "late"},
    {value = 6, label = "6", phase = "post-late"},
    {value = 7, label = "7", phase = "post-late"},
    {value = 8, label = "8 high", phase = "very high"},
}

M.loadout_presets = {
    none = {
        label = "None",
        description = "Class starting inventory only.",
    },
    gragh = {
        label = "Gragh",
        description = "Focused boss-test gear around the Gragh progression point.",
        default = {
            "Health Potion*2",
            "Mana Potion",
        },
        Fighter = {
            "Iron Broadsword",
            "Chainmail",
            "Leather Gloves",
            "Iron Helmet",
            "Leather Boots",
            "Belt of Protection",
            "Ring of Vitality",
        },
        Necromancer = {
            "Apprentice Staff",
            "Robe",
            "Wizard's Ring",
            "Amulet of Great Pain",
        },
        Pyrocaster = {
            "Apprentice Staff",
            "Red Robe",
            "Wizard's Hat",
            "Ring of Spells",
            "Amulet of Greater Fire",
        },
        Stormcaller = {
            "Wizard's Staff",
            "White Robe",
            "Wizard's Hat",
            "Ring of Spells",
            "Amulet of Light",
        },
    },
    late = {
        label = "Late",
        description = "Level 5 late-game comparison gear.",
        default = {
            "Health Potion*3",
            "Mana Potion*2",
        },
        Fighter = {
            "Gragh's Club",
            "Red Dragonplate",
            "Steel Gloves",
            "Iron Helmet",
            "Platelegs",
            "Jack Boots",
            "Belt of Slaying",
            "Ring of Vampirism",
            "Ring of Vitality",
            "Amulet of Healing",
        },
        Templar = {
            "Mace of Light",
            "Crystalline Plate",
            "Steel Gloves",
            "Iron Helmet",
            "Platelegs",
            "Jack Boots",
            "Belt of Protection",
            "Ring of Stone",
            "Ring of Vitality",
            "Amulet of Fortification",
        },
        Rogue = {
            "Iron Scimitar",
            "Leather Armour",
            "Leather Gloves",
            "Leather Boots",
            "Dank Belt",
            "Ring of Vampirism",
            "Hydra Ring",
            "Amulet of Fear",
        },
        Necromancer = {
            "Runed Robe",
            "Wizard's Ring",
            "Hydra Ring",
            "Moonstone Ring",
            "Wishful Belt",
            "Amulet of Great Pain",
        },
        Pyrocaster = {
            "Gallanthor's Crutch",
            "Red Robe",
            "Runed Gloves",
            "Wizard's Hat",
            "Magic Skirt",
            "Wishful Belt",
            "Magician's Ring",
            "Ring of Spells",
            "Amulet of Greater Fire",
        },
        Stormcaller = {
            "Wizard's Staff",
            "Robe of Vitality",
            "Runed Gloves",
            "Wizard's Hat",
            "Magic Skirt",
            "Wishful Belt",
            "Magician's Ring",
            "Ring of Spells",
            "Amulet of Light",
        },
    },
}

M.loadout_order = {"none", "gragh", "late"}

M.maps = {
    {
        id = "",
        label = "Campaign World",
        generator = "map_descs.World",
        phase = "campaign",
        preview_mode = "world",
        density = 0.30,
        palette = {
            floor = "#31553c",
            floor2 = "#3a5f48",
            wall = "#1d2922",
            accent = "#25465a",
        },
    },
    {
        id = "map_descs.Overworld",
        label = "Overworld variety",
        generator = "map_descs.Overworld",
        phase = "opening",
        preview_mode = "overworld",
        density = 0.22,
        palette = {
            floor = "#31553c",
            floor2 = "#416447",
            wall = "#1d2922",
            accent = "#25465a",
        },
    },
    {
        id = "map_descs.GraghsLair",
        label = "Gragh's Lair",
        generator = "map_descs.GraghsLair",
        phase = "late",
        preview_mode = "lair",
        density = 0.36,
        palette = {
            floor = "#514235",
            floor2 = "#66503d",
            wall = "#27231f",
            accent = "#b6675a",
        },
    },
    {
        id = "map_descs.Hell",
        label = "Hell",
        generator = "map_descs.Hell",
        phase = "late",
        preview_mode = "hell",
        density = 0.34,
        palette = {
            floor = "#6d3130",
            floor2 = "#8c3e36",
            wall = "#281a1a",
            accent = "#d7c37a",
        },
    },
    {
        id = "map_descs.HiveDepths",
        label = "Hive Depths",
        generator = "map_descs.HiveDepths",
        phase = "late",
        preview_mode = "hive",
        density = 0.30,
        palette = {
            floor = "#5c6042",
            floor2 = "#777344",
            wall = "#282a1f",
            accent = "#d2b85a",
        },
    },
    {
        id = "map_descs.PixullochiaDepths",
        label = "Pixullochia Depths",
        generator = "map_descs.PixullochiaDepths",
        phase = "late",
        preview_mode = "pixullochia",
        density = 0.33,
        palette = {
            floor = "#3e4f63",
            floor2 = "#4f667a",
            wall = "#20262e",
            accent = "#8dbb75",
        },
    },
    {
        id = "map_descs.Crypt",
        label = "Crypt",
        generator = "map_descs.Crypt",
        phase = "mid",
        preview_mode = "crypt",
        density = 0.35,
        palette = {
            floor = "#4b4d51",
            floor2 = "#63656a",
            wall = "#1d2024",
            accent = "#8d789d",
        },
    },
    {
        id = "map_descs.Underdungeon",
        label = "Underdungeon",
        generator = "map_descs.Underdungeon",
        phase = "late",
        preview_mode = "underdungeon",
        density = 0.38,
        palette = {
            floor = "#3d4650",
            floor2 = "#50606b",
            wall = "#171d22",
            accent = "#b69058",
        },
    },
}

M.encounters = {
    {
        id = "",
        label = "No forced encounter",
        kind = "map",
        phase = "free play",
        description = "Use the selected map generator without forcing a duel.",
        monsters = {},
    },
    {
        id = "giant-chicken-duel",
        label = "Giant Chicken duel",
        kind = "duel",
        phase = "early micro",
        description = "Dash bursts and knockback timing in a clean arena.",
        monsters = {"Giant Chicken"},
        arena = {
            room_size = {16, 16},
        },
    },
    {
        id = "mummoner-duel",
        label = "Mummoner duel",
        kind = "duel",
        phase = "summon pressure",
        description = "A 1v1 shell that turns into timed add management.",
        features = {"add timing", "targeted pulse"},
        monsters = {"Mummoner"},
        arena = {
            room_size = {17, 17},
        },
        package = {
            primary = "Mummoner",
            pulse = {
                interval = 165,
                offset = 80,
                windup = 46,
                active = 20,
                radius = 58,
                damage = 3.0,
                power = 20,
            },
        },
    },
    {
        id = "purple-dragon-package",
        label = "Purple Dragon package",
        kind = "boss package",
        phase = "late micro",
        description = "Bounce projectile pressure plus periodic Fire Bat summons.",
        features = {"rotating orbs", "targeted AOE rhythm", "Fire Bat adds"},
        monsters = {"Purple Dragon"},
        arena = {
            room_size = {18, 18},
        },
        package = {
            primary = "Purple Dragon",
            orbit = {
                interval = 310,
                offset = 45,
                count = 4,
                radius = 126,
                speed = 0.042,
                duration = 260,
                hit_radius = 18,
                damage = 3.5,
                power = 26,
            },
            pulse = {
                interval = 145,
                offset = 95,
                windup = 44,
                active = 22,
                radius = 66,
                damage = 4.0,
                power = 28,
            },
            summon = {
                interval = 390,
                offset = 180,
                monsters = {"Fire Bat"},
                count = 2,
                radius = 145,
                max_alive = 4,
            },
        },
    },
    {
        id = "red-dragon-duel",
        label = "Red Dragon duel",
        kind = "boss duel",
        phase = "late micro",
        description = "Sustained projectile dodging and burn-pressure comparison.",
        features = {"projectile pressure", "targeted AOE rhythm"},
        monsters = {"Red Dragon"},
        arena = {
            room_size = {18, 18},
        },
        package = {
            primary = "Red Dragon",
            pulse = {
                interval = 135,
                offset = 76,
                windup = 42,
                active = 22,
                radius = 62,
                damage = 4.0,
                power = 30,
            },
        },
    },
    {
        id = "gragh-duel",
        label = "Gragh duel",
        kind = "boss duel",
        phase = "late micro",
        description = "Charge, pull, taunt, and melee rescue timing in isolation.",
        features = {"melee pursuit", "rotating orbs", "short AOE windows"},
        monsters = {"Gragh"},
        arena = {
            room_size = {17, 17},
        },
        package = {
            primary = "Gragh",
            orbit = {
                interval = 260,
                offset = 35,
                count = 3,
                radius = 96,
                speed = 0.052,
                duration = 210,
                hit_radius = 17,
                damage = 3.2,
                power = 24,
            },
            pulse = {
                interval = 125,
                offset = 85,
                windup = 38,
                active = 18,
                radius = 54,
                damage = 3.2,
                power = 24,
            },
        },
    },
    {
        id = "executioner-duel",
        label = "Executioner duel",
        kind = "duel",
        phase = "late melee",
        description = "Close-quarters blade pressure against melee and regen classes.",
        monsters = {"Executioner"},
        arena = {
            room_size = {15, 15},
        },
    },
    {
        id = "hell-pressure-pair",
        label = "Hell pressure pair",
        kind = "package",
        phase = "late enemy mix",
        description = "A Mana Sapper support check with an Executioner bodyguard.",
        features = {"support pressure", "melee bodyguard", "targeted pulse"},
        monsters = {"Mana Sapper", "Executioner"},
        arena = {
            room_size = {17, 17},
        },
        package = {
            primary = "Executioner",
            pulse = {
                interval = 150,
                offset = 70,
                windup = 42,
                active = 20,
                radius = 58,
                damage = 3.5,
                power = 25,
            },
        },
    },
}

M.resources = {
    {
        id = "early-overworld-pressure",
        name = "Early overworld pressure",
        meta = "L1, no loadout, Overworld",
        className = "Necromancer",
        level = 1,
        preset = "none",
        mapDesc = "map_descs.Overworld",
        seed = 17300,
        goal = "opening enemy pressure",
    },
    {
        id = "necromancer-regen-abuse",
        name = "Necromancer regen abuse",
        meta = "L5, Gragh loadout, Gragh arena",
        className = "Necromancer",
        level = 5,
        preset = "gragh",
        mapDesc = "map_descs.GraghsLair",
        seed = 12345683,
        goal = "regen-adjusted boss pressure",
    },
    {
        id = "fighter-late-sanity",
        name = "Fighter late-game sanity",
        meta = "L5, late loadout, Gragh arena",
        className = "Fighter",
        level = 5,
        preset = "late",
        mapDesc = "map_descs.GraghsLair",
        seed = 12345685,
        goal = "melee overperformance check",
    },
    {
        id = "pyro-conversion-check",
        name = "Pyro conversion check",
        meta = "L5, late loadout, Hell",
        className = "Pyrocaster",
        level = 5,
        preset = "late",
        mapDesc = "map_descs.Hell",
        seed = 9500,
        goal = "caster damage conversion",
    },
    {
        id = "stormcaller-comparison",
        name = "Stormcaller comparison",
        meta = "L5, late loadout, Hell",
        className = "Stormcaller",
        level = 5,
        preset = "late",
        mapDesc = "map_descs.Hell",
        seed = 9500,
        goal = "ranged caster baseline",
    },
    {
        id = "content-lab-purple-package",
        name = "Purple Dragon package",
        meta = "L5, late loadout, duel arena",
        className = "Stormcaller",
        level = 5,
        preset = "late",
        mapDesc = "",
        encounter = "purple-dragon-package",
        seed = 22400,
        goal = "prototype full-package 1v1",
    },
    {
        id = "content-lab-mummoner-duel",
        name = "Mummoner add timing",
        meta = "L3, no loadout, duel arena",
        className = "Fighter",
        level = 3,
        preset = "none",
        mapDesc = "",
        encounter = "mummoner-duel",
        seed = 22410,
        goal = "summon-pressure micro",
    },
}

function M.loadout_preset_names()
    local names = {}
    local seen = {}
    for _, name in ipairs(M.loadout_order) do
        if M.loadout_presets[name] then
            names[#names + 1] = name
            seen[name] = true
        end
    end
    local extras = {}
    for name, _ in pairs(M.loadout_presets) do
        if not seen[name] then
            extras[#extras + 1] = name
        end
    end
    table.sort(extras)
    for _, name in ipairs(extras) do
        names[#names + 1] = name
    end
    return names
end

function M.catalog()
    return {
        schema = 1,
        classes = M.classes,
        levels = M.levels,
        loadouts = M.loadout_presets,
        loadout_names = M.loadout_preset_names(),
        maps = M.maps,
        encounters = M.encounters,
        resources = M.resources,
    }
end

return M
