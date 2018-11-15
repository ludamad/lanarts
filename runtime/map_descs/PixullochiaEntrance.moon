OldMaps = require "maps.OldMaps"
Tilesets = require "tiles.Tilesets"
return OldMaps.create_map_desc table.merge OldMaps.Dungeon4[1], {
    tileset: Tilesets.pixulloch
    label: "Pixullochia Entrance"
}
