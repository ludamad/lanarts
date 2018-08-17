EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
Display = require "core.Display"
DataW = require "DataWrapped"
LuaGameObject = require "objects.LuaGameObject"
ObjectUtils = require "objects.ObjectUtils"
TypeEffectUtils = require "spells.TypeEffectUtils"

do_damage = (caster, types, target, damage, power = 0, magic_percentage = 1) ->
    stats = caster\effective_stats()
    power += magic_percentage * stats.magic + (1 - magic_percentage) * stats.strength
    for type in *types
        power += TypeEffectUtils.get_power(caster, type)
        damage *= TypeEffectUtils.get_resistance(target, type)
    return target\damage(damage, power, magic_percentage, caster)

spell_object_type = (T) ->
    wrapped_on_init = T.init
    T.types or= {}
    T.init = (args) =>
        {:caster, :duration, :xy} = args
        @caster = assert caster
        @time_left = assert duration
        @total_time = assert duration
        T.parent_init(@, xy or @caster.xy)
        if wrapped_on_init
            wrapped_on_init(@, args)
    -- Time based utility functions
    T.time_passed = () => @total_time - @time_left
    T.percent_passed = () => @time_passed() / @total_time
    T.in_time_slice = (frac1, frac2) =>
        return (@time_passed() >= frac1 * @total_time and @time_passed() < frac2 * @total_time)
    T.every = (duration) =>
        return @time_passed() % duration == 0
    T.caster_type_power = () =>
        power = 0
        for type in *@types
            power += TypeEffectUtils.get_power(@caster, type)
        return power
    T.do_damage = (target, damage, power = 0, magic_percentage = 1) =>
        return do_damage(@caster, @types, target, damage, power, magic_percentage)

    T.message = (msg, color) =>
        for _ in screens()
            if @caster\is_local_player()
                EventLog.add(msg, color)
    T.get_progress = (frac1, frac2) =>
        progress = (@time_passed() / @total_time - frac1) / (frac2 - frac1)
        return math.max(0, math.min(progress, 1))

    wrapped_on_step = T.on_step
    T.on_step = () =>
        if @caster.destroyed or @time_left <= 0
            GameObject.destroy(@)
            return
        @time_left -= 1
        if wrapped_on_step
            wrapped_on_step(@)
    return LuaGameObject.type_create(T)

passing_projectile = (args) -> 
    base = {
        can_pass_through: true -- Custom projectile
        deals_special_damge: true -- Custom projectile
    }
    combined = table.merge base, args
    combined.on_map_init = () =>
        @n_steps = args.n_steps
        @attacked_map = OrderedDict()
        @n_hits = 0
        if args.on_map_init
            args.on_map_init(@)
    combined.on_step = () =>
        @n_steps -= 1
        if @n_steps <= 0
            GameObject.destroy(@)
            return
        @attacked_map or= OrderedDict()
        @n_hits or= 0
        for k,v in strictpairs @attacked_map
            if v == 0
                @attacked_map[k] = nil
            else
                @attacked_map[k] = v - 1
        if args.on_step
            args.on_step(@)
    combined.on_hit_func = (target, atkstats) =>
        if @destroyed or @attacked_map[target] ~= nil
            atkstats.damage = 0 -- No damage this step
            return
        @attacked_map[target] = args.redamage_cooldown
        @n_hits += 1
        atkstats.type_multiplier *= (1.0 + (math.sqrt(@n_hits - 1) - 1.0) / 20)
        if args.on_hit_func
            args.on_hit_func(@, target, atkstats)
    return combined

for_all_visible_mons = (caster, f) ->
    for mon in *(Map.enemies_list caster)
        if not Map.object_visible(mon, mon.xy, caster)
            continue
        f(mon)
    return nil

message = (user, msg, color) ->
    for _ in screens()
        if user\is_local_player()
            EventLog.add(msg, color)
mon_title = (mon) -> if mon.unique then mon.name else "the #{mon.name}"
return {:spell_object_type, :mon_title, :message, :passing_projectile, :do_damage, :for_all_visible_mons}
