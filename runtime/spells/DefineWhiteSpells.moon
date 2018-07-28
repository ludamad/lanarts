EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
Display = require "core.Display"
DataW = require "DataWrapped"
ObjectUtils = require "objects.ObjectUtils"
SpellUtils = require "spells.SpellUtils"
TypeEffectUtils = require "spells.TypeEffectUtils"

M = nilprotect {
    _flash: tosprite "spr_effects.tornado"
    _bolt: tosprite "spr_spells.spell_icon_lightning_bolt"
}

-- A standardized stun for white mage:
try_stun = (caster, mon) ->
    white_power = TypeEffectUtils.get_power(caster, "White")
    resist = math.min(1.25, TypeEffectUtils.get_resistance(mon, "White")) -- Max 25%+ on the spell
    if not mon.is_enemy
        resist = 0.5
    if chance(resist)
        if not mon.is_enemy
            white_power = -5
        mon\add_effect("Stunned", (100 + white_power * 10) * resist)
        mon\add_effect("Stunned", (100 + white_power * 10) * resist)
        play_sound "sound/ringfire-hit.ogg"

Flash = SpellUtils.spell_object_type {
    types: {"White"}
    init: (args) =>
        @_damage = args.damage
        -- Damage cooldowns by individual aspects
        @cooldown = args.cooldown or 50
        @damage_cooldowns = {}
    _on_kill: (obj) => nil -- Default do nothing
    on_step: () =>
        if @caster.is_ghost
            GameObject.destroy(@)
            return
        if @caster.destroyed or @caster.map ~= @map
            return
        @for_all_rings (x, y, damage_cooldown) ->
            for mon in *Map.radius_collision_check(@map, 15, {x, y})
                if damage_cooldown[mon] or (mon.team == nil) or mon.team == @caster.team
                    continue
                if not mon.destroyed and @do_damage(mon, @_damage)
                    @_on_kill(mon)
                try_stun(@caster, mon)
                damage_cooldown[mon] = @cooldown
            for k,v in pairs damage_cooldown
                v -= 1
                damage_cooldown[k] = (if v > 0 then v else nil)
    for_all_rings: (f) =>
        rad = 1
        for dy=-rad,rad
            for dx=-rad,rad
                if dx*dx + dy*dy > rad*rad
                    continue
                {cx, cy} = @xy
                x, y = cx + dx * Map.TILE_SIZE, cy + dy * Map.TILE_SIZE
                if not @caster\within_field_of_view({x, y})
                    continue
                @damage_cooldowns[dy] or= {}
                @damage_cooldowns[dy][dx] or= {}
                f(x, y, @damage_cooldowns[dy][dx])
    on_draw: () =>
        if @caster.destroyed or @caster.map ~= @map
            return
        alpha = 1.0
        if @in_time_slice(0.0, 0.5)
            alpha = @get_progress(0.0, 0.5)
        elseif @in_time_slice(0.5, 1)
            alpha = 1.0 - @get_progress(0.5, 1)
        else
            alpha = 0
        frame = @time_passed()
        @for_all_rings (x, y) ->
            --ObjectUtils.screen_draw(M._flash, {x, y}, alpha / 2, frame)
            ObjectUtils.screen_draw(M._bolt, {x, y}, alpha / 2, frame)
}

interpolate = (val, min, max, omin, omax) ->
    val = math.max(min, math.min(max, val))
    w1 = max - min
    w2 = omax - omin
    return (val - min) * (w2 / w1) + omin

-- CHAIN LIGHTNING
DataW.spell_create {
    name: "Chain Lightning",
    description: "A slow, powerful blast of lightning. The blast can bounce off an enemy twice before dissipating.",
    spr_spell: "spr_spells.spell_icon_chain_lightning",
    projectile: {
        types: {"White"}
        spr_attack: "charge_effected",
        speed: 4
        number_of_target_bounces: 3
        on_hit_func: (target, atkstats) =>
            if @caster
                return
            white_power = TypeEffectUtils.get_power(@caster, "White")
            prob = interpolate(white_power, 0, 5, 0.0, 0.2)
            if chance(prob)
                try_stun(@caster, target)
            
    }
    mp_cost: 20,
    cooldown: 65
    spell_cooldown: 65
}

-- ENERGY SPEAR
-- A basic elemental spell that goes through multiple enemies.
DataW.spell_create {
    name: "Energy Spear",
    description: "Allows you to create a durable spear of lightning.",
    spr_spell: "spr_effects.lightningbolt"
    types: {"White"}
    projectile: SpellUtils.passing_projectile {
        speed: 5
        damage_multiplier: 1
        n_steps: 500 / 8
        can_wall_bounce: true
        redamage_cooldown: 40 -- Cooldown for when enemies are damaged again by effect
        on_map_init: () =>
            @bonus = 0.9
        on_hit_func: (target, atkstats) =>
            if @caster
                return
            white_power = TypeEffectUtils.get_power(@caster, "White")
            prob = interpolate(white_power, 0, 5, 0.0, 0.2)
            if chance(prob)
                try_stun(@caster, target)
            @bonus += 0.1
            atkstats.damage *= @bonus
    }
    mp_cost: 12
    cooldown: 35
}

draw_console_text = (xy, texts) ->
    {x, y} = xy
    for {color, text} in *texts
        x += font_cached_load(settings.font, 10)\draw {
            :color
            origin: Display.LEFT_CENTER
        }, {x, y}, text 
    return nil


draw_console_effect = (xy, sprite, texts) ->
    {x, y} = xy
    sprite\draw {
        origin: Display.LEFT_CENTER
    }, {x, y + 4}
    draw_console_text {x + Map.TILE_SIZE + 4, y}, texts


-- TORNADO STORM
DataW.spell_create {
    name: "Tornado Storm",
    description: "Tornados of swirling death.",
    spr_spell: "spr_effects.tornado"
    types: {"White"}
    console_draw_func: (get_next) =>
        draw_console_effect get_next(), M._flash, {
            {COL_PALE_GREEN, "Tornados burst into lightning"}
        }
        draw_console_effect get_next(), M._bolt, {
            {COL_PALE_GREEN, "5"}
            {COL_PALE_YELLOW, " damage per second"}
        }
    projectile: SpellUtils.passing_projectile {
        speed: 4
        damage_multiplier: 0.9
        n_steps: 30
        can_wall_bounce: false
        on_map_init: () =>
            @cx, @cy = @caster.x, @caster.y
        on_step: () =>
            if @map ~= @caster.map
                GameObject.destroy(@)
                return
            dx, dy = @caster.x - @cx, @caster.y - @cy
            --@x += random(-1, 1) + dx
            --@y += random(-1, 1) + dy
            @x += dx
            @y += dy
            @cx, @cy = @caster.x, @caster.y
        on_deinit: () => 
            GameObject.add_to_level Flash.create({caster: @caster, duration: 30, damage: 10, xy: @xy})
        redamage_cooldown: 40 -- Cooldown for when enemies are damaged again by effect
    }
    mp_cost: 25,
    cooldown: 35
    spell_cooldown: 300
}

DataW.effect_create {
    name: "Stunned"
    effected_sprite: "spr_amulets.light"
    effected_colour: {200,200,200}
    can_use_rest: false
    can_use_spells: false
    can_use_stairs: false
    can_use_weapons: false
    apply_func: (obj) =>
        @ox,@oy = obj.x, obj.y
    step_func: (obj) =>
        obj.x, obj.y = @ox, @oy
}

--WHITE_TYPES = {"White"}
---- Blinding Light
--DataW.spell_create {
--    name: "Blinding Light"
--    types: {"White"}
--    spr_spell: "spr_amulets.light"
--    damage_type: {magic: 0.5, physical: 0.5}
--    description: "Thunder rains from above."
--    mp_cost: 20
--    cooldown: 0
--    spell_cooldown: 1600
--    can_cast_with_held_key: true
--    fallback_to_melee: false
--    action_func: (caster, x, y) ->
--        GameObject.add_to_level Flash.create({:caster, duration: 100, damage: 10})
--        --white_power = TypeEffectUtils.get_power(caster, "White")
--        --for_all_visible_mons caster, (mon) ->
--        --    resist = math.min(1.25, TypeEffectUtils.get_resistance(mon, "White")) -- Max 25%+ on the spell
--        --    mon\add_effect("Stunned", (100 + white_power * 10) * resist)
--        --SpellUtils.message(caster, "You try to daze all enemies in sight!", {200,200,255})
--        ----caster\add_effect("Blinding Light", 500 + TypeEffectUtils.get_power(caster,"White")*100)
--        play_sound "sound/ludaze.ogg"
--    autotarget_func: (caster) -> caster.x, caster.y
--    prereq_func: (caster) -> 
--        for mon in *(Map.enemies_list caster)
--            if Map.object_visible(mon, mon.xy, caster)
--                return true
--        EventLog.add("No monsters in sight!", COL_PALE_RED)
--        return false
--}


return M
