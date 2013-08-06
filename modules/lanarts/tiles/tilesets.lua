local utils = import "core.utils"

local M = {} -- Submodule

M.grass = Data.tileset_create {
	floor = { images = images_load (path_resolve "outside_floor/grass/grass(0-11).png") },
    wall = { images = images_load (path_resolve "outside_wall/tree/tree.png%32x32")},
    dirt = { images = images_load (path_resolve "outside_floor/sandrockfloor/floor_sand_rock(0-3).png")},
--    dirt = { images = images_load (path_resolve "floor_overworld_dirt.png%32x32")},
--    wall_alt = { images = images_load (path_resolve "wall_overworld_wall.png%32x32") }
    wall_alt = { images = images_load (path_resolve "dngn_wall/snake/snake(0-9).png") }
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

M.crystal = {
    floor = M.temple.floor,
    floor_alt = M.temple.floor_alt,
    wall = Data.tile_create { images = images_load(path_resolve "dngn_wall/crystal/crystal_wall(0-13).png") }
}

M.snake = Data.tileset_create {
    floor = { images = images_load (path_resolve "dngn_floor/snake/snake-d(0-3).png") },
    floor_alt = { images = images_load (path_resolve "dngn_floor/snake/snake-a(0-3).png") },
    floor_tunnel = { images = images_load (path_resolve "dngn_floor/snake/snake-c(0-3).png") },
    wall = { images = images_load (path_resolve "dngn_wall/snake/snake(0-9).png") }
}

for k,tileset in pairs(M) do
    if not rawget(tileset, "floor_alt") then
        tileset.floor_alt = tileset.floor
    end
    if not rawget(tileset, "floor_tunnel") then
        tileset.floor_tunnel = tileset.floor_alt
    end
end

-----
--
--  - name: {images = images_load "modules/lanarts/tiles/dngn_wall/stone_dark/stone_dark(0-3).png"}
--    files: modules/lanarts/tiles/dngn_wall/stone_dark/stone_dark(0-3).png
--
--  - name: {images = images_load "modules/lanarts/tiles/ngn_wall/brickdark/brick_dark.png%32x32"}
--    files: modules/lanarts/tiles/dngn_wall/{images = images_load "modules/lanarts/tiles/ngn_wall/brickdark/brick_dark.png%32x32"}dark/{images = images_load "modules/lanarts/tiles/ngn_wall/brickdark/brick_dark.png%32x32"}_dark.png%32x32
--
--  - name: crypt_wall
--    files: modules/lanarts/tiles/dngn_wall/crypt/crypt(0-14).png
--    
--  - name: hive_wall
--    files: modules/lanarts/tiles/dngn_wall/hive/hive(0-7).png
--    
--  - name: crystal_wall
--    files: modules/lanarts/tiles/dngn_wall/crystal/crystal_wall(0-13).png
--    
--  - name: grass_tree
--    files: [modules/lanarts/tiles/outside_wall/tree/tree.png]
--    
--  - name: marble_wall
--    files: modules/lanarts/tiles/dngn_wall/marble_wall/marble_wall(0-11).png
--    
--  - name: snake_wall
--    files: modules/lanarts/tiles/dngn_wall/snake/snake(0-9).png
--    
--#Features
--  - name: stairs_up
--    files: [modules/lanarts/tiles/features/stairs_up.png]
--    
--  - name: stairs_down
--    files: [modules/lanarts/tiles/features/stairs_down.png]
--#Floors
--
--  - name: crypt_floor
--    files: modules/lanarts/tiles/dngn_floor/crypt2/crypt(0-7).png
--
--  - name: cobalt_floor
--    files: modules/lanarts/tiles/dngn_floor/cobalt/black_cobalt(0-11).png
--    
--  - name: cobble_blood
--    files: modules/lanarts/tiles/dngn_floor/cobbleblood/cobble_blood(0-11).png
--    
--  - name: crystal_floor
--    files: modules/lanarts/tiles/dngn_floor/crystal/crystal_floor(0-5).png
--    
--  - name: demonic_red
--    files: modules/lanarts/tiles/dngn_floor/demonic_red/demonic_red(0-8).png
--    
--  - name: hive
--    files: modules/lanarts/tiles/dngn_floor/hive/hive.png%32x32
--    
--  - name: mesh
--    files: modules/lanarts/tiles/dngn_floor/mesh/mesh(0-3).png
--    
--  - name: pebble_brown
--    files: modules/lanarts/tiles/dngn_floor/pebblebrown/pebble_brown(0-8).png 
--    
--  - name: nerve_floor
--    files: modules/lanarts/tiles/dngn_floor/nerves/floor_nerves(0-6).png
--    
--  - name: lair_floor
--    files: modules/lanarts/tiles/dngn_floor/lair/lair(0-7).png
--    
--  - name: spider_floor
--    files: modules/lanarts/tiles/dngn_floor/spider/spider0(0-9).png
--    
--  - name: snake_floor1
--    files: modules/lanarts/tiles/dngn_floor/snake/snake-a(0-3).png
--    
--  - name: snake_floor2
--    files: modules/lanarts/tiles/dngn_floor/snake/snake-c(0-3).png
--    
--  - name: snake_floor3
--    files: modules/lanarts/tiles/dngn_floor/snake/snake-d(0-3).png
--    
--#Outside floor
--  - name: sand_rock
--    files: modules/lanarts/tiles/outside_floor/sandrockfloor/floor_sand_rock(0-3).png
--    
--
--M.sand_tiles = Data.tile_set_create {
--    floor = { images = images_load "modules/lanarts/tiles/outside_floor/sandrockfloor/floor_sand_rock(0-3).png"},
--    wall = { images = images_load "modules/lanarts/tiles/dngn_floor/hive/hive.png%32x32"}
--}
--    floor_tile: sand_rock
--    wall_tile: hive_wall
--
--  - tileset: crypt_tiles
--    floor_tile: crypt_floor
--    wall_tile: crypt_wall
--
--  - tileset: lair_tiles1
--    floor_tile: lair_floor
--    alt_floor_tile: mesh
--    wall_tile: marble_wall
--
--  - tileset: lair_tiles2
--    floor_tile: lair_floor
--    alt_floor_tile: cobalt_floor
--    wall_tile: hive_wall
--    
--  - tileset: lair_tiles3
--    floor_tile: spider_floor
--    alt_floor_tile: pebble_brown
--    wall_tile: hive_wall
--
--  - tileset: snake_tiles
--    floor_tile: snake_floor3
--    alt_floor_tile: snake_floor1
--    corridor_tile: snake_floor2
--    wall_tile: snake_wall
--
--  - tileset: cobalt_tiles
--    floor_tile: cobalt_floor
--    alt_floor_tile: mesh
--    corridor_tile: mesh
--    wall_tile: marble_wall
--
--  - tileset: pebble_tiles
--    floor_tile: pebble_brown
--    wall_tile: {images = images_load "modules/lanarts/tiles/ngn_wall/brickdark/brick_dark.png%32x32"}
--    corridor_tile: hive
--    alt_wall_tile: {images = images_load "modules/lanarts/tiles/ngn_wall/brickdark/brick_dark.png%32x32"}
--    alt_floor_tile: hive
--    alt_corridor_tile: hive
--
--  - tileset: crystal_tiles
--    floor_tile: crystal_floor
--    wall_tile: crystal_wall
--    corridor_tile: mesh
--    alt_floor_tile: crystal_floor
--    alt_wall_tile: marble_wall
--    alt_corridor_tile: crystal_floor
--
--  - tileset: crystal_tiles_with_marble
--    floor_tile: crystal_floor
--    wall_tile: marble_wall
--    corridor_tile: mesh
--    alt_wall_tile: crystal_wall
--    
--  - tileset: cobble
--    floor_tile: cobble_blood
--    wall_tile: stone_dark

-- Make aliases
M.pebble.floor_corridor = M.pebble.floor_alt

return M