-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
BonusesUtils = require "items.BonusesUtils"
EventLog = require "ui.EventLog"

define_bonus {
    name: "Spiky"
    sprite: tosprite "spr_bonuses.spiky"
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
    effect: {
        console_draw_func: (obj, get_next) =>
            sprite = tosprite "spr_bonuses.spiky"
            draw_console_effect get_next(), sprite, {
                {COL_LIGHT_GRAY, "Spiky: "}
                {COL_WHITE, "You deal +25% damage back on melee."}
            }
            get_next()
        init_func: (obj) =>
            @n_animations = 0
        on_receive_melee_func: (attacker, obj, damage) =>
            percentage_recoil = @n_derived * 0.25
            attacker\direct_damage(random_round(damage * percentage_recoil), obj)
            for _ in screens()
                if obj.is_local_player and obj\is_local_player()
                    EventLog.add("You strike back with spikes!", COL_PALE_BLUE)
            BonusesUtils.create_animation @, attacker, "spr_bonuses.spiky"
    }
}
    --
    -- key: "recoil_percentage" -- Additive effect, accessed with @value().
    -- console_draw_func: (player, get_next) =>
    --     draw_console_effect(tosprite("spr_spells.spectral_weapon"), "+#{math.floor(@recoil_percentage * 100)}% melee recoil damage", get_next())
    -- on_receive_melee_func: (attacker, defender, damage) =>
    --     percentage_recoil = @value()
    --     attacker\direct_damage(damage * percentage_recoil, defender)
    --     for _ in screens()
    --         if defender.is_local_player and defender\is_local_player()
    --             EventLog.add("You strike back with spikes!", COL_PALE_BLUE)
