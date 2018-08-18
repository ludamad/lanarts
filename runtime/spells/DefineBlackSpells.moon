EventLog = require "ui.EventLog"
ObjectUtils = require "objects.ObjectUtils"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
World = require "core.World"
Bresenham = require "core.Bresenham"
Display = require "core.Display"
SpellObjects = require "objects.SpellObjects"
DataW = require "DataWrapped"
require "spells.DefineFireSpells"
require "spells.DefineWhiteSpells"
SummonUtils = require "spells.SummonUtils"
TypeEffectUtils = require "spells.TypeEffectUtils"

--DataW.effect_create {
--    name: "Ghostform"
--    can_use_rest: false
--    init_func: (obj) =>
--        @n_frames = 0
--        @old_sprite_name = obj.sprite_name
--        obj.sprite_name = 'spr_enemies.undead.freezing_wraith'
--    stat_func: (obj, old, new) =>
--        new.magic = math.max(new.magic - 4, 0)
--        new.strength = math.max(new.strength - 4, 0)
--        new.speed *= 0.5
--    remove_func: (obj) =>
--        obj.sprite_name = @old_sprite_name
--    step_func: (obj) =>
--        @n_frames += 1
--        if @n_frames % 100 == 0
--            play_sound "sound/lifelink.ogg"
--    on_receive_damage_func: (attacker, obj, dmg) =>
--        -- Negate all damage:
--        dmg = 0
--        return dmg
--}


--DataW.spell_create {
--    name: "Ghostform"
--    description: "A ghastly ethereal form. For a limited time, you become a ghost, impervious to damage, haunting all around you. Can be extended arbitrarily with mana cost."
--    types: {"Black"}
--    spr_spell: 'spr_enemies.undead.freezing_wraith'
--    prereq_func: (obj) -> return true
--    autotarget_func: (obj) -> obj.x, obj.y
--    action_func: (obj, x, y) ->
--        obj\add_effect "Ghostform", 10
--    on_channel_func: (obj, x, y) ->
--        effect = obj\add_effect "Ghostform", 1
--        if effect.n_frames % 10 == 0
--            obj.stats.mp = math.max(obj.stats.mp - 1, 0)
--    is_channeled_func: (obj, x, y) ->
--        return obj\has_effect("Ghostform") and obj.stats.mp > 0
--    console_draw_func: (get_next) =>
--        draw_console_effect get_next(), tosprite("spr_enemies.undead.freezing_wraith"), {
--            {COL_PALE_GREEN, "Immune to damage."}
--        }
--    mp_cost: 1
--    spell_cooldown: 100
--    cooldown: 0
--}

DataW.spell_create {
    name: "Skeleton Army"
    types: {"Black"}
    description: "Briefly summon an army of skeletons. Grow stronger based on caster's willpower.",
    can_cast_with_held_key: true
    spr_spell: "spr_spells.animate_skeleton"
    can_cast_with_cooldown: false
    mp_cost: 10
    cooldown: 25
    spell_cooldown: 400
    fallback_to_melee: false
    prereq_func: (obj) ->
        return true
    action_func: (obj) ->
        n_summoned = 0
        bpower = TypeEffectUtils.get_power(obj, "Black")
        ObjectUtils.nearby_square_iterate obj, (xy) ->
            mon = SummonUtils.summon_one_hit_monster(obj, xy, "SummonedSkeleton", 100 + bpower * 10)
            mon.stats.strength = obj.stats.willpower / 3
            n_summoned += 1
            return n_summoned >= 10 + obj.stats.level * 3 + bpower * 2
}

