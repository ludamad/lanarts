EventLog = require "ui.EventLog"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
Display = require "core.Display"
DataW = require "DataWrapped"
ObjectUtils = require "objects.ObjectUtils"
SpellUtils = require "spells.SpellUtils"

M = nilprotect {
    _fire: tosprite "spr_effects.fire-anim"
}

-- Utility class that does most of RingOfFire, leaving enough for its commonality
-- to implemented the SpawnedFire's that result from killing enemies with RingOfFire
RingFireBase = (extension) -> SpellUtils.spell_object_type table.merge {
    types: {"Red"}
    base_init: (args) =>
        assert @_damage
        -- Damage cooldowns by individual flame
        @cooldown = args.cooldown or 5
    _on_kill: (obj) => nil -- Default do nothing
    base_on_step: () =>
        if @every 20
            play_sound "sound/ringfire-loop.ogg"
        @for_all_rings (x, y, damage_cooldown) ->
            for obj in *Map.radius_collision_check(@map, 15, {x, y})
                if damage_cooldown[obj] or (obj.team == nil) or obj.team == @caster.team
                    continue
                if @do_damage(obj, @_damage)
                    @_on_kill(obj)

                --    @message "You gain mana from killing #{mon_name}!", COL_PALE_BLUE
                --    @caster\heal_mp(5)
                play_sound "sound/ringfire-hit.ogg"
                damage_cooldown[obj] = @cooldown
            for k,v in pairs damage_cooldown
                v -= 1
                damage_cooldown[k] = (if v > 0 then v else nil)
    on_step: () => @base_on_step()
    on_draw: () =>
        if @caster.destroyed 
            return
        alpha = 1.0
        if @in_time_slice(0.0, 0.1)
            alpha = @get_progress(0.0, 0.1)
        elseif @in_time_slice(0.9, 1)
            alpha = 1.0 - @get_progress(0.9, 1)
        frame = @time_passed()
        @for_all_rings (x, y) ->
            ObjectUtils.screen_draw(M._fire, {x, y}, alpha, frame)
}, extension

-- When something dies from RingOfFire, this is spawned
SpawnedFire = RingFireBase {
    -- RingFireBase config
    init: (args) =>
        @_damage = 2 / 8
        @damage_cooldowns = {} -- takes [dx][dy][obj]
        @base_init(args)
    for_all_rings: (f) =>
        rad = 1
        for dy=-rad,rad
            for dx=-rad,rad
                if math.abs(dx) + math.abs(dy) == 2
                    continue
                {cx, cy} = @xy
                x, y = cx + dx * Map.TILE_SIZE, cy + dy * Map.TILE_SIZE
                if not @caster\within_field_of_view({x, y})
                    continue
                @damage_cooldowns[dy] or= {}
                @damage_cooldowns[dy][dx] or= {}
                f(x, y, @damage_cooldowns[dy][dx])
}

RingOfFire = RingFireBase {
    -- RingFireBase config
    init: (args) =>
        @_damage = 2 / 8
        @damage_cooldowns = {} -- takes [index][obj]
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
        mon_name = SpellUtils.mon_title(obj)
        @message("Fire springs forth from the defeated #{mon_name}!", COL_PALE_BLUE)
        GameObject.add_to_level SpawnedFire.create {
            caster: @caster
            xy: obj.xy -- Create around the damaged enemy
            duration: 40
        }
    -- GameObject methods
    -- Extra functionality over RingFireBase: Manage caster's vision radius
    on_map_deinit: () =>
        if @caster ~= nil and not @caster.destroyed
            @caster.__vision_radius_override = nil -- Stop overriding vision radius
    on_step: () =>
        @caster.__vision_radius_override = 9
        @base_on_step()

    for_all_rings: (f) =>
        radius_percentage = nil
        for i=1,10
            if @percent_passed() <= i / 10
                radius_percentage = @get_progress((i-1)/10,i/10)
                break
        rings = @_create_rings(50 - 50 * radius_percentage)
        for idx=1,#rings
            {x, y} = rings[idx]
            @damage_cooldowns[idx] or= {}
            f(x + @caster.x, y + @caster.y, @damage_cooldowns[idx])
}

DataW.spell_create {
    name: "Ring of Fire",
    description: ""
    types: {"Red"}
    spr_spell: "spr_spells.spell_icon_ring_of_fire"
    prereq_func: (caster) -> return true
    autotarget_func: (caster) -> caster.x, caster.y
    action_func: (caster, x, y) ->
        GameObject.add_to_level RingOfFire.create({:caster, duration: 400})
    mp_cost: 50
    spell_cooldown: 1200
    cooldown: 0
}

return M
