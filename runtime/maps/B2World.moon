-- Helpers for using Box2D to make levels with

b2 = require 'b2'
GenerateUtils = require "maps.GenerateUtils"
Keys = require "core.Keyboard"
DebugUtils = require "maps.DebugUtils"
B2Utils = require "maps.B2Utils"

-- Used for shape simulations and transformation utilities
-- Useful class for using simulations to create meaningful shapes
B2World = newtype {
    init: () =>
        @world = b2.World(b2.Vec2(0.0,0.0)) -- No gravity
        @dynamic_bodies = {}
        @static_bodies = {}
        @all_bodies = {}
    add_body: (shape, density = 1.0) =>
        -- Density 0.0 implies a static box2d object
        body = B2Utils.create_body @world, shape, density
        if density == 0.0
            append @static_bodies, body
        else
            append @dynamic_bodies, body
        append @all_bodies, body
        return body
    unique_within: (o1, o2, p1, p2) =>
        for o in *@all_bodies
            if o == o1 or o == o2
                continue
            if B2Utils.ray_cast(o.fixtures, p1, p2)
                return false
        return true
    remove_body: (body) =>
        -- TODO clear lists above?
        @world\DestroyBody(body)
    has_overlaps: () =>
        for {:body} in *@all_bodies
            if body\OverlapsOtherBody()
                return true
        return false
    visualize: (title) =>
        if not DebugUtils.is_debug_visualization()
            return
        @world\SetDebugDraw with b2.GLDrawer()
            \SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
        w,h = DebugUtils.visualization_size()
        require("core.Display").set_world_region {-w/2, -h/2, w/2, h/2}
        require("core.GameState").game_loop () ->
            font = font_cached_load "fonts/Gudea-Regular.ttf", 14
            @world\DrawDebugData()
            if Keys.key_pressed "N"
                return true
            -- Hack for dealing with box2d clobbering:
            require("core.Display").reset_blend_func()
            font\draw({color: COL_WHITE, origin: require("core.Display").CENTER}, {100, 100}, title)
            return false
}

return nilprotect {:B2World}
