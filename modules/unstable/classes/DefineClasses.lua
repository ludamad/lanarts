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
    items = {"Mana Potion"},

    -- Takes 'weapon_skill', 'skill'
    on_create = function(class, args)
        local self = Utils.default_class_create(class, args)
        self.name = MAGE_NAMES_FOR_SKILL[self.magic_skill]
        self.skills[self.magic_skill] = SPECIALITY 
        self.skills[self.weapon_skill] = MINOR
        return self 
    end,

    on_init = function(self, stats)
        Utils.default_class_on_init(self, stats)
        -- TODO more items
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
    items = {"Health Potion"},

    -- Takes 'weapon_skill'
    on_create = function(class, args)
        local self = Utils.default_class_create(class, args)
        self.skills[self.weapon_skill] = SPECIALITY 
        return self 
    end,

    on_init = function(self, stats)
        Utils.default_class_on_init(self, stats)
        -- TODO more items
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