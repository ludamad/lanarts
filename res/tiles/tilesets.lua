Tilesets = {}

Tilesets.grass = Data.tileset_create {
	floor = images_load "res/tiles/outside_floor/grass/grass(0-11).png",
	wall = {image_load "res/tiles/outside_wall/tree/tree.png"}
}

Tilesets.pebble = Data.tileset_create {
	floor = images_load "res/tiles/dngn_floor/pebblebrown/pebble_brown(0-8).png",
	floor_alt = images_load "res/tiles/dngn_floor/hive/hive.png%32x32",
	wall = images_load "res/tiles/dngn_wall/brickdark/brick_dark.png%32x32",
}
-- Make aliases
Tilesets.pebble.floor_corridor = Tilesets.pebble.floor_alt