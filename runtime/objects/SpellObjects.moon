GameObject = require "core.GameObject"
Map = require "core.Map"
Display = require "core.Display"

ObjectUtils = require "objects.ObjectUtils"
LuaGameObject = require "objects.LuaGameObject"
GlobalData = require "core.GlobalData"

M = nilprotect {} -- Submodule
M._spell_wall = Display.image_load "spr_spells/spell-wall.png"
M._spell_arrow = Display.image_load "spr_spells/arrow.png"

M.SpellWall = LuaGameObject.type_create()

SPELL_WALL_RADIUS = 15
SPELL_WALL_DEPTH = 100
SPELL_WALL_DURATION = 100

M.SpellWall.try_solidify = () =>
    tile_xy = ObjectUtils.tile_xy(@, true)
    if @solidified 
        -- Make sure units are never stuck
        collisions = Map.rectangle_collision_check(@map, {@x - 16, @y - 16, @x+16, @y+16}, @)
        collisions = table.filter(collisions, () => @solid)
        if #collisions == 0
            return
        @solidified = false
        Map.tile_set_solid(@map, tile_xy, false)
        return
    if Map.tile_is_solid(self.map, tile_xy)
        return
    collisions = Map.rectangle_collision_check(@map, {@x - 16, @y - 16, @x+16, @y+16}, @)
    collisions = table.filter(collisions, () => @solid)
    if #collisions ~= 0
        return
    Map.tile_set_solid(@map, tile_xy, true)
    @solidified = true

M.SpellWall.init = (args) => 
    {:points, :point_index} = args
    M.SpellWall.parent_init(@, points[point_index], SPELL_WALL_RADIUS, true, SPELL_WALL_DEPTH)
    @traits or= {}
    @solidified = false
    @sprite = M._spell_wall
    @n_steps = 0
M.SpellWall.on_map_init = () => 
    @try_solidify()
M.SpellWall.on_step = () =>
    @n_steps += 1
    if @n_steps >= SPELL_WALL_DURATION
        tile_xy = ObjectUtils.tile_xy(@, true)
        if @solidified
            Map.tile_set_solid(self.map, tile_xy, false)
        GameObject.destroy(@)
    else
        @try_solidify()
M.SpellWall.on_draw = () =>
    if Display.object_within_view(@) 
        alpha = (@n_steps / SPELL_WALL_DURATION) * 2
        if alpha > 1.0
            -- Wrap around the alpha:
            alpha = 1 - (alpha - 1) 
        alpha = math.max(0, math.min(alpha, 1))
        ObjectUtils.screen_draw(@sprite, @xy, alpha)

M.SpellSpikes = LuaGameObject.type_create()
M.SpellSpikes.init = (args) => 
    {:points, :point_index, :caster, :duration} = args
    M.SpellSpikes.parent_init(@, points[point_index], 16, true, SPELL_WALL_DEPTH)
    @traits or= {}
    @sprite = M._spell_arrow
    @n_steps = 0
    @caster = caster
    @duration = duration
    @n_ramp = 45
    @collided = {}
M.SpellSpikes.on_step = () =>
    @n_steps += 1
    if @n_steps >= @duration
        tile_xy = ObjectUtils.tile_xy(@, true)
        if @solidified
            Map.tile_set_solid(self.map, tile_xy, false)
        GameObject.destroy(@)
    else
        collisions = Map.rectangle_collision_check(@map, {@x - 8, @y - 8, @x+8, @y+8}, @)
        for col in *collisions
            if not col.is_combat_object --is_enemy
                continue
            if @collided[col.id] and @collided[col.id] + 45 > @n_steps
                continue
            @collided[col.id] = @n_steps
            eff = @caster\effective_stats()
            col\damage(eff.magic, 2 + eff.magic * 0.2, 0, 1)

M.SpellSpikes.on_draw = () =>
    if Display.object_within_view(@) 
        min = math.min(@n_ramp, if @n_steps > @duration / 2 then math.abs(@n_steps - @duration) else @n_steps)
        alpha = (min / @n_ramp)
        alpha = math.max(0, math.min(alpha, 1))
        ObjectUtils.screen_draw(@sprite, @xy, alpha)

return M
