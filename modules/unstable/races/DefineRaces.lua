local M = nilprotect {} -- Submodule

local Spells = import "@Spells"
local Stats = import "@Stats"
local Races = import "@Races"
local SpellTraits = import ".spell_traits"
local AptitudeTypes = import ".aptitude_types"
local StatContext = import "@StatContext"
local LogUtils = import "lanarts.log_utils"

local UNDEAD_HEAL_AMOUNT = 30

local undead_heal = Spells.define {
    name = "Benevolent Curse",
    description = "You gain control of the curse that brought you to unlife, manipulating it to heal yourself.",
    traits = {SpellTraits.BUFF_SPELL},
    mp_cost = 30,
    cooldown_offensive = 35,
    target_type = Spells.TARGET_NONE,
    on_prerequisite = function (self, caster)
        return caster.base.hp < caster.derived.max_hp
    end,
    on_use = function (self, caster)
        local actual = StatContext.add_hp(caster, UNDEAD_HEAL_AMOUNT)
        LogUtils.resolved_log(caster.obj, "<The >$You invoke the curse to gain{s} " .. actual .. "HP!", COL_GREEN)
    end
}

M.undead = Races.define {
    name = "Undead",
    description = string.pack [[
    A creature of unlife, summoned by an ancient curse. The greatest of the undead can control their new form.
    Undead adventurers do not regenerate HP naturally, instead manipulating their curse to heal themselves.
    They possess great aptitude in the dark arts.
]],
    on_create = function(name, team)
        local stats = Stats.stats_create {
            hp = 80, mp = 80,
            name = name, team = team,
            immunities = {poison = true}, 
        }

        -- All the convenient methods are defined over a stat context, so create one
        local context = StatContext.stat_context_create(stats)
        StatContext.add_all_aptitudes(context, AptitudeTypes.dark, 2, --[[Permanent]] true)
        StatContext.add_spell(context, undead_heal)

        return stats
    end
}

M.human = Races.define {
    name = "Human",
    description = string.pack [[
    A versatile race. Humans adventurers have high health and mana reserves. 
    Additionally, they possess great aptitude at using magic devices and performing physical maneuvers. 
]],
    on_create = function(name, team)
        local stats = Stats.stats_create {
            hp = 100, mp = 100,
            name = name, team = team
        }

        -- All the convenient methods are defined over a stat context, so create one
        local context = StatContext.stat_context_create(stats)
        StatContext.add_effectiveness(context, AptitudeTypes.self_mastery, 2, --[[Permanent]] true)
        StatContext.add_effectiveness(context, AptitudeTypes.magic_devices, 2, --[[Permanent]] true)

        return stats
    end
}

return M