EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
Display = require "core.Display"
DataW = require "DataWrapped"
ObjectUtils = require "objects.ObjectUtils"
SpellUtils = require "spells.SpellUtils"
TypeEffectUtils = require "spells.TypeEffectUtils"
{:draw_console_text, :draw_console_effect} = require 'ui.ConsoleUtils'

M = nilprotect {
    _fire: tosprite "spr_effects.fire-anim"
    _burning: tosprite "spr_effects.burning"
}

----------------- <RING OF FIRE IMPL> ---------------------
-- Utility class that does most of RingOfFire, leaving enough for its commonality
-- to implemented the SpawnedFire's that result from killing enemies with RingOfFire
RingFireBase = (extension) -> SpellUtils.spell_object_type table.merge {
    types: {"Red"}
    base_init: (args) =>
        assert @_damage
        -- Damage cooldowns by individual flame
        @cooldown = args.cooldown or 20
    _on_kill: (obj) => nil -- Default do nothing
    _on_damage: (obj) => nil
    base_on_step: () =>
        if @caster.is_ghost
            GameObject.destroy(@)
            return
        if @caster.destroyed or @caster.map ~= @map
            return
        if @every 20
            play_sound "sound/ringfire-loop.ogg"
        @for_all_rings (x, y, damage_cooldown) ->
            for obj in *Map.radius_collision_check(@map, 15, {x, y})
                if damage_cooldown[obj] or (obj.team == nil) or obj.team == @caster.team
                    continue
                if not obj.destroyed
                    if @do_damage(obj, @_damage)
                        @_on_kill(obj)
                    @_on_damage(obj, @_damage)
                --    @message "You gain mana from killing #{mon_name}!", COL_PALE_BLUE
                --    @caster\heal_mp(5)
                play_sound "sound/ringfire-hit.ogg"
                damage_cooldown[obj] = @cooldown
            for k,v in strictpairs damage_cooldown
                v -= 1
                damage_cooldown[k] = (if v > 0 then v else nil)
    on_step: () => @base_on_step()
    on_draw: () =>
        if @caster.destroyed or @caster.map ~= @map
            return
        alpha = 1.0
        if @in_time_slice(0.0, 0.1)
            alpha = @get_progress(0.0, 0.1)
        elseif @in_time_slice(0.9, 1)
            alpha = 1.0 - @get_progress(0.9, 1)
        frame = @time_passed()
        @for_all_rings (x, y) ->
            ObjectUtils.screen_draw(M._fire, {x, y}, alpha / 2, frame)
}, extension
M.RingOfFire = RingOfFire

-- When something dies from RingOfFire, this is spawned
local SpawnedFire
SpawnedFire = RingFireBase {
    -- RingFireBase config
    _damage: 0.2
    init: (args) =>
        @damage_cooldowns = OrderedDict() -- takes [dx][dy][obj]
        @initial_duration = args.duration
        @fire_radius = args.fire_radius or 1
        @base_init(args)
    _on_kill: (obj) =>
        --r_pow = TypeEffectUtils.get_power(@caster, 'Red')
        ---- Augment chain fire chance with Red power
        --if chance(.1 + 0.4 * bounds_percentage(r_pow, 0, 4))
        --    mon_name = SpellUtils.mon_title(obj)
        --    SpellUtils.message(@caster, "Fire springs forth from the defeated #{mon_name}!", COL_PALE_BLUE)
        --    GameObject.add_to_level SpawnedFire.create {
        --        caster: @caster
        --        xy: obj.xy -- Create around the damaged enemy
        --        duration: 40
        --    }
        ---- Very rare: Spawned fire creates mana potion.
        --if chance(0.01 * bounds_percentage(r_pow, 2, 4))
        --    SpellUtils.message(@caster, "Your awesome fire magic creates a red mana potion!", COL_PALE_BLUE)
        --    ObjectUtils.spawn_item_near(@caster, "Red Mana Potion", 1, obj.x, obj.y)
        nil -- Experiment: Don't chain on deaths

    _on_damage: (obj) =>
        divider = if @fire_radius == 0 then 1 else 3
        if chance(.05 / divider)
            mon_name = SpellUtils.mon_title(obj)
            SpellUtils.message(@caster, "Suddenly, #{mon_name} catches on fire!", COL_PALE_RED)
            GameObject.add_to_level SpawnedFire.create {
                caster: @caster
                fire_radius: 0 -- @fire_radius
                xy: obj.xy -- Create around the damaged enemy
                cooldown: @cooldown
                duration: @initial_duration
            }

    for_all_rings: (f) =>
        rad = @fire_radius
        for dy=-rad,rad
            for dx=-rad,rad
                if math.abs(dx) + math.abs(dy) == 2
                    continue
                {cx, cy} = @xy
                x, y = cx + dx * Map.TILE_SIZE, cy + dy * Map.TILE_SIZE
                if not @caster\within_field_of_view({x, y})
                    continue
                @damage_cooldowns[dy] or= OrderedDict()
                @damage_cooldowns[dy][dx] or= OrderedDict()
                f(x, y, @damage_cooldowns[dy][dx])
}
M.SpawnedFire = SpawnedFire

RingOfFire = RingFireBase {
    -- RingFireBase config
    _damage: 0.5
    init: (args) =>
        @damage_cooldowns = OrderedDict() -- takes [index][obj]
        @fireballs = {}
        @base_init(args)
    _create_rings: (radius_bonus = 0) =>
        sub_radius = 20 -- / (1 + @caster_type_power() / 10)
        n_points = 14 + @caster_type_power()
        radius = sub_radius * n_points / (2 * math.pi) + radius_bonus
        step_angle = (2 * math.pi) / n_points
        return for i=0,n_points-1
            angle = step_angle * i
            {math.sin(angle) * radius, math.cos(angle) * radius}
    -- Extra functionality over RingFireBase: creating SpawnedFire
    _on_kill: (obj) =>
        --mon_name = SpellUtils.mon_title(obj)
        --SpellUtils.message(@caster, "Fire springs forth from the defeated #{mon_name}!", COL_PALE_BLUE)
        --GameObject.add_to_level SpawnedFire.create {
        --    caster: @caster
        --    xy: obj.xy -- Create around the damaged enemy
        --    duration: 40
        --}
        --r_pow = TypeEffectUtils.get_power(@caster, 'Red')
        ---- Very rare: Ring of fire creates mana potion.
        --if chance(0.01 * bounds_percentage(r_pow, 2, 4))
        --    SpellUtils.message(@caster, "Your awesome fire magic creates a red mana potion!", COL_PALE_BLUE)
        --    ObjectUtils.spawn_item_near(@caster, "Red Mana Potion", 1, obj.x, obj.y)
        nil
    _on_damage: (obj) =>
        nil
    -- GameObject methods
    -- Extra functionality over RingFireBase: Manage caster's vision radius
    on_map_deinit: () =>
        if @caster ~= nil and not @caster.destroyed
            @caster.__vision_radius_override = nil -- Stop overriding vision radius
    on_step: () =>
        if @caster.destroyed or @caster.map ~= @map
            return
        @caster.__vision_radius_override = 9
        @base_on_step()

    for_all_rings: (f) =>
        radius_percentage = nil
        for i=1,10
            if @percent_passed() <= i / 10
                radius_percentage = @get_progress((i-1)/10,i/10)
                break
        rings = @_create_rings(30 - 30 * radius_percentage)
        for idx=1,#rings
            {x, y} = rings[idx]
            @damage_cooldowns[idx] or= OrderedDict()
            f(x + @caster.x, y + @caster.y, @damage_cooldowns[idx])
}
M.RingOfFire = RingOfFire

DataW.effect_create {
    name: "Ring of Flames Stat Boost"
    stat_func: (effect, obj, old, new) ->
        new.defence += 4
        new.willpower += 4
}

DataW.spell_create {
    name: "Ring of Flames",
    description: ""
    types: {"Red"}
    spr_spell: "spr_spells.spell_icon_ring_of_flames"
    prereq_func: (caster) -> return true
    autotarget_func: (caster) -> caster.x, caster.y
    action_func: (caster, x, y) ->
        duration = 350 + 100 * bounds_percentage(TypeEffectUtils.get_power(caster, 'Red'), 0, 4)
        GameObject.add_to_level RingOfFire.create({:caster, :duration})
        caster\add_effect "Ring of Flames Stat Boost", duration
    console_draw_func: (get_next) =>
        STAT_BOOST = 2  -- TODO playing with this constant
        damage = math.floor(RingOfFire._damage * 60 / 5 * STAT_BOOST)
        power = @effective_stats().magic + TypeEffectUtils.get_power(@, "Red")
        draw_console_effect get_next(), tosprite("spr_spells.spell_icon_ring_of_flames"), {
            {COL_PALE_GREEN, damage}
            {COL_PALE_YELLOW, " damage per second"}
        }
        get_next() -- Leave a gap
        draw_console_text get_next(), {
            {COL_PALE_YELLOW, "Power: "}
            {COL_PALE_GREEN, power}
        }
    mp_cost: 20
    spell_cooldown: 800
    cooldown: 100
}
----------------- </RING OF FIRE IMPL> ---------------------


----------------- <INNER FIRE IMPL> ---------------------
DataW.effect_create {
    name: "Dragonform"
    can_use_rest: false
    fade_out: 5
    effected_colour: {255,160,160}
    effected_sprite: "spr_effects.inner_flame"
    apply_func: (caster, time_left) =>
        @kill_tracker = caster.kills
        @max_time = math.max(@max_time or 0, time_left)
        @extensions = 0
        -- TODO go farther, have claw as weapon during.
        @fire_sprite_name = caster.sprite_name
        caster.sprite_name = 'spr_enemies.dragons.fire_dragon'
    stat_func: (obj, old, new) =>
        new.defence += 6
        new.strength += 6
        new.willpower += 6
        new.speed *= 0.8
    remove_func: (caster) =>
        --if @initial_state
        caster.sprite_name = @fire_sprite_name --@initial_state.sprite_name
        --    caster.target_radius = @initial_state.target_radius
    step_func: (caster) =>
        time_passed = @max_time - @time_left
        if time_passed % 20 == 0
            play_sound "sound/ringfire-loop.ogg"
        diff = math.max(caster.kills - @kill_tracker, 0)
        for i=1,diff
            @time_left = math.min(@max_time * 1.5, @time_left + 10)
            for _ in screens()
                if caster\is_local_player()
                    EventLog.add("Your inner fire grows ...", {200,200,255})
                    --play_sound "sound/berserk.ogg"
            @extensions += 1
        @kill_tracker = caster.kills
}

DataW.spell_create {
    name: "Dragonform"
    description: "A form of great fire. For a limited time, you become a slow but immensely powerful dragon."
    types: {"Red"}
    spr_spell: 'spr_enemies.dragons.fire_dragon'
    prereq_func: (caster) -> return true
    autotarget_func: (caster) -> caster.x, caster.y
    action_func: (caster, x, y) ->
        caster\add_effect "Dragonform", 300 + 200 * bounds_percentage(caster.stats.level, 0, 10)
    console_draw_func: (get_next) =>
        draw_console_effect get_next(), tosprite("fire bolt"), {
            {COL_PALE_GREEN, "4x Fire Bolts"}
        }
        draw_console_effect get_next(), tosprite("spr_effects.fire-anim"), {
            {COL_PALE_GREEN, "Fire Bolts explode"}
        }
    mp_cost: 30
    spell_cooldown: 1600
    cooldown: 0
}

----------------- </INNER FIRE IMPL> ---------------------

FLAME_RATE = 30
MAX_INTENSITY = 60 * 20 -- 20 seconds
INTENSITY_SPREAD_INTERVAL = MAX_INTENSITY / 3

get_burn_intensity = (inst) ->
    if inst\has_effect("Burn")
        return inst\get_effect("Burn").intensity
    else
        return 0

apply_raw_burn = (inst, attacker, intensity) ->
    inst\add_effect("Burn", {:intensity, :attacker, time_left: math.huge})

apply_burn_intensity = (inst, attacker, intensity) ->
    -- Over 10 seconds, we will lose that much life.
    -- Intensity determined by how much % of our life has been taken * MAX_INTENSITY
    flammability = inst.def.flammability or 1
    intensity *= flammability
    apply_raw_burn(inst, attacker, intensity)

apply_burn_damage = (inst, attacker, damage) ->
    -- Calculate burn intensity from damage done
    percent_damage = damage / inst.stats.max_hp
    intensity = percent_damage * MAX_INTENSITY * 4
    --intensity = INTENSITY_SPREAD_INTERVAL
    apply_burn_intensity(inst, attacker, intensity)

DataW.effect_create {
    -- Burn is controlled by 2 variables:
    -- - Intensity, from 0 to 300.
    -- -    - Shown as one flame, 2 flame, 3 flame etc based on rounding down after dividing by 100.
    -- - Flammability, 0 to 1+
    -- -    - Scaling factor. Per enemy. Generally 0.5 for red/blue creatures, 1.5 for green.
    -- -    - Used above in apply_burn.
    name: "Burn"
    can_use_rest: false
    effected_colour: {255,160,160}
    effected_sprite: "spr_effects.inner_flame"
    fade_out: 5
    init_func: (inst) =>
        @intensity = 0
        @flammability = inst.def.flammability
        @n_steps = 0
    apply_func: (inst, args) =>
        @attacker = assert args.attacker
        @intensity = math.min(@intensity + args.intensity, MAX_INTENSITY)
    step_func: (inst) =>
        -- Intensity goes down by 1 a turn, to a maximum of 0
        @intensity -= 1
        @intensity = math.max(@intensity, 0)
        @n_steps += 1
        if @intensity > INTENSITY_SPREAD_INTERVAL and @n_steps % FLAME_RATE == 0
            -- At 300 intensity, the target loses 5% of their max HP every half second.
            -- At 100 intensity, the target loses 1.25% of their max HP every half second.
            percentage_intensity = @intensity / MAX_INTENSITY
            percentage_loss = percentage_intensity / 20
            damage = percentage_loss * inst.stats.max_hp
            @spread_fire(inst)
            --animation = GameObject.animation_create {
            --    xy: inst.xy
            --    sprite: "spr_effects.fire-anim"
            --    duration: 25
            --}
            --animation.on_step = () => @xy = inst.xy
            -- Make sure to destroy object only AFTER fire spreading
            inst\direct_damage(damage, @attacker)
    spread_fire: (inst) =>
        -- Called every half second.
        -- When INTENSITY_SPREAD_INTERVAL more intensity than a nearby enemy and they are less than 32px away...
        --      - 90% chance, do nothing
        --      - otherwise, increase nearby enemy intensity by INTENSITY_SPREAD_INTERVAL, up to our intensity - INTENSITY_SPREAD_INTERVAL
        induced_intensity = @intensity - INTENSITY_SPREAD_INTERVAL
        for obj in *Map.radius_collision_check(inst.map, 32 + inst.target_radius, inst.xy)
            if (obj.team == nil) or obj.team == @attacker.team
                continue -- Not eligible
            obj_intensity = get_burn_intensity(obj)
            if induced_intensity <= obj_intensity
                continue -- Cant induce intensity, lesser induced intensity
            --if chance(0.9)
            --    continue -- 90% chance to fail to do anything
            new_intensity = math.min(induced_intensity, obj_intensity + INTENSITY_SPREAD_INTERVAL)
            apply_burn_intensity obj, @attacker, (new_intensity - obj_intensity) -- delta
    draw_func: (inst) =>
        --alpha = 1.0
        --if @in_time_slice(0.0, 0.1)
        --    alpha = @get_progress(0.0, 0.1)
        --elseif @in_time_slice(0.9, 1)
        --    alpha = 1.0 - @get_progress(0.9, 1)
        for i=1,3
            if @intensity > INTENSITY_SPREAD_INTERVAL * i
                {x, y} = inst.xy
                -- place in top left
                x += inst.radius / 2 + 4
                y += inst.radius / 2 + 4
                ObjectUtils.screen_draw(M._burning, {x+ i*8, y}, 1, @n_steps)
            --ObjectUtils.screen_draw(M._fire, inst.xy, 0.8, @n_steps)
            --font_cached_load(settings.font, 20)\draw {
            --    color: COL_WHITE
            --    origin: Display.CENTER
            --}, Display.to_screen_xy(inst.xy), tostring(@intensity)
}

-- FIRE BOLT
DataW.spell_create {
    name: "Fire Bolt",
    spr_spell: "fire bolt",
    description: "A fast bolt of fire. Hits a single target.",
    types: {"Red"}
    projectile: {
        speed: 7
        damage_multiplier: 1.2
        spr_attack: "fire bolt"
        on_damage: (target, damage) =>
            if @caster.destroyed
                return
            --apply_burn_damage(target, @caster, damage)
            r_pow = TypeEffectUtils.get_power(@caster, 'Red')
            if @caster\has_effect("Dragonform")
                SpellUtils.message(@caster, "Fire springs forth from the fire bolt!", COL_PALE_BLUE)
                GameObject.add_to_level SpawnedFire.create {
                    fire_radius: 1
                    caster: @caster
                    xy: @xy
                    :damage
                    duration: 80
                }
            elseif chance(0.1 + 0.02 * r_pow)
                SpellUtils.message(@caster, "Fire springs forth from the fire bolt!", COL_PALE_BLUE)
                GameObject.add_to_level SpawnedFire.create {
                    fire_radius: 0
                    caster: @caster
                    xy: @xy
                    :damage
                    duration: 200
                }

    }
    mp_cost: 10,
    cooldown: 35
}

return M
