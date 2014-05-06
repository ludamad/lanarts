local ClassType = import "@ClassType"
local ItemType = import "@ItemType"
local Apts = import "@stats.AptitudeTypes"
local SkillType = import "@SkillType"
local StatContext = import "@StatContext"
local ExperienceCalculation = import "@stats.ExperienceCalculation"
local LogUtils = import "lanarts.LogUtils"

local M = nilprotect {} -- Submodule

function M.default_spend_skill_points(self, stats, skill_points, --[[Optional]] log_increases)
    local total = 0
    for k,v in pairs(self.skills) do 
       total = total + v
    end
    if total == 0 then 
        return
    end
    local mult = 1/total
    local remainder = 0
    for k,v in pairs(self.skills) do
        local amount = math.floor(mult * skill_points)
        remainder = remainder + (mult * skill_points - amount)
        local new_v = v * mult * skill_points
        SkillType.get_skill_slot(stats.base.skills, k):on_spend_skill_points(stats, new_v, log_increases)
    end
end

function M.default_class_create(class_type, self_args)
    local self = setmetatable(table.deep_clone(self_args), {__index = class_type})
    self.type = self_args.type or class_type
    self.skills = table.deep_clone(self.skills)

    self.name = rawget(self, "name") or (class_type.name and class_type.name:interpolate(self))
    self.description = rawget(self, "description") or class_type.description:interpolate(self)
    return self
end

local function default_class_create_closure(class_type)
    return function(self_args) return M.default_class_create(class_type, self_args) end
end
 
local APT_FOR_SKILL = {
    ["Slashing Weapons"] = Apts.SLASHING,
    ["Piercing Weapons"] = Apts.PIERCING,
    ["Blunt Weapons"] = Apts.BLUNT,
    ["Ranged Fighting"] = Apts.RANGED
}

local function find_least_worth_weapon(trait)
    local wep, worth = nil, math.huge
    for item_type in ItemType.values() do
        if table.contains(item_type.traits, trait) then
            if item_type.gold_worth < worth then
                worth = item_type.gold_worth
                wep = item_type
            end
        end
    end
    return wep
end

function M.default_class_on_level_gain(self, stats)
    local HP_GAIN, MP_GAIN = 15, 15
    StatContext.permanent_add(stats, {hp=HP_GAIN, max_hp=HP_GAIN, mp=MP_GAIN, max_mp=MP_GAIN})
end

function M.default_class_on_map_init(self, stats)
    self:on_spend_skill_points(stats, ExperienceCalculation.SKILL_POINT_START_AMOUNT)
    if self.items then
        for item in values(self.items) do 
            StatContext.add_item(stats, item)
        end
    end
    if self.spells then
        for spell in values(self.spells) do 
            StatContext.add_spell(stats, spell)
        end
    end
    local wep = self.weapon
    if not wep then 
        wep = find_least_worth_weapon(APT_FOR_SKILL[self.weapon_skill])
    end
    if wep then
        if type(wep) == 'string' then wep = ItemType.lookup(wep) end
        assert(wep)
        if not wep.type then wep = {type = wep} end
        wep.equipped = true
        StatContext.add_item(stats, wep)
    end
end

function M.class_define(class_type)
    class_type.lookup_key = class_type.lookup_key or class_type.name
    assert(class_type.lookup_key)
    class_type.on_spend_skill_points = class_type.on_spend_skill_points or M.default_spend_skill_points
    class_type.on_create = class_type.on_create or default_class_create_closure(class_type)
    class_type.on_level_gain = class_type.on_level_gain or M.default_class_on_level_gain 
    class_type.on_map_init = class_type.on_map_init or M.default_class_on_map_init
    for k,v in pairs(class_type.skills) do 
        assert(SkillType.lookup(k), k .. " is not a skill!") 
    end

    return ClassType.define(class_type)
end

return M