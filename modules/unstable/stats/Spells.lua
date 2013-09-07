local Spells = import "@Spells"
local StatusType = import "@StatusType"

local SpellTraits = import ".spell_traits"
local status_types = import ".status_types"

local M = nilprotect {} -- Submodule

M.berserk = Spells.define {
	name = "Berserk",
	description = "Allows you to strike powerful blows for a limited duration, afterwards you are slower and vulnerable.",
	mp_cost = 40,

	-- Used for AI and interface decisions
	traits = {SpellTraits.ABILITY_SPELL, SpellTraits.BUFF_SPELL},

    on_prerequisite = function(self, caster)
        local D = caster.derived

        local not_exhausted = not StatusType.get_hook(D.hooks, status_types.Exhausted)
        local not_berserk = not StatusType.get_hook(D.hooks, status_types.Berserk)
 
        return not_exhausted and not_berserk
    end,

    on_use = function(self, caster)
        local B = caster.base
        StatusType.update_hook(B.hooks, status_types.Berserk, caster, 150 + math.min(4, B.level)  * 20)
    end,

    cooldown_self = 1000,
    cooldown_offensive = 50
}

return M