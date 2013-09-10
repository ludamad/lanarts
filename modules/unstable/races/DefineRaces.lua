local M = nilprotect {} -- Submodule

local Spells = import "@Spells"
local Stats = import "@Stats"
local StatContext = import "@StatContext"
local SpellTraits = import "@stats.SpellTraits"
local Apts = import "@stats.AptitudeTypes"
local LogUtils = import "lanarts.LogUtils"

local races_define = (import "@Races").define

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
        LogUtils.resolved_log(caster.obj, "<The >$You invoke [the]{its} curse to gain{s} " .. actual .. "HP!", COL_GREEN)
    end
}

races_define {
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
            hp_regen = 0.0, mp_regen = 0.012
        }

        -- All the convenient methods are defined over a stat context, so create one
        local context = StatContext.stat_context_create(stats)
        StatContext.add_all_aptitudes(context, Apts.DARK, 2, --[[Permanent]] true)
        StatContext.add_all_aptitudes(context, Apts.LIGHT, -2, --[[Permanent]] true)
        StatContext.add_effectiveness(context, Apts.CURSES, 2, --[[Permanent]] true)
        StatContext.add_effectiveness(context, Apts.ENCHANTMENTS, -2, --[[Permanent]] true)
        StatContext.add_resistance(context, Apts.POISON, 20, --[[Permanent]] true)
        StatContext.add_spell(context, undead_heal)

        return stats
    end
}

races_define {
    name = "Human",
    description = string.pack [[
    A versatile race. Humans adventurers have high health and mana reserves. 
    They possess great aptitude at using tools and performing physical maneuvers. 
]],
    on_create = function(name, team)
        local stats = Stats.stats_create {
            hp = 100, mp = 100,
            name = name, team = team,
            hp_regen = 0.0, mp_regen = 0.012
        }

        -- All the convenient methods are defined over a stat context, so create one
        local context = StatContext.stat_context_create(stats)
        StatContext.add_effectiveness(context, Apts.SELF_MASTERY, 2, --[[Permanent]] true)
        StatContext.add_effectiveness(context, Apts.MAGIC_ITEMS, 2, --[[Permanent]] true)
        StatContext.add_effectiveness(context, Apts.WEAPON_IDENTIFICATION, 2, --[[Permanent]] true)

        return stats
    end
}

races_define {
    name = "Orc",
    description = string.pack [[
    A brutish race. Orcish magic and combat focuses on dealing heavy blows. 
    Additionally, they possess great aptitude at using magic devices and performing physical maneuvers. 
]],
    on_create = function(name, team)
        local stats = Stats.stats_create {
            hp = 90, mp = 80,
            name = name, team = team,
            hp_regen = 0.0, mp_regen = 0.012
        }

        -- All the convenient methods are defined over a stat context, so create one
        local context = StatContext.stat_context_create(stats)
        StatContext.add_offensive_aptitudes(context, Apts.MAGIC, {-4,2}, --[[Permanent]] true)
        StatContext.add_offensive_aptitudes(context, Apts.MELEE, {-4,2}, --[[Permanent]] true)
        StatContext.add_effectiveness(context, Apts.FORTITUDE, 2, --[[Permanent]] true)
        StatContext.add_effectiveness(context, Apts.WILLPOWER, 2, --[[Permanent]] true)
        StatContext.add_effectiveness(context, Apts.EARTH, 2, --[[Permanent]] true)
        StatContext.add_effectiveness(context, Apts.AIR, -2, --[[Permanent]] true)

        return stats
    end
}