-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"

summon_console_draw_func = (bonus, inst, get_next) ->
    draw_console_effect get_next(), bonus.sprite, {
        {COL_GOLD, "#{bonus.name}: "}
        {COL_WHITE, "Can summon after kill."}
    }
    get_next()

define_bonus {
    name: "Centaur"
    sprite: tosprite "spr_enemies.humanoid.centaur"
    effects_granted: {"PossiblySummonCentaurOnKill"}
    console_draw_func: summon_console_draw_func
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

define_bonus {
    name: "Golem"
    sprite: tosprite "golem"
    effects_granted: {"PossiblySummonGolemOnKill"}
    console_draw_func: summon_console_draw_func
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

define_bonus {
    name: "Stormcall"
    sprite: tosprite "storm elemental"
    effects_granted: {"PossiblySummonStormElementalOnKill"}
    console_draw_func: summon_console_draw_func
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

define_bonus {
    name: "Mummycall"
    sprite: tosprite "spr_enemies.undead.mummy"
    effects_granted: {"SummonMummyOnKill"}
    console_draw_func: (inst, get_next) =>
        draw_console_effect get_next(), @sprite, {
            {COL_GOLD, "#{bonus.name}: "}
            {COL_WHITE, "Summons after each kill."}
        }
        get_next()
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}
