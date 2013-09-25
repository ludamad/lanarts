local Apts = import "@stats.AptitudeTypes"
local spell_define = (import ".SpellDefineUtilsReplacement").spell_define
local StatusType = import "@StatusType"

local SpellTraits = import ".SpellTraits"

local function bonus_str1(val) return (val >= 0) and '+'..val or val end
local function bonus_str2(b1,b2) return ("%s,%s"):format(bonus_str1(b1 or 0), bonus_str1(b2 or 0)) end

local function resolve_bonuses(self)
    local eff,dam = self.effectiveness_bonus,self.damage_bonus
    self.name = bonus_str2(eff,dam) .. ' ' .. self.lookup_key
end

spell_define {
    lookup_key = "Minor Missile2",
    description = "A minor missile attack.",
    sprite = "sprites/minor_missile.png%32x32",

    mp_cost = 500,
    cooldown_offensive = 25,

    created_projectile = {
        speed = 7, radius = 4,
        aptitude_types = {Apts.MAGIC, Apts.FORCE},
        damage = 5, effectiveness = 2
    },

    on_create = function(type, args)
        local self = type:base_on_create(args)
        resolve_bonuses(self)
        return self
    end
}