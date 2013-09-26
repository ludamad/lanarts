local spell_define = (import ".SpellDefineUtils").spell_define
local StatusType = import "@StatusType"

local SpellTraits = import ".SpellTraits"

spell_define {
	name = "Berserk",
	description = "Allows you to strike powerful blows for a limited duration, afterwards you are slower and vulnerable.",
	mp_cost = 40,

    user_statuses_cant_have = {"Exhausted", "Berserk"},
    user_statuses_added = {{"Berserk", function(caster) return 150 + math.min(4, caster.base.level) * 20 end}},

    cooldown_self = 1000,
    cooldown_ability = 50
}