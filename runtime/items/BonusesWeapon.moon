-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
Display = require "core.Display"
EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"

-- Knockback: Stun bonus (description from effect)
define_bonus {
    name: "Knockback"
    sprite: tosprite "spr_effects.fleeing"
    effects_granted: {"KnockbackWeapon"}
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 20, y}
}

-- Fortified: Grants fortifaction (description from effect)

define_bonus {
    name: "Fortified"
    sprite: tosprite "spr_spells.statue_form"
    effects_granted: {"Fortification"}
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

-- Poison: Stun bonus (description from effect)
define_bonus {
    name: "Poison"
    sprite: tosprite "spr_weapons.i-venom"
    effects_granted: {{"PoisonedWeapon", {poison_percentage: 0.1}}}
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 16, y - 16}
}

define_bonus {
    name: "Confusion"
    sprite: tosprite "spr_weapons.i-confusion"
    effects_granted: {"ConfusingWeapon"}
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 16, y - 16}
}

VAMPIRISM_VELOCITIES = {
    {-4, -4}
    {0, -4}
    {4, -4}
    {4, 0}
    {4, 4}
    {0, 4}
    {-4, 4}
    {-4, 0}
}

-- TODO:
-- - Vampirism, gain health on weapon damage, Black/Green.
-- - Enliven - gain health on weapon kill, White/Green.
define_bonus {
    name: "Vampiric"
    sprite: tosprite "spr_weapons.i-vampirism"
    item_draw_func: (options, x, y) =>
        options = table.merge options, {scale: {1,1}}
        @sprite\draw options, {x - 16, y - 16}

    console_draw_func: (obj, get_next) =>
        xy = get_next()
        obj.weapon_sprite\draw {
            origin: Display.LEFT_CENTER
        }, {xy[1], xy[2] + 4}
        draw_console_effect xy, @sprite, {
            {COL_LIGHT_GRAY, "#{@name}: "}
            {COL_WHITE, "Heal +20% of melee damage dealt."}
        }
        get_next()

    effect: {
        init_func: (obj) =>
            @attacks = 0
        on_melee_func: (obj, defender, damage) =>
            for _ in screens()
                if obj\is_local_player()
                    EventLog.add("You steal the enemy's life!", {200,200,255})
            GameObject.animation_create {
                xy: obj.xy
                sprite: "spr_bonuses.fangs"
                duration: 25
                velocity: VAMPIRISM_VELOCITIES[(@attacks % #VAMPIRISM_VELOCITIES) + 1]
            }
            obj\heal_hp(damage / 5 * @n_derived)
            @attacks += 1
            return damage
    }
}
