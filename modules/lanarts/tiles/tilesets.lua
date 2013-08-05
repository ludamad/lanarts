local utils = import "core.utils"

local M = {} -- Submodule

M.grass = Data.tileset_create {
	floor = { images = images_load (path_resolve "outside_floor/grass/grass(0-11).png") },
	wall = { images = { image_load (path_resolve "outside_wall/tree/tree.png") }}
}

M.pebble = Data.tileset_create {
	floor = { images = images_load (path_resolve "dngn_floor/pebblebrown/pebble_brown(0-8).png") },
	floor_alt = { images = images_load (path_resolve "dngn_floor/hive/hive.png%32x32") },
	wall = { images = images_load (path_resolve "dngn_wall/brickdark/brick_dark.png%32x32")}
}

M.temple = Data.tileset_create {
    floor = { images = images_load (path_resolve "dngn_floor/crystal/crystal_floor(0-5).png") },
    floor_alt = { images = images_load (path_resolve "dngn_floor/mesh/mesh(0-3).png") },
    wall = { images = images_load (path_resolve "dngn_wall/marble_wall/marble_wall(0-11).png")}
}

-- Make aliases
M.pebble.floor_corridor = M.pebble.floor_alt

return M