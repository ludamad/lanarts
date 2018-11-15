OldMaps = require "maps.OldMaps"
Tilesets = require "tiles.Tilesets"

return OldMaps.create_map_desc table.merge OldMaps.Dungeon2[2], {
    tileset: Tilesets.temple
    label: "Temple Chamber"
}
