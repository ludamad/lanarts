local Utils = import ".ClassDefineUtils"
local StatContext = import "@StatContext"
local class_define = Utils.class_define

local MINOR = 100
local MODERATE = 300
local SPECIALITY = 700 

class_define {
    lookup_key = "Elementalist",
    name = "${skill} ${lookup_key}",
    description = "A magical master of %{skill}.",
    items = {"Mana Potion"},

    on_create = function(class, args)
        local self = Utils.default_class_create(class, args)
        self.skills[self.skill] = SPECIALITY 
        self.skills[self.weapon_skill] = MINOR
        return self 
    end,

    on_init = function(self, stats)
        Utils.default_class_on_init(self, stats)
    end,

    skills = {
        ["Melee"] = MINOR,
        ["Melee Resistance"] = MINOR,
        ["Magic Resistance"] = MODERATE,
        ["Magic Items"] = MODERATE,
        ["Willpower"] = MODERATE,
        ["Fortitude"] = MINOR,
        ["Armour"] = MINOR
    }
}
