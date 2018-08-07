-- TODO move effects_granted console drawing into here??

{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'
{:define_bonus} = require "items.Bonuses"

summon_console_draw_func = (bonus, obj, get_next) ->
    draw_console_effect get_next(), bonus.sprite, {
        {COL_GOLD, "#{bonus.name}: "}
        {COL_WHITE, "Can summon after kill."}
    }
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
    sprite: tosprite "spr_enemies.humanoid.centaur"
    effects_granted: {"PossiblySummonCentaurOnKill"}
    console_draw_func: (obj, get_next) =>
        draw_console_effect get_next(), @sprite, {
            {COL_LIGHT_GRAY, "#{@name}: "}
            {COL_WHITE, "Summon a Centaur. "}
        }
        get_next()
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
    console_draw_func: (obj, get_next) =>
        draw_console_effect get_next(), @sprite, {
            {COL_GOLD, "#{bonus.name}: "}
            {COL_WHITE, "Summons after each kill."}
        }
        get_next()
    item_draw_func: (options, x, y) =>
        @sprite\draw options, {x, y}
}
