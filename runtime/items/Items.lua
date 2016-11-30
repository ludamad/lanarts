local GlobalData = require "core.GlobalData"

assert(GlobalData.keys_picked_up)

Data.item_create {
    name = "Gold", -- An entry named gold must exist, it is handled specially
    spr_item = "gold"
}

Data.item_create {
    name = "Azurite Key",
    type = "key",
    description = "Now that you have picked up this key, you can open Azurite doors.",
    use_message = "Now that you have picked up this key, you can open Azurite doors.",
    spr_item = "key1",
    pickup_func = function(self, user)
        GlobalData.keys_picked_up[self.name] = true 
        pretty("KEYS: ", GlobalData.keys_picked_up)
    end,
    prereq_func = function (self, user)
        return false
    end,
    stackable = false
}

Data.item_create {
    name = "Dandelite Key",
    description = "Now that you have picked up this key, you can open Dandelite doors.",
    type = "key",
    use_message = "Now that you have picked up this key, you can open Dandelite doors.",
    spr_item = "key2",
    pickup_func = function(self, user)
        GlobalData.keys_picked_up[self.name] = true 
        pretty("KEYS: ", GlobalData.keys_picked_up)
    end,
    prereq_func = function (self, user)
        return false
    end,
    stackable = false
}

Data.item_create {
    name = "Burgundite Key",
    description = "Now that you have picked up this key, you can open Burgundite doors.",
    type = "key",
    use_message = "Now that you have picked up this key, you can open Burgundite doors.",
    spr_item = "key3",
    pickup_func = function(self, user)
        local GlobalData = require "core.GlobalData"
        GlobalData.keys_picked_up[self.name] = true 
        pretty("KEYS: ", GlobalData.keys_picked_up)
    end,
    prereq_func = function (self, user)
        return false
    end,
    stackable = false
}

Data.item_create {
    name = "Mana Potion",
    description = "A magical potion of energizing infusions, it restores 50 MP to the user.",
    type = "potion",

    shop_cost = {15, 35},

    spr_item = "mana_potion",

    prereq_func = function (self, user)
        return user.stats.mp < user.stats.max_mp
    end,

    action_func = function(self, user)
        user:heal_mp(50)
    end
}

Data.item_create {
    name = "Scroll of Experience",
    description = "Bestows the user with a vision, leading to increased experience.",
    use_message = "Experience is bestowed upon you!",

    shop_cost = {55,105},

    spr_item = "scroll_exp",

    action_func = function(self, user)
        user.stats.xp = user.stats.xp + 50
    end
}

Data.item_create {
    name = "Defence Scroll",
    description = "A mantra of unnatural modification, it bestows the user with a permanent, albeit small, increase to defence.",
    use_message = "Defence is bestowed upon you!",

    shop_cost = {55,105},

    spr_item = "scroll_defence",

    action_func = function(self, user)
        user.stats.defence = user.stats.defence + 1
    end
}

Data.item_create {
    name = "Health Potion",
    description = "A blessed potion of healing infusions, it restores 50 HP to the user.",
    type = "potion",

    shop_cost = {15,35},

    spr_item = "health_potion",

    prereq_func = function (self, user) 
        return user.stats.hp < user.stats.max_hp 
    end,

    action_func = function(self, user)
        user:heal_hp(50)
    end
}

Data.item_create {
    name = "Strength Scroll",
    description = "A mantra of unnatural modification, it bestows the user with a permanent, albeit small, increase to strength.",
    use_message = "Strength is bestowed upon you!",

    shop_cost = {55,105},

    spr_item = "scroll_strength",

    action_func = function(self, user)
        user.stats.strength = user.stats.strength + 1
    end
}

Data.item_create  {
    name = "Haste Scroll",
    description = "A scroll that captures the power of the winds, it bestows the user with power and speed for a limited duration.",
    use_message = "You feel yourself moving faster and faster!",

    shop_cost = {25,45},

    spr_item = "scroll_haste",

    action_func = function(self, user)
        user:add_effect(effects.Haste.name, 800)
    end
}

Data.item_create  {
    name = "Will Scroll",
    description = "A mantra of unnatural modification, it bestows the user with a permanent, albeit small, increase to will.",
    use_message = "Will is bestowed upon you!",

    shop_cost = {55,105},

    spr_item = "scroll_will",

    action_func = function(self, user)
        user.stats.willpower = user.stats.willpower + 1
    end
}

Data.item_create {
    name = "Magic Scroll",
    use_message = "Magic is bestowed upon you!",
    description = "A mantra of unnatural modification, it bestows the user with a permanent, albeit small, increase to magic.",

    shop_cost = {55,105},

    spr_item = "scroll_magic",

    action_func = function(self, user)
        user.stats.magic = user.stats.magic + 1
    end
}
