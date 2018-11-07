World = require "core.World"
SourceMap = require "core.SourceMap"
Map = require "core.Map"
MapUtils = require "maps.MapUtils"

MapLinker = newtype {
    -- TODO if we ever want two defined-region map links, we need to revisit this
    init: (@map_desc) =>
        @backwards, @forwards = {}, {}
        @map = false
    get: () =>
        if not @map
            @map = @map_desc\generate(@backwards, @forwards)
        return @map
    link_portal: (portal, from_sprite) =>
        -- for defined region -> general entrance
        bportal = {nil}
        portal.on_player_interact = (portal, user) ->
            Map.transfer(user, @get(), bportal[1].xy)
        append @backwards, (map, xy) ->
            bportal[1] = MapUtils.spawn_portal(map, xy, from_sprite)
            bportal[1].on_player_interact = (bportal_, user) ->
                Map.transfer(user, portal.map, portal.xy)
    link_linker: (other_linker, to_sprite, from_sprite) =>
        -- for general exit -> general entrance
        fportal = {nil}
        bportal = {nil}
        append other_linker.backwards, (map, xy) ->
            fportal[1] = MapUtils.spawn_portal(map, xy, to_sprite)
            fportal[1].on_player_interact = (fportal_, user) ->
                Map.transfer(user, @get(), bportal[1].xy)
        append @forwards, (map, xy) ->
            bportal[1] = MapUtils.spawn_portal(map, xy, from_sprite)
            bportal[1].on_player_interact = (bportal_, user) ->
                Map.transfer(user, other_linker\get(), fportal[1].xy)
}

-- forward_link = map_linker map, (back_links) ->
--     return require("map_descs.HiveDepths")\generate for back_link in *back_links
--         (map, xy) -> back_link(MapUtils.spawn_portal(map, xy, "spr_gates.exit_dungeon"))
--
-- place_dungeon = (map, xy) ->
--     forward_link(MapUtils.spawn_portal(map, xy, "spr_gates.hive_portal"))
-- vault = SourceMap.area_template_create(Vaults.ridge_dungeon {dungeon_placer: place_dungeon, tileset: Tilesets.hive})
-- if not place_feature(map, vault, regions)
--     return nil

return {:MapLinker}
