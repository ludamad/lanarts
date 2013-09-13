local Utils,Traits = import ".ItemDefineUtils", import ".ItemTraits"
local LogUtils = import "lanarts.LogUtils"
local StatContext = import "@StatContext"

local HEALTH_POTION_HEAL_AMOUNT = 50
Utils.potion_define {
    name = "Health Potion",
    description = "A blessed potion of healing infusions, it restores "..HEALTH_POTION_HEAL_AMOUNT.." HP to the user.",

    on_prerequisite = function(self, stats)
        return stats.derived.hp < stats.derived.max_hp
    end,

    on_use = function(self, user)
        local actual = StatContext.add_hp(user, HEALTH_POTION_HEAL_AMOUNT)
        LogUtils.resolved_log(user.obj, "<The >$You gain{s} " .. actual .. "HP!")
    end
}

local MANA_POTION_MP_GAIN_AMOUNT = 50
Utils.potion_define {
    name = "Mana Potion",
    description = "A prized concoction, it produces a spike of magical energy, worth "..MANA_POTION_MP_GAIN_AMOUNT.." MP.",

    on_prerequisite = function(self, stats)
        return stats.derived.hp < stats.derived.max_hp
    end,

    on_use = function(self, user)
        local actual = StatContext.add_mp(user, MANA_POTION_MP_GAIN_AMOUNT)
        LogUtils.resolved_log(user.obj, "<The >$You gain{s} " .. actual .. "MP!", COL_WHITE)
    end
}