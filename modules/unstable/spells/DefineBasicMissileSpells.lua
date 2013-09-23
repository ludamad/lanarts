local Apts = import "@stats.AptitudeTypes"
local SpellDefineUtils = import ".SpellDefineUtils"
local missile_spell_define = SpellDefineUtils.missile_spell_define
local StatusType = import "@StatusType"

local SpellTraits = import ".SpellTraits"

local function bonus_str1(val) return (val >= 0) and '+'..val or val end
local function bonus_str2(b1,b2) return ("%s,%s"):format(bonus_str1(b1 or 0), bonus_str1(b2 or 0)) end

local function resolve_bonuses(self)
    local eff,dam = self.effectiveness_bonus,self.damage_bonus
    self.name = bonus_str2(eff,dam) .. ' ' .. self.lookup_key
end

missile_spell_define {
    lookup_key = "Minor Missile",
    description = "A minor missile attack.",
    sprite = "sprites/minor_missile.png%32x32",

    mp_cost = 5, cooldown = 25,
    speed = 7, radius = 4,

    types = {Apts.MAGIC, Apts.FORCE},
    damage = 5, effectiveness = 2,

    on_create = function(type, args)
        local self = SpellDefineUtils.base_spell_on_create(type, args)
        resolve_bonuses(self)
        return self
    end
}