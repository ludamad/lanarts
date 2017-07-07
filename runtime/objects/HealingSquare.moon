GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
EventLog = require "ui.EventLog"
Display = require "core.Display"

ObjectUtils = require "objects.ObjectUtils"
LuaGameObject = require "objects.LuaGameObject"
GlobalData = require "core.GlobalData"

DungeonFeatures = require "objects.DungeonFeatures"

M = nilprotect {} -- Submodule
M.healingsqr_sprite = tosprite "spr_doors.healingsqr"

-- Door
M.HealingSquare = LuaGameObject.type_create({base: DungeonFeatures.FeatureBase})
with M.HealingSquare
    .parent_on_step = () =>
    .on_step = () =>
        collisions = Map.rectangle_collision_check(@map, @area, @)
        @stepped_on = false
        for col in *collisions
            -- Must not be nil, this object must be specifying it's a player, essentially.
            -- TODO have a real, documented API
            if col.is_enemy == false
                {:mp, :max_mp, :hp, :max_hp, :hpregen, :mpregen} = col\effective_stats()
                if hp >= max_hp and mp >= max_mp
                    continue
                @stepped_on = true
                if col.is_resting
                    col\heal_hp(hpregen * 8)
                    col\heal_mp(mpregen * 8)
                    for _ in screens()
                        EventLog.add("You rest twice as quickly!", COL_PALE_BLUE)
                else
                    col\heal_hp(hpregen)
                    col\heal_mp(mpregen)
                    for _ in screens()
                        EventLog.add("You regain health and mana twice as quickly!", COL_PALE_BLUE)
        if not @seen and Map.object_visible(@)
            @seen = true
    .bbox = () => 
        {x, y} = Display.screen_coords @xy
        return {x-16,y-16,x+16,y+16}
    .on_draw = () =>
        if Display.object_within_view(@)
            if @seen or Map.object_visible(@)
                Display.draw_rectangle(with_alpha(COL_PALE_BLUE, (if @stepped_on then 0.4 else 0.05)), @bbox())
                ObjectUtils.screen_draw(@sprite, @xy, (if @stepped_on then 1 else 0.5), 0.0)
    .on_map_init = () =>
        tile_xy = ObjectUtils.tile_xy(@, true)
        Map.tile_set_seethrough(@map, tile_xy, true)
        @area = {
            @x - 16, @y - 16, @x + 16, @y + 16
        }
    .init = (args) =>
        M.HealingSquare.parent_init(@, args)
        @depth = DungeonFeatures.FEATURE_DEPTH
        @stepped_on = false
        @seen = false
        @sprite = M.healingsqr_sprite
return M
