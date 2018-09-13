{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
{:draw_simple_effect} = require "items.BonusesUtils"

define_stat_bonus = (name, description, stat_bonuses) ->
    define_bonus {
        :name
        shop_cost: {100, 200}
        sprite: tosprite("spr_bonuses.#{name\lower()}")
        console_draw_func: (obj, get_next) =>
            -- draw_simple_effect @, get_next(), name, description
            nil
        :stat_bonuses
        item_draw_func: (options, x, y) =>
            @sprite\draw options, {x, y}
    }

-- ITEM COST BONUSES
-- TODO NEVER PUT LUXURY IN STORES

define_stat_bonus "Polymath",
    "Gain Willpower.",
    {willpower: 2}

define_stat_bonus "Strength",
    "Gain Strength",
    {strength: 2}

define_stat_bonus "Heart",
    "Gain Health.",
    {hp: 30}

define_stat_bonus "Mana",
    "Gain Mana.",
    {mp: 30}

define_stat_bonus "Regeneration",
    "Gain Health Regeneration.",
    {hpregen: 4/60}

define_stat_bonus "Wit",
    "Gain Mana Regeneration.",
    {mpregen: 4/60}

define_stat_bonus "Power",
    "Gain Magic.",
    {magic: 2}

define_stat_bonus "Quickshot",
    "Increase firing rate.",
    {ranged_cooldown_multiplier: 1/(1.08)}

define_stat_bonus "Quickcast",
    "Increase casting rate.",
    {spell_cooldown_multiplier: 1/(1.08)}

define_stat_bonus "Flurry",
    "Increase melee rate.",
    {melee_cooldown_multiplier: 1/(1.08)}

define_stat_bonus "Speedbolt",
    "Increase spell velocity.",
    {spell_velocity_multiplier: 1.3}

define_stat_bonus "Savage",
    "Gain Strength, but lose Defence.",
    {strength: 4, defence: -2}

define_stat_bonus "Focus",
    "Gain Magic, but lose Willpower.",
    {magic: 4, willpower: -2}

define_stat_bonus "Shield",
    "Gain Defence.",
    {defence: 2}
