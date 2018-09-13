-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
BonusesUtils = require "items.BonusesUtils"
EventLog = require "ui.EventLog"

define_bonus {
    name: "Spiky"
    sprite: tosprite "spr_bonuses.spiky"
    shop_cost: {100, 200}
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
    effect: {
        console_draw_func: (obj, get_next) =>
            sprite = tosprite "spr_bonuses.spiky"
            BonusesUtils.draw_simple_effect {sprite: tosprite "spr_bonuses.spiky"}, get_next(), "Spiky",
                "You deal +25% damage back on melee."
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

-- Fortified: Grants fortification (description from effect)

define_bonus {
    name: "Fortified"
    shop_cost: {100, 200}
    sprite: tosprite "spr_spells.statue_form"
    effects_granted: {"Fortification"}
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}
