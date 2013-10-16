local CodeGeneration = import "core.CodeGeneration"

local SkillType = import "@SkillType"
local StatContext = import "@StatContext"
local ContentUtils = import ".ContentUtils"
local Apts = import "@stats.AptitudeTypes"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local LogUtils = import "lanarts.LogUtils"

local function resolve_aptitude_bonuses(apts)
    if type(apts) == "string" then
        return {[apts] = {dup(1,4)}}
    end
    if #apts > 0 then
        local temp = {}
        for i=1,#apts,2 do
            temp[apts[i]] = apts[i+1]
        end
        apts = temp
    end
    for k,v in pairs(apts) do
        if type(v) == "number" then
            apts[k] = {dup(v,4)}
        end
    end
    return apts
end

local function default_on_calculate(skill_slot, user)
    local apts = user.derived.aptitudes
    local level = user.base.level
    for type, apt in pairs(skill_slot.type.aptitudes) do
        local eff,dam,res,def = apt[1],apt[2],apt[3],apt[4]
        apts.effectiveness[type] = (apts.effectiveness[type] or 0) + eff * level   
        apts.damage[type] = (apts.damage[type] or 0) + dam * level   
        apts.resistance[type] = (apts.resistance[type] or 0) + res * level   
        apts.defence[type] = (apts.defence[type] or 0) + def * level   
    end
end

local function specialized_on_calculate(type, apt)
    local eff_val,dam_val,res_val,def_val = apt[1],apt[2],apt[3],apt[4]
    return function(_, user)
        local apts, level = user.derived.aptitudes, user.base.level
        local eff, dam, res, def = apts.effectiveness, apts.damage, apts.resistance, apts.defence
        eff[type] = (eff[type] or 0) + eff_val * level
        dam[type] = (dam[type] or 0) + dam_val * level
        res[type] = (res[type] or 0) + res_val * level
        def[type] = (def[type] or 0) + def_val * level
    end
end

local function on_create(skill)
    assert(skill)
    local slot = {type = skill, level = 0, skill_points = 0, cost_multiplier = 1.0}
    setmetatable(slot, ContentUtils.RESOURCE_METATABLE)
    return slot
end

local function xp2level(skill_slot, xp)
    return ExperienceCalculation.skill_level_from_cost(skill_slot.cost_multiplier, xp)
end

local function on_spend_skill_points(skill_slot, stats, sp, --[[Optional]] print)
    local old_level = skill_slot.level
    skill_slot.skill_points = skill_slot.skill_points + sp
    skill_slot.level = xp2level(skill_slot, skill_slot.skill_points)

    if print and skill_slot.level > old_level then
        local msg = ("{$You's}[Your] %s skill rises from %.1f to %.1f!"):format(
            skill_slot.name, old_level, skill_slot.level
        )
        LogUtils.event_log_player(stats.obj, msg, COL_PALE_GREEN)
    end
end

local function skill_define(args)
    args.aptitudes = resolve_aptitude_bonuses(args.aptitudes)
    local keys = table.key_list(args.aptitudes)
    if #keys == 1 then 
        local k = keys[1]
        args.on_calculate = args.default_on_calculate or specialized_on_calculate(k, args.aptitudes[k])
    else
        args.on_calculate = args.on_calculate or on_calculate
    end
    args.on_spend_skill_points = args.on_spend_skill_points or on_spend_skill_points
    args.on_create = args.on_create or on_create
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
    name = "Ranged Fighting",
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
    description = "Increases MP & the ability to resist the effects of magics.",
    aptitudes = {Apts.WILLPOWER, {1.0, dup(0,3)}},
    on_calculate = function(self, user)
        default_on_calculate(self, user)
        user.derived.max_hp = user.derived.max_mp + self.level * 20
    end
}

skill_define {
    name = "Fortitude",
    description = "Increases HP & the ability to resist undesirable status changes.",
    aptitudes = {Apts.FORTITUDE, {1.0, dup(0,3)}},
    on_calculate = function(self, user)
        default_on_calculate(self, user)
        user.derived.max_hp = user.derived.max_hp + self.level * 20
    end
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
    name = "Summoning",
    description = "Increases power of and control over summoned creatures.",
    aptitudes = {Apts.SUMMONING, {1.0, dup(0,3)}}
}

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