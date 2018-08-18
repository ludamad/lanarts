-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
{:of_colors, :draw_color_bound_weapon_effect} = require "items.BonusesUtils"

Display = require "core.Display"
EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
BonusesUtils = require "items.BonusesUtils"

-- Knockback: Stun bonus (description from effect)
define_bonus {
    name: "Knockback"
    shop_cost: {50, 100}
    sprite: tosprite "spr_effects.fleeing"
    effects_granted: {"KnockbackWeapon"}
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 20, y}
}

-- Poison: Stun bonus (description from effect)
define_bonus {
    name: "Poison"
    shop_cost: {100, 200}
    sprite: tosprite "spr_weapons.i-venom"
    effects_granted: {{"PoisonedWeapon", {poison_percentage: 0.1}}}
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 16, y - 16}
}

define_bonus {
    name: "Confusion"
    shop_cost: {100, 200}
    sprite: tosprite "spr_weapons.i-confusion"
    effects_granted: {"ConfusingWeapon"}
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 16, y - 16}
}

-- - Vampirism, gain health on weapon damage, Black/Green.
--  TODO - Enliven - gain health on weapon kill, White/Blue.
define_bonus {
    name: "Vampiric"
    shop_cost: {100, 200}
    sprite: tosprite "spr_weapons.i-vampirism"
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 16, y - 16}

    console_draw_func: (obj, get_next) =>
        draw_color_bound_weapon_effect @, obj, get_next, {"Black", "Green"},
            "Heal +20% of melee damage dealt."

    effect: {
        init_func: (obj) =>
            @n_animations = 0
        on_melee_func: (obj, defender, damage) =>
            if of_colors obj, {"Black", "Green"}
                for _ in screens()
                    if obj\is_local_player()
                        EventLog.add("You steal the enemy's life!", {200,200,255})
                BonusesUtils.create_animation @, obj, "spr_bonuses.fangs"
                obj\heal_hp(damage / 5 * @n_derived)
            return damage
    }
}
