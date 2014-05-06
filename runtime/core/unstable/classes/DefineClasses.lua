local Utils = import ".ClassDefineUtils"
local StatContext = import "@StatContext"
local class_define = Utils.class_define

local MINOR = 100
local MODERATE = 300
local SPECIALITY = 700 

local MAGE_NAMES_FOR_SKILL = {
    ["Fire"] = "Pyromancer",
    ["Water"] = "Aquamancer",
    ["Dark"] = "Deathmage",
    ["Light"] = "Lifemage",
    ["Curses"] = "Hexcrafter",
    ["Enchantments"] = "Enchanter",
    ["Force"] = "Warmage",
    ["Earth"] = "Druid",
    ["Air"] = "Windsmage"
}

class_define {
    lookup_key = "Mage",
    description = "A magical master of ${magic_skill}.",
    items = {
        "Mana Potion",
        {type="Robe", bonus=1, equipped=true},
        {type="Wizard's Hat", bonus=0, equipped=true}
    },

    -- Takes 'weapon_skill', 'skill'
    on_create = function(class, args)
        local self = Utils.default_class_create(class, args)
        self.name = MAGE_NAMES_FOR_SKILL[self.magic_skill]
        self.skills[self.magic_skill] = SPECIALITY 
        self.skills[self.weapon_skill] = MINOR
        return self 
    end,

    spells = {"Minor Missile"},

    skills = {
        ["Magic"] = SPECIALITY,
        ["Magic Resistance"] = MODERATE,
        ["Magic Items"] = MODERATE,
        ["Willpower"] = MODERATE,

        ["Melee"] = MINOR,
        ["Melee Resistance"] = MINOR,
        ["Armour"] = MINOR,
        ["Fortitude"] = MINOR
    }
}

class_define {
    name = "Knight",
    description = "A disciplined, heavily armoured warrior.",
    items = { 
        "Health Potion", 
        "Hand Axe",
        {type="Chainshirt", bonus=1, equipped=true},
        {type="Horned Helmet", bonus=1, equipped=true}
    },
    spells = {"Berserk"},

    -- Takes 'weapon_skill'
    on_create = function(class, args)
        local self = Utils.default_class_create(class, args)
        self.skills[self.weapon_skill] = SPECIALITY 
        return self 
    end,

    skills = {
        ["Melee"] = SPECIALITY,
        ["Melee Resistance"] = MODERATE,
        ["Armour"] = MODERATE,
        ["Fortitude"] = MODERATE,

        ["Magic"] = MINOR,
        ["Magic Resistance"] = MINOR,
        ["Willpower"] = MINOR,
        ["Magic Items"] = MINOR
    }
}

class_define {
    name = "Archer",
    description = "A master of ranged combat. Fires swiftly from afar.",
    items = {
        {type="Leather Armour", bonus=0, equipped=true}, 
        {type="Leather Cap", bonus=0, equipped=true}, 
        {type="Short Bow", equipped=true}, 
        {type="Arrow", amount=400, equipped=true}
    },
    spells = {"Magic Arrow", "Berserk", "Minor Missile"},

    skills = {
        ["Melee"] = SPECIALITY,
        ["Ranged Fighting"] = SPECIALITY,
        ["Melee Resistance"] = MODERATE,
        ["Armour"] = MODERATE,
        ["Fortitude"] = MODERATE,

        ["Magic"] = MINOR,
        ["Magic Resistance"] = MINOR,
        ["Willpower"] = MINOR,
        ["Magic Items"] = MINOR
    }
}