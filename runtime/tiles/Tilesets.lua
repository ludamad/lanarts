local Display = require "core.Display"

local M = {} -- Submodule

M.grass = Data.tileset_create {
    floor = { images = Display.images_load (path_resolve "floor_overworld_grass1.png%32x32") },
    floor_alt1 = { images = Display.images_load (path_resolve "floor_overworld_grass2.png%32x32") },
    floor_alt2 = { images = Display.images_load (path_resolve "floor_overworld_grass3.png%32x32") },
    wall = { images = Display.images_load (path_resolve "wall_overworld_trees.png%32x32")},
    dirt = { images = Display.images_load (path_resolve "outside_floor/sandrockfloor/floor_sand_rock(0-3).png")},
--    dirt = { images = Display.images_load (path_resolve "floor_overworld_dirt.png%32x32")},
--    wall_alt = { images = Display.images_load (path_resolve "wall_overworld_wall.png%32x32") }
    wall_alt = { images = Display.images_load (path_resolve "dngn_wall/snake/snake(0-9).png") }
}
if math.random() > 0.6 then
    M.grass.floor, M.grass.floor_alt1 = M.grass.floor_alt1, M.grass.floor
end

M.pebble = Data.tileset_create {
	floor = { images = Display.images_load (path_resolve "dngn_floor/pebblebrown/pebble_brown(0-8).png") },
	floor_alt = { images = Display.images_load (path_resolve "dngn_floor/hive/hive.png%32x32") },
	wall = { images = Display.images_load (path_resolve "dngn_wall/brickdark/brick_dark.png%32x32")}
}

M.temple = Data.tileset_create {
    floor = { images = Display.images_load (path_resolve "dngn_floor/crystal/crystal_floor(0-5).png") },
    floor_alt = { images = Display.images_load (path_resolve "dngn_floor/mesh/mesh(0-3).png") },
    wall = { images = Display.images_load (path_resolve "dngn_wall/marble_wall/marble_wall(0-11).png")}
}

M.crystal = {
    floor = M.temple.floor,
    floor_alt = M.temple.floor_alt,
    wall = Data.tile_create { images = Display.images_load(path_resolve "dngn_wall/crystal/crystal_wall(0-13).png") }
}

M.snake = Data.tileset_create {
    floor = { images = Display.images_load (path_resolve "dngn_floor/snake/snake-d(0-3).png") },
    floor_alt = { images = Display.images_load (path_resolve "dngn_floor/snake/snake-a(0-3).png") },
    floor_tunnel = { images = Display.images_load (path_resolve "dngn_floor/snake/snake-c(0-3).png") },
    wall = { images = Display.images_load (path_resolve "dngn_wall/snake/snake(0-9).png") }
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
