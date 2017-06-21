EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Display = require "core.Display"
DataW = require "DataWrapped"
LuaGameObject = require "objects.LuaGameObject"

M = nilprotect {
    _fire: tosprite "spr_effects.fire-anim"
}

spell_object_type = (T) ->
    wrapped_on_init = T.on_init
    T.init = (caster, time_left) =>
        @caster = assert caster
        @time_left = assert time_left
        @total_time = assert time_left
        T.parent_init(@, @caster.xy)
        if wrapped_on_init
            wrapped_on_init(@)
    T.in_time_slice = (frac1, frac2) =>
        time_passed = @total_time - @time_left
        return (time_passed >= frac1 * @total_time and time_passed < frac2 * @total_time)
    T.get_progress = (frac1, frac2) =>
        time_passed = @total_time - @time_left
        progress = time_passed / @total_time
        return math.max(0, math.min(progress, 1)

    wrapped_on_step = T.on_step
    T.on_step = () =>
        @xy = @caster.xy
        if @caster.destroyed or @time_left <= 0
            @caster = nil
            GameObject.destroy(@)
            return
        @time_left -= 1
        if wrapped_on_step
            wrapped_on_step(@)
    return LuaGameObject.type_create(T)


RingOfFireObject = spell_object_type {
--    on_step: () =>

    on_draw: () =>
        if @in_time_slice 0.1


    _position: () =>

}

DataW.spell_create {
    name: "Ring of Fire",
    description: ""
    types: {"Red"}
    prereq_func: (caster) -> return true
    autotarget_func: (caster) -> caster.x, caster.y
    action_func: (caster, x, y) ->
    projectile: {
        speed: 5
        damage_multiplier: 2.00
    }
    mp_cost: 10
    spell_cooldown: 400
    cooldown: 35
}

return M
