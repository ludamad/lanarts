{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
{:draw_simple_effect} = require "items.BonusesUtils"

define_stat_bonus = (name, description, stat_bonuses) ->
    define_bonus {
        :name
        sprite: tosprite("spr_bonuses.#{name\lower()}")
        console_draw_func: (obj, get_next) =>
            draw_simple_effect @, get_next(), name, description
        :stat_bonuses
        item_draw_func: (options, x, y) =>
            @sprite\draw options, {x, y}
    }

-- ITEM COST BONUSES
-- TODO NEVER PUT LUXURY IN STORES

define_stat_bonus "Polymath",
    "Gain Magic & Willpower.",
    {magic: 1, willpower: 1}

define_stat_bonus "Combat",
    "Gain Strength & Defence.",
    {strength: 1, defence: 1}

define_stat_bonus "Heart",
    "Gain Health and Healing.",
    {hp: 20, hpregen: 2/60}

define_stat_bonus "Mana",
    "Gain Mana and Regeneration.",
    {mp: 20, mpregen: 2/60}

define_stat_bonus "Power",
    "Gain Strength & Magic.",
    {magic: 1, strength: 1}

define_stat_bonus "Quickshot",
    "Increase firing rate.",
    {ranged_cooldown_multiplier: 1/(1.05)}

define_stat_bonus "Quickcast",
    "Increase casting rate.",
    {spell_cooldown_multiplier: 1/(1.05)}

define_stat_bonus "Flurry",
    "Increase melee rate.",
    {melee_cooldown_multiplier: 1/(1.05)}

define_stat_bonus "Savage",
    "Gain Strength, but lose Defence.",
    {strength: 3, defence: -2}

define_stat_bonus "Focus",
    "Gain Magic, but lose Willpower.",
    {magic: 3, willpower: -2}
