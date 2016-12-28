EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
Map = require "core.Map"
Bresenham = require "core.Bresenham"
SpellObjects = require "objects.SpellObjects"

Data.spell_create {
    name: "Ice Form"
    spr_spell: "spr_spells.iceform"
    description: "A very powerful ability for safe exploration. Initiates Ice Form, preventing attacks and spells, lowering speed drastically, but providing near immunity for 10 seconds." 
    mp_cost: 10
    cooldown: 100
    can_cast_with_held_key: false
    fallback_to_melee: false
    action_func: (caster, x, y) ->
        caster\add_effect("Ice Form", 300)
        if caster\is_local_player()
            EventLog.add("You enter a glacial state!", {200,200,255})
        elseif caster.name == "Your ally"
            EventLog.add(caster.name .. " enters a glacial state!", {200,200,255})
    autotarget_func: (caster) -> caster.x, caster.y
    prereq_func: (caster) -> 
        return not caster\has_effect("Berserk") and not caster\has_effect("Exhausted")  and not caster\has_effect("Ice Form")
}

Data.effect_create {
    name: "Ice Form"
    effected_sprite: "spr_spells.iceform"
    can_use_rest: false
    can_use_spells: false
    can_use_stairs: true -- Important!!
    can_use_weapons: false
    effected_colour: {200, 200, 255}
    stat_func: (effect, obj, old, new) ->
        new.defence += 20
        new.willpower += 20
        new.reduction += 20
        new.magic_reduction += 20
        new.mp, new.max_mp = 1,1
        new.speed /= 2
    fade_out: 55
}
