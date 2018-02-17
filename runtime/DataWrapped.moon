EffectUtils = require "spells.EffectUtils"
GameObject = require "core.GameObject"

draw_console_effect = (sprite, text, xy, color = COL_PALE_YELLOW) ->
    Map = require "core.Map"
    Display = require "core.Display"
    sprite\draw {
        origin: Display.LEFT_CENTER
    }, {xy[1], xy[2] + 4}
    font_cached_load(settings.font, 10)\draw {
        :color
        origin: Display.LEFT_CENTER
    }, {xy[1] + Map.TILE_SIZE + 4, xy[2]}, text

-- Normal effect. By default, takes the 'max' of all applied time_left's
effect_create = (args) ->
    wrapped_init = args.init_func
    wrapped_step = args.step_func
    wrapped_apply = args.apply_func
    wrapped_remove = args.remove_func
    args.init_func = (obj) =>
        @time_left = 0
        @active = false
        @n_derived = 0
        if wrapped_init
            wrapped_init(@, obj)
    args.apply_derived_func or= (obj, args) =>
        @active = true
        @n_derived += 1
        if wrapped_apply
            wrapped_apply(@, obj, args)
    args.remove_derived_func or= (obj, args) =>
        @n_derived = 0
        @active = (@time_left > 0)
    args.apply_buff_func or= (obj, args) =>
        @active = true
        if wrapped_apply
            wrapped_apply(@, obj, args)
        time_left = if type(args) == 'number' then args else args.time_left
        @time_left = math.max(@time_left, time_left)
    args.remove_func = (obj) =>
        if wrapped_remove
            wrapped_remove(@, obj)
        @init_func(obj)
    args.step_func = (obj) =>
        if wrapped_step
            wrapped_step(@, obj)
        @time_left = math.max(@time_left - 1, 0)
        if @time_left <= 0 and @n_derived == 0
            @remove_func(obj)
            @time_left = 0
    Data.effect_create(args)
    yield_point()

-- List of subeffects, with an accumulated stat.
_subeffect_effect_create = (args, starting_value, accum) ->
    wrapped_init = args.init_func
    wrapped_step = args.step_func
    wrapped_apply = args.apply_func
    wrapped_remove = args.remove_func

    args.init_func = (obj) =>
        @active = false
        @subeffects = {}
        @current = starting_value
        @time_left = 0
    args.apply_derived_func or= (obj, args) =>
        @active = true
        @current = accum(@current, args)
    args.remove_derived_func or= (obj, args) =>
        @active = (#@subeffects > 0)
        @current = starting_value
    args.apply_buff_func or= (obj, args) =>
        @active = true
        assert(type(args.time_left) == 'number')
        append @subeffects, table.clone(args)
    args.remove_func = (obj) =>
        @active = false
        table.clear @subeffects
        @current = starting_value
    args._get_value = (obj) =>
        x = @current
        for eff in *@subeffects
            x = accum(x, eff)
        return x
    args.step_func = (obj) =>
        -- Step subeffects, remove those at t=0
        filter = false
        for eff in *@subeffects
            if wrapped_step
                wrapped_step(@, obj, eff)
            eff.time_left = math.max(eff.time_left - 1, 0)
            if eff.time_left <= 0
                filter = true
        if filter
            @subeffects = table.filter(@subeffects, (x) -> x.time_left > 0)
        -- If nothing left to this effect, remove it (make it inactive)
        if @subeffects == 0 and @current ~= starting_value
            @remove_func(obj)
    Data.effect_create(args)
    yield_point()

additive_effect_create = (args) ->
    key = args.key or 'value'
    accum = (x, next) -> x + next[key]
    return _subeffect_effect_create(args, 0, accum)

STANDARD_WEAPON_DPS = 10
STANDARD_RANGED_DPS = 7

add_console_draw_func = (args, f1) ->
    f2 = args.console_draw_func
    if f2 == nil
        args.console_draw_func = f1
    else
        args.console_draw_func = (...) ->
            f1(...)
            f2(...)

add_on_hit_func = (args, f1) ->
    f2 = args.on_hit_func
    if f2 == nil
        args.on_hit_func = f1
    else
        args.on_hit_func = (obj, target, atkstats) ->
            f1(obj, target, atkstats)
            f2(obj, target, atkstats)

add_attack_stat_func = (args, f1) ->
    f2 = args.attack_stat_func
    if f2 == nil
        args.attack_stat_func = f1
    else
        args.attack_stat_func = (obj, target, atkstats) ->
            f1(obj, target, atkstats)
            f2(obj, target, atkstats)

add_cooldown_multiplier = (args, f1) ->
    f2 = args.cooldown_multiplier
    if f2 == nil
        args.cooldown_multiplier = f1
    else
        args.cooldown_multiplier = () =>
            f1(@)
            f2(@)

add_types = (args, types) ->
    for type in *types
        assert table.contains(EffectUtils.TYPES, type), "Invalid type!"
        add_on_hit_func args, (obj, target, atkstats) ->
            atkstats.power += EffectUtils.get_power(obj, type)
            atkstats.type_multiplier *= EffectUtils.get_resistance(target, type)

power_effects = (powers, effects = {}) ->
    for type, power in pairs powers
        assert table.contains(EffectUtils.TYPES, type), "Invalid type!"
        if power == 0 -- Special case for simplifying code-generated content
            continue
        append effects, {"#{type}Power", {:power}}
    return effects

resistance_effects = (resists, effects = {}) ->
    for type, resist in pairs resists
        assert table.contains(EffectUtils.TYPES, type), "Invalid type!"
        if resist == 0 -- Special case for simplifying code-generated content
            continue
        append effects, {"#{type}Resist", {:resist}}
    return effects

weapon_create = (args, for_enemy = false) ->
    damage_multiplier = args.damage_multiplier or 1.0
    dps = if args.type == "bows" then STANDARD_RANGED_DPS else STANDARD_WEAPON_DPS
    damage = damage_multiplier * (args.cooldown / 60 * dps)
    power = 0
    -- For enemy_create made weapons, directly set base damage:
    if type(args.damage) == 'number'
        damage = args.damage
        args.damage = {base: {math.floor(damage *0.9), math.ceil(damage * 1.1)}, strength: 0}
    else
        args.damage or= {base: {math.floor(damage), math.ceil(damage)}, strength: 0}
    if args.types ~= nil
        add_types args, args.types
    args.power or= {base: {power, power}, strength: 1}
    args.range or= 7
    for type in *(args.types or {})
        add_console_draw_func args, (inst, get_next) ->
            E = require "spells.Effects"
            switch type
                when "Red"
                    draw_console_effect E._fire_power, "Red Type", get_next()
                when "Blue"
                    draw_console_effect E._ice_power, "Blue Type", get_next()
                when "White"
                    draw_console_effect E._storm_power, "White Type", get_next()
                when "Black"
                    draw_console_effect E._black_power, "Black Type", get_next()
                when "Green"
                    draw_console_effect E._poison_power, "Green Type", get_next()
    Data.weapon_create(args)
    yield_point()

spell_create = (args) ->
    proj = args.projectile
    if proj
        damage_multiplier = proj.damage_multiplier or 1.0
        damage = damage_multiplier * (args.cooldown / 60 * STANDARD_RANGED_DPS)
        proj.name or= args.name
        proj.weapon_class or= "magic"
        proj.spr_item or= "none"
        proj.spr_attack or= args.spr_spell
        proj.cooldown or= args.cooldown
        proj.range or= 300
        proj.types or= args.types
        proj.damage_type or= {magic: 1.0}
        proj.damage or= {base: {math.floor(damage), math.ceil(damage)}, strength: 0}
        proj.power or= {base: {0, 0}, magic: 1}
        if proj.types ~= nil
            add_types proj, proj.types
        Data.projectile_create(proj)
        yield_point()
        args.projectile = proj.name
    if args.types
        for type in *args.types
            add_cooldown_multiplier args, () =>
                return 1 - EffectUtils.get_power(@, type) * 0.03
    for type in *(args.types or {})
        add_console_draw_func args, (caster, get_next) ->
            E = require "spells.Effects"
            switch type
                when "Red"
                    draw_console_effect E._fire_power, "Red Type", get_next()
                when "Blue"
                    draw_console_effect E._ice_power, "Blue Type", get_next()
                when "White"
                    draw_console_effect E._storm_power, "White Type", get_next()
                when "Black"
                    draw_console_effect E._black_power, "Black Type", get_next()
                when "Green"
                    draw_console_effect E._poison_power, "Green Type", get_next()
    Data.spell_create(args)
    yield_point()

projectile_create = (args, for_enemy = false) ->
    if args.cooldown
        damage_multiplier = args.damage_multiplier or 1.0
        damage = damage_multiplier * (args.cooldown / 60 * STANDARD_WEAPON_DPS)
        if for_enemy
            -- For enemies, we want all damage to come from 'damage'.
            -- The strength and magic stats work differently for enemies thusly.
            args.power or= {base: {0, 0}}
            args.damage or= {base: {0, 0}, strength: args.damage_type.physical, magic: args.damage_type.magic}
        else
            --damage = damage_multiplier * (args.cooldown / 60 * args.damage)
            args.damage or= {base: {math.floor(damage), math.ceil(damage)}, strength: 0}
            args.power or= {base: {0, 0}, strength: args.damage_type.physical, magic: args.damage_type.magic}
    args.spr_item or= "none"
    args.range or= 300
    if args.types ~= nil
        add_types args, args.types
    Data.projectile_create(args)
    yield_point()

-- Ensures derived enemy types are reachable:
DERIVED_ENEMY_TYPES = {}

-- High-level enemy definition function
-- Provides behind-the-scenes metatable manipulation for custom methods.
-- Adds suitable effects to the enemy based on types / type resistances.
-- Creates necessary eg weapon entries inline in the monster definition.
enemy_create = (args) ->
    args.stats.attacks or= {}

    -- If an inline weapon definition is given, create a weapon entry
    w = args.weapon
    if w ~= nil
        w.name or= args.name .. " Melee"
        w.type or= "unarmed"
        w.spr_item or= "none"
        w.types or= args.types
        append args.stats.attacks, {weapon: w.name}
        weapon_create(w, true)

    -- If an inline projectile definition is given, create a projectile entry
    p = args.projectile
    if p ~= nil
        p.name or= args.name .. " Projectile"
        p.spr_item or= "none"
        append args.stats.attacks, {projectile: p.name}
        p.types or= args.types
        projectile_create(p, true)

    -- If no resistances provided, derive based on provided types
    if args.resistances == nil and args.types ~= nil
        args.resistances = EffectUtils.get_monster_resistances(args.types)

    -- Compile resistances into effects
    if args.resistances ~= nil
        args.effects_active or= {}
        resistance_effects(args.resistances, args.effects_active)

    -- Update enemy information drawing based on provided types
    for type in *(args.types or {})
        add_console_draw_func args, (edata, get_next) ->
            E = require "spells.Effects"
            switch type
                when "Red"
                    draw_console_effect E._fire_power, "Red Type", get_next()
                when "Blue"
                    draw_console_effect E._ice_power, "Blue Type", get_next()
                when "White"
                    draw_console_effect E._storm_power, "White Type", get_next()
                when "Black"
                    draw_console_effect E._black_power, "Black Type", get_next()
                when "Green"
                    draw_console_effect E._poison_power, "Green Type", get_next()

    -- Are there methods we want to make available on the enemy object?
    if args.methods
        -- (1) Ensure that the 'methods' argument is being used properly
        for k,v in pairs args.methods
            assert type(k) == "string", "Method key should be string!"
            assert type(v) == "function", "Method value should be function!"

        -- (2) Derive from the EnemyType metatable
        derived_type = GameObject.EnemyType.clone()
        table.merge_into(derived_type.__constants, args.methods)

        -- (3) Stash the derived EnemyType metatable so that serialization can reach it
        DERIVED_ENEMY_TYPES[args.name] = derived_type

        -- (4) Install the new metatable on initialization
        wrapped_init = args.init_func
        args.init_func = () =>
            setmetatable(@, derived_type)
            if __DEBUG_CHECKS
                for k,v in pairs args.methods
                    assert @[k] == v
            if wrapped_init
                wrapped_init(@)

    -- Finally, create the enemy entry
    Data.enemy_create(args)
    yield_point()

return {:additive_effect_create, :effect_create, :weapon_create, :spell_create, :projectile_create, :enemy_create, :resistance_effects, :power_effects, :add_types, :DERIVED_ENEMY_TYPES}
