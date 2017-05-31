local Display = require "core.Display"

local M = {} -- Submodule

M.grass = Data.tileset_create {
    floor = { images = Display.images_load ("tiles/floor_overworld_grass2.png%32x32") },
    floor_alt1 = { images = Display.images_load ("tiles/floor_overworld_grass1.png%32x32") },
    floor_alt2 = { images = Display.images_load ("tiles/floor_overworld_grass3.png%32x32") },
    wall = { images = Display.images_load ("tiles/wall_overworld_trees.png%32x32")},
    dirt = { images = Display.images_load ("tiles/outside_floor/sandrockfloor/floor_sand_rock(0-3).png")},
--    dirt = { images = Display.images_load ("tiles/floor_overworld_dirt.png%32x32")},
--    wall_alt = { images = Display.images_load ("tiles/wall_overworld_wall.png%32x32") }
    wall_alt = { images = Display.images_load ("tiles/dngn_wall/snake/snake(0-9).png") }
}

M.pebble = Data.tileset_create {
	floor = { images = Display.images_load ("tiles/dngn_floor/pebblebrown/pebble_brown(0-8).png") },
	floor_alt = { images = Display.images_load ("tiles/dngn_floor/hive/hive.png%32x32") },
	wall = { images = Display.images_load ("tiles/dngn_wall/brickdark/brick_dark.png%32x32")}
}

M.crypt = Data.tileset_create {
	floor = { images = Display.images_load ("spr_tile_floors/crypt.png%32x32") },
	floor_alt = { images = Display.images_load ("spr_tile_floors/grey_dirt.png%32x32") },
	wall = { images = Display.images_load ("spr_tile_walls/catacombs.png%32x32") },
	wall_alt = { images = Display.images_load ("spr_tile_walls/vault_stone.png%32x32") },
}

M.hell = Data.tileset_create {
	floor = { images = Display.images_load ("spr_tile_floors/demonic_red.png%32x32") },
	floor_alt = { images = Display.images_load ("spr_tile_floors/floor_nerves.png%32x32") },
	floor_alt2 = { images = Display.images_load ("spr_tile_floors/crypt.png%32x32") },
	wall = { images = Display.images_load ("spr_tile_walls/volcanic_wall.png%32x32") },
	wall_alt = { images = Display.images_load ("spr_tile_walls/spider.png%32x32") },
}

M.temple = Data.tileset_create {
    floor = { images = Display.images_load ("tiles/dngn_floor/crystal/crystal_floor(0-5).png") },
    floor_alt = { images = Display.images_load ("tiles/dngn_floor/mesh/mesh(0-3).png") },
    wall = { images = Display.images_load ("tiles/dngn_wall/marble_wall/marble_wall(0-11).png")}
}

M.crystal = {
    floor = M.temple.floor,
    floor_alt = M.temple.floor_alt,
    wall = Data.tile_create { images = Display.images_load("tiles/dngn_wall/crystal/crystal_wall(0-13).png") }
}

M.hive = Data.tileset_create {
    floor = { images = Display.images_load "spr_tile_floors/acidic_floor.png%32x32"},
    floor_alt = { images = Display.images_load ("tiles/dngn_floor/snake/snake-a(0-3).png") },
    wall = { images = Display.images_load "spr_tile_walls/beehives.png%32x32" },
    wall_alt = { images = Display.images_load "spr_tile_walls/lair.png%32x32" }
}

M.pixulloch = Data.tileset_create {
    floor = { images = Display.images_load "spr_tile_floors/rect_gray.png%32x32" },
    floor_alt = { images = Display.images_load "spr_tile_floors/purple_nerves.png%32x32" },
    wall = { images = Display.images_load "spr_tile_walls/zot_blue.png%32x32" },
    wall_alt = { images = Display.images_load "spr_tile_walls/spider.png%32x32" }
}


M.orc = Data.tileset_create {
    floor = { images = Display.images_load "spr_tile_floors/orc.png%32x32" },
    floor_alt = { images = Display.images_load "spr_tile_floors/swamp.png%32x32" },
    wall = { images = Display.images_load "spr_tile_walls/orc.png%32x32" },
    wall_alt = { images = Display.images_load ("tiles/dngn_wall/snake/snake(0-9).png") }
}

M.lair = Data.tileset_create {
    floor = { images = Display.images_load "spr_tile_floors/lair.png%32x32" },
    floor_alt = { images = Display.images_load ("tiles/dngn_floor/snake/snake-a(0-3).png") },
    wall = { images = Display.images_load "spr_tile_walls/lair.png%32x32" },
    wall_alt = { images = Display.images_load ("tiles/dngn_wall/snake/snake(0-9).png") }
}

M.snake = Data.tileset_create {
    floor = { images = Display.images_load ("tiles/dngn_floor/snake/snake-d(0-3).png") },
    floor_alt = { images = Display.images_load ("tiles/dngn_floor/snake/snake-a(0-3).png") },
    floor_tunnel = { images = Display.images_load ("tiles/dngn_floor/snake/snake-c(0-3).png") },
    wall = { images = Display.images_load ("tiles/dngn_wall/snake/snake(0-9).png") }
}

for k,tileset in pairs(M) do
    if not rawget(tileset, "wall_alt") then
        tileset.wall_alt = tileset.wall
    end
    if not rawget(tileset, "floor_alt") then
        tileset.floor_alt = tileset.floor
    end
    if not rawget(tileset, "floor_tunnel") then
        tileset.floor_tunnel = tileset.floor_alt
    end
end

-- Make aliases
M.pebble.floor_corridor = M.pebble.floor_alt

return M
