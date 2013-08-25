local ItemType = import "@ItemType"
local ItemTraits = import ".item_traits"
local StatContext = import "@StatContext"

local HEALTH_POTION_HEAL_AMOUNT = 50

ItemType.define {
	name = "Health Potion",
	description = "A blessed potion of healing infusions, it restores "..HEALTH_POTION_HEAL_AMOUNT.." HP to the user.",

	traits = {ItemTraits.POTION},

	on_prerequisite = function(self, user)
		return user.base.hp < user.stats.hp
	end,

	on_use = function(self, user)
		StatContext.hp_add(user, HEALTH_POTION_HEAL_AMOUNT)
	end
}