local TypeEffectUtils = require "spells.TypeEffectUtils"
local MiscSpellAndItemEffects = require "core.MiscSpellAndItemEffects"
local EventLog = require "ui.EventLog"
local Map = require "core.Map"
local GameObject = require "core.GameObject"

Data.item_create {
    name = "Gold", -- An entry named gold must exist, it is handled specially
    spr_item = "gold",
    pickup_func = function(self, user, amount)
        user:gain_gold(amount)
        play_sound("sound/gold.ogg")
        GameObject.animation_create {
            map = user.map,
            xy = {user.x + 10, user.y},
            sprite = "spr_effects.coin",
            duration = 25,
            velocity = {-1, -1}
        }
        return true -- dont pickup
    end
}

Data.item_create {
    name = "Azurite Key",
    type = "key",
    entry_type = "Key",
    description = "Now that you have picked up this key, you can open Azurite doors.",
    use_message = "Now that you have picked up this key, you can open Azurite doors.",
    spr_item = "key1",
    pickup_func = function(self, user)
        local GlobalData = require "core.GlobalData"
        if not GlobalData.keys_picked_up[self.name] then
            play_sound "sound/win sound 2-1.ogg"
        end
        GlobalData.keys_picked_up[self.name] = true
    end,
    prereq_func = function (self, user)
        return false
    end,
    stackable = false,
    sellable = false
}

Data.item_create {
    name = "Dandelite Key",
    description = "Now that you have picked up this key, you can open Dandelite doors.",
    type = "key",
    entry_type = "Key",
    use_message = "Now that you have picked up this key, you can open Dandelite doors.",
    spr_item = "key2",
    pickup_func = function(self, user)
        local GlobalData = require "core.GlobalData"
        if not GlobalData.keys_picked_up[self.name] then
            play_sound "sound/win sound 2-1.ogg"
        end
        GlobalData.keys_picked_up[self.name] = true
    end,
    prereq_func = function (self, user)
        return false
    end,
    sellable = false,
    stackable = false
}

Data.item_create {
    name = "Magentite Key",
    description = "Now that you have picked up this key, you can open Magentite doors.",
    type = "key",
    entry_type = "Key",
    spr_item = "spr_keys.magentite_key",
    pickup_func = function(self, user)
        local GlobalData = require "core.GlobalData"
        if not GlobalData.keys_picked_up[self.name] then
            play_sound "sound/win sound 2-1.ogg"
        end
        GlobalData.keys_picked_up[self.name] = true
    end,
    prereq_func = function (self, user)
        return false
    end,
    sellable = false,
    stackable = false
}

Data.item_create {
    name = "Burgundite Key",
    description = "Now that you have picked up this key, you can open Burgundite doors.",
    type = "key",
    entry_type = "Key",
    use_message = "Now that you have picked up this key, you can open Burgundite doors.",
    spr_item = "key3",
    pickup_func = function(self, user)
        local GlobalData = require "core.GlobalData"
        if not GlobalData.keys_picked_up[self.name] then
            play_sound "sound/win sound 2-1.ogg"
        end
        GlobalData.keys_picked_up[self.name] = true
    end,
    prereq_func = function (self, user)
        return false
    end,
    sellable = false,
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
    name = "Red Mana Potion",
    description = "A magical potion of energizing infusions, it restores 25 MP to the user for every point of Red Power.",
    type = "potion",

    shop_cost = {15, 35},

    spr_item = "spr_scrolls.red_mana_potion",

    prereq_func = function (self, user)
        if TypeEffectUtils.get_power(user, "Red") <= 0 then
            for _ in screens() do
                if caster:is_local_player() then
                    EventLog.add("You do not have any Red Power!", COL_PALE_RED)
                end
            end
            return false
        end
        return user.stats.mp < user.stats.max_mp
    end,

    action_func = function(self, user)
        user:heal_mp(TypeEffectUtils.get_power(user, "Red") * 25)
    end
}

Data.item_create {
    name = "Scroll of Fear",
    description = "Bestows the user with a terrible apparition, scaring away all enemies.",
    use_message = "You appear frightful!",

    shop_cost = {55,105},

    spr_item = "spr_scrolls.fear",

    prereq_func = function(self, user) 
        return not user:has_effect("Fear Aura")
    end,

    action_func = function(self, user)
        user:add_effect("Fear Aura", 800).range = 120
    end
}

local function scroll_of_exp_func(self, user)
    -- Collect ally players.
    local ally_players = {user}
    for _, ally in ipairs(Map.allies_list(user)) do
        append(ally_players, ally)
    end
    -- Grant all of them an XP share.
    local xp_granted = 100 / #ally_players
    for _, ally in ipairs(ally_players) do
        ally:gain_xp(xp_granted)
    end
end

Data.item_create {
    name = "Scroll of Experience",
    description = "Grants 100XP for the user's party.",
    use_message = "Experience is bestowed upon you!",

    shop_cost = {55,105},

    spr_item = "scroll_exp",

    pickup_func = function(self, user, amount)
        play_sound "sound/win sound 2-1.ogg"
        EventLog.add_all("Experience is bestowed upon you!", {255,255,255})
        for i=1,amount do
            scroll_of_exp_func(self, user, amount)
        end
        return true -- dont pickup
    end,
    action_func = scroll_of_exp_func
}

Data.item_create {
    name = "Defence Scroll",
    description = "A mantra of unnatural modification, it bestows the user with a permanent, albeit small, increase to defence.",
    use_message = "Defence is bestowed upon you!",

    shop_cost = {550,1050},

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
    name = "Honeycomb",
    description = "A tasty honeycomb. Restores 100 HP to the user.",
    type = "potion",

    shop_cost = {45,95},

    spr_item = "spr_keys.honeycomb",

    prereq_func = function (self, user)
        return user.stats.hp < user.stats.max_hp
    end,

    action_func = function(self, user)
        user:heal_hp(100)
    end
}

Data.item_create {
    name = "Strength Scroll",
    description = "A mantra of unnatural modification, it bestows the user with a permanent, albeit small, increase to strength.",
    use_message = "Strength is bestowed upon you!",

    shop_cost = {550,1050},

    spr_item = "scroll_strength",

    action_func = function(self, user)
        user.stats.strength = user.stats.strength + 1
    end
}

Data.item_create  {
    name = "Haste Scroll",
    description = "A scroll that captures the power of the winds, it bestows the user with power and speed for a limited duration.",
    use_message = "You feel yourself moving faster and faster!",

    shop_cost = {25,35},

    spr_item = "scroll_haste",

    prereq_func = function(self, user) 
        return not user:has_effect("Haste")
    end,

    action_func = function(self, user)
        play_sound "sound/haste.ogg"
        user:add_effect("Haste", 800)
    end
}

Data.item_create  {
    name = "Will Scroll",
    description = "A mantra of unnatural modification, it bestows the user with a permanent, albeit small, increase to will.",
    use_message = "Will is bestowed upon you!",

    shop_cost = {550,1050},

    spr_item = "scroll_will",

    action_func = function(self, user)
        user.stats.willpower = user.stats.willpower + 1
    end
}

Data.item_create {
    name = "Luminos Spellbook",
    use_message = "You learn Luminos!",
    description = "Teaches the spell 'Luminos'. The user must be a Stormcaller.",

    shop_cost = {55,205},

    spr_item = "spr_books.yellow",

    prereq_func = function(self, user)
        return user:has_effect "Stormcaller"
    end,
    action_func = function(self, user)
        user:learn_spell "Luminos"
    end
}

Data.item_create {
    name = "Fear Strike Manual",
    use_message = "You learn Fear Strike!",
    description = "Teaches the spell 'Fear Strike' to any class.",

    shop_cost = {55,105},

    spr_item = "spr_books.dark_gray",

    prereq_func = function(self, user)
        return true
    end,
    action_func = function(self, user)
        user:learn_spell "Fear Strike"
    end
}

Data.item_create {
    name = "Magic Scroll",
    use_message = "Magic is bestowed upon you!",
    description = "A mantra of unnatural modification, it bestows the user with a permanent, albeit small, increase to magic.",

    shop_cost = {550,1050},

    spr_item = "scroll_magic",

    action_func = function(self, user)
        user.stats.magic = user.stats.magic + 1
    end
}

Data.item_create {
    name = "Magic Map",
    use_message = "You gain knowledge of your surroundings!",
    description = "A magic map that reveals the current level.",

    shop_cost = {25,35},

    spr_item = "spr_scrolls.magic-map",

    action_func = function(self, user)
        MiscSpellAndItemEffects.magic_map_effect(user)
        EventLog.add_all("The map is revealed!", {255,255,255})
    end
}
