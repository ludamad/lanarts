World = require "core.World"
SourceMap = require "core.SourceMap"
Map = require "core.Map"
MapUtils = require "maps.MapUtils"
Display = require "core.Display"
GameState = require "core.GameState"

portal_set_label = (portal, label, is_down_arrow) ->
    color = with_alpha(COL_WHITE, 0.75)
    arrow_color = with_alpha(COL_WHITE, 0.75)
    portal.on_post_draw = () =>
        progress = math.abs(1 - (GameState.frame % 100)/50)
        if Map.object_visible(@)
            screen_xy = Display.to_screen_xy({@x, @y - 8 - 8 * progress})
            bitmap_draw_wrapped({:color, origin: Display.CENTER_BOTTOM}, screen_xy, 96, label)
            sprite = tosprite(if is_down_arrow then "spr_menu.down" else "spr_menu.up")
            sprite\draw({color: arrow_color, origin: Display.CENTER_BOTTOM}, vector_add(screen_xy, {0,19}))

MapLinker = newtype {
    -- TODO if we ever want two defined-region map links, we need to revisit this
    init: (@map_desc) =>
        @backwards, @forwards = {}, {}
        @map = false
        assert @label()
    get: () =>
        if not @map
            @map = @map_desc\generate(@backwards, @forwards)
        return @map
    label: () =>
        return @map_desc.map_label
    link_portal: (portal, from_sprite) =>
        -- for defined region -> general entrance
        bportal = {nil}
        portal.on_player_interact = (portal, user) ->
            Map.transfer(user, @get(), bportal[1].xy)
        portal_set_label(portal, @label(), true)
        append @backwards, (map, xy) ->
            bportal[1] = MapUtils.spawn_portal(map, xy, from_sprite)
            bportal[1].on_player_interact = (bportal_, user) ->
                Map.transfer(user, portal.map, portal.xy)
            portal_set_label(bportal[1], Map.map_label(portal.map), false)
    link_linker: (other_linker, to_sprite, from_sprite) =>
        -- for general exit -> general entrance
        fportal = {nil}
        bportal = {nil}
        append other_linker.backwards, (map, xy) ->
            fportal[1] = MapUtils.spawn_portal(map, xy, from_sprite)
            fportal[1].on_player_interact = (fportal_, user) ->
                Map.transfer(user, @get(), bportal[1].xy)
            portal_set_label(fportal[1], @label(), false)
        append @forwards, (map, xy) ->
            bportal[1] = MapUtils.spawn_portal(map, xy, to_sprite)
            bportal[1].on_player_interact = (bportal_, user) ->
                Map.transfer(user, other_linker\get(), fportal[1].xy)
            portal_set_label(bportal[1], other_linker\label(), true)
}

return nilprotect {:MapLinker}
