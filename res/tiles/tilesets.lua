require "utils"

TileSets = {}

TileSets.grass = Data.tileset_create {
	floor = { images = images_load "res/tiles/outside_floor/grass/grass(0-11).png"},
	wall = { images = {image_load "res/tiles/outside_wall/tree/tree.png"}}
}

TileSets.pebble = Data.tileset_create {
	floor = { images = images_load "res/tiles/dngn_floor/pebblebrown/pebble_brown(0-8).png" },
	floor_alt = { images = images_load "res/tiles/dngn_floor/hive/hive.png%32x32" },
	wall = { images = images_load "res/tiles/dngn_wall/brickdark/brick_dark.png%32x32"},
}
-- Make aliases
TileSets.pebble.floor_corridor = TileSets.pebble.floor_alt

pretty_print(TileSets)
