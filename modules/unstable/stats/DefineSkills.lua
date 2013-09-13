local SkillType = import "@SkillType"
local StatContext = import "@StatContext"
local Apts = import "@stats.AptitudeTypes"
local ExperienceCalculation = import "@stats.ExperienceCalculation"

local M = {} -- Submodule

local function resolve_aptitude_bonuses(a)
    if type(a) == "string" then
        return {[a] = {dup(1,4)}}
    end
    if #a > 0 then
        local temp = {}
        for i=1,#a,2 do
            temp[a[i]] = a[i+1]
        end
        a = temp
    end
    for k,v in pairs(a) do
        if type(v) == "number" then
            a[k] = {dup(v,4)}
        end
    end
    return a
end

local function on_calculate(skill_slot, user)
    for type, apt in pairs(skill_slot.type.aptitudes) do
        StatContext.add_all_aptitudes(user, type, vector_scale(apt, skill_slot.level))
    end
end

local function xp2level(skill_slot, xp)
    return ExperienceCalculation.skill_level_from_cost(skill_slot.type.cost_multiplier, xp)
end

local function on_spend_experience(skill_slot, xp)
    skill_slot.experience = skill_slot.experience + xp
    skill_slot.level = xp2level(skill_slot, skill_slot.experience)
end

local function skill_define(args)
    args.aptitudes = resolve_aptitude_bonuses(args.aptitudes)
    args.on_calculate = on_calculate
    args.on_spend_experience = on_spend_experience
    return SkillType.define(args)
end

-- Main combat proficiencies
skill_define {
    name = "Melee",
    description = "Increases efficacy in physical combat.",
    aptitudes = {Apts.MELEE, {1,0,0,0}}
}

skill_define {
    name = "Magic",
    description = "Increases efficacy in the magic arts.",
    aptitudes = {Apts.MAGIC, {1,0,0,0}}
}

skill_define {
    name = "Melee Resistance",
    description = "Increases ability to withstand physical combat.",
    aptitudes = {Apts.MELEE, {0,0,1,0.25}}
}

skill_define {
    name = "Magic Resistance",
    description = "Increases ability to withstand magic damage.",
    aptitudes = {Apts.MAGIC, {0,0,1,0.25}}
}

skill_define {
    name = "Ranged",
    description = "Increases effectiveness and damage with ranged weaponry.",
    aptitudes = {Apts.RANGED, {1.0,0.5,0,0}}
}

-- Weapon types
skill_define {
    name = "Piercing Weapons",
    description = "Increases effectiveness and damage with piercing weapons.",
    aptitudes = {Apts.PIERCING, {1.0,0.5,0,0}}
}

skill_define {
    name = "Slashing Weapons",
    description = "Increases effectiveness and damage with slashing weapons.",
    aptitudes = {Apts.SLASHING, {1.0,0.5,0,0}}
}

skill_define {
    name = "Blunt Weapons",
    description = "Increases effectiveness and damage with blunt weapons.",
    aptitudes = {Apts.BLUNT, {1.0,0.5,0,0}}
}

-- Misc proficiencies

skill_define {
    name = "Armour",
    description = "Increases ability to use armour.",
    aptitudes = {Apts.ARMOUR, {1.0, dup(0,3)}}
}

skill_define {
    name = "Willpower",
    description = "Increases ability to resist the effects of magics.",
    aptitudes = {Apts.WILLPOWER, {1.0, dup(0,3)}}
}

skill_define {
    name = "Fortitude",
    description = "Increases ability to resist undesirable status changes.",
    aptitudes = {Apts.FORTITUDE, {1.0, dup(0,3)}}
}

skill_define {
    name = "Self-Mastery",
    description = "Increases ability to perform physical maneuvers and tactics.",
    aptitudes = {Apts.SELF_MASTERY, {1.0, dup(0,3)}}
}

 -- Ability to perform maneuvers

skill_define {
    name = "Magic Items",
    description = "Increases ability to use magic items.",
    aptitudes = {Apts.MAGIC_ITEMS, {1.0, dup(0,3)}}
}

-- Spell types
skill_define {
    name = "Enchantments",
    description = "Increases efficacy in enchanting magics.",
    aptitudes = {Apts.ENCHANTMENTS, {1.0, 0.5, 1.0, 0.5}}
}

skill_define {
    name = "Curses",
    description = "Increases efficacy in curses and hexes.",
    aptitudes = {Apts.CURSES, {1.0, 0.5, 1.0, 0.5}}
}

skill_define {
    name = "Force",
    description = "Increases efficacy in forceful, damaging magics.",
    aptitudes = {Apts.FORCE, {1.0, 0.5, 1.0, 0.5}}
}

-- Major 'elements'
skill_define {
    name = "Dark",
    description = "Increases control over the dark arts.",
    aptitudes = {Apts.DARK, {1.0, 0.5, 1.0, 0.5}}
}

skill_define {
    name = "Light",
    description = "Increases control over the powers of light.",
    aptitudes = {Apts.LIGHT, {1.0,  0.5, 1.0, 0.5}}
}

skill_define {
    name = "Fire",
    description = "Increases control over fire.",
    aptitudes = {Apts.FIRE, {1.0, 0.5, 1.0, 0.5}}
}

skill_define {
    name = "Water",
    description = "Increases control over water and ice.",
    aptitudes = {Apts.WATER, {1.0, 0.5, 1.0, 0.5}}
}

-- Minor elements
skill_define {
    name = "Earth",
    description = "Increases control over earth.",
    aptitudes = {Apts.EARTH, {1.0, 0.5, 1.0, 0.5}}
}

skill_define {
    name = "Air",
    description = "Increases control over the air.",
    aptitudes = {Apts.AIR, {1.0, 0.5, 1.0, 0.5}}
}

return M