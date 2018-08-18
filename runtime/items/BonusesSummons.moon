-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"
{:draw_simple_effect} = require "items.BonusesUtils"

summon_console_draw_func = (bonus, obj, get_next) ->
    draw_simple_effect bonus, get_next(), bonus.name, "Can summon after kill"
    get_next()

-- TODO redo Centaur
-- TODO what do we want to do with centaurs?
-- 1. Restrict to colours - Black and White (servants and allies)
--      - Black centaurs are undead!!
-- 2. Have one centaur of capacity, show cooldown on item (?)
--      - Actual dedicated ally?
--          - Would need to put on a major slot, e.g. amulet
--          - Super rare ring enchant, too
-- 3. Have it just periodically summon, disenchant on floor change + death
--      - Use cooldown system, show cooldown on item itself(!)
define_bonus {
    name: "Centaur"
    shop_cost: {100, 200}
    sprite: tosprite "spr_enemies.humanoid.centaur"
    effects_granted: {"PossiblySummonCentaurOnKill"}
    console_draw_func: summon_console_draw_func
    -- effect: {
    --     init_func: (obj) =>
    --         @n_animations = 0
    --     on_melee_func: (obj, defender, damage) =>
    --         for _ in screens()
    --             if obj\is_local_player()
    --                 EventLog.add("You steal the enemy's life!", {200,200,255})
    --         BonusesUtils.create_animation @, obj, "spr_bonuses.fangs"
    --         obj\heal_hp(damage / 5 * @n_derived)
    --         return damage
    -- }
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

define_bonus {
    name: "Golem"
    shop_cost: {100, 200}
    sprite: tosprite "golem"
    effects_granted: {"PossiblySummonGolemOnKill"}
    console_draw_func: summon_console_draw_func
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

define_bonus {
    name: "Stormcall"
    shop_cost: {50, 100}
    sprite: tosprite "storm elemental"
    effects_granted: {"PossiblySummonStormElementalOnKill"}
    console_draw_func: summon_console_draw_func
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}

define_bonus {
    name: "Mummycall"
    shop_cost: {200, 400}
    sprite: tosprite "spr_enemies.undead.mummy"
    effects_granted: {"SummonMummyOnKill"}
    console_draw_func: summon_console_draw_func
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}
