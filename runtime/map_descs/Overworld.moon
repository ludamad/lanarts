----
-- Generates the game maps, starting with high-level details (places that will be in the game)
-- and then generating actual tiles.
----

{:Tile, :spread_region_delta_func} = require "maps.GenerateUtils"

NewMaps = require "maps.NewMaps"
Tilesets = require "tiles.Tilesets"
MapUtils = require "maps.MapUtils"
ItemUtils = require "maps.ItemUtils"
ItemGroups = require "maps.ItemGroups"

Vaults = require "maps.Vaults"
{:place_vault_in} = require "maps.VaultUtils"
SourceMap = require "core.SourceMap"
OldMaps = require "maps.OldMaps"
Region1 = require "maps.Region1"

{:MapDesc, :MapNode} = require "maps.MapDesc"

-- Generation constants and data
{   :FLAG_INNER_PERIMETER, :FLAG_DOOR_CANDIDATE,
    :FLAG_OVERWORLD, :FLAG_NO_ENEMY_SPAWN
} = Vaults

create_overworld_scheme = (tileset) -> nilprotect {
    floor1: Tile.create(tileset.floor, false, true, {FLAG_OVERWORLD})
    floor2: Tile.create(tileset.floor_alt, false, true, {FLAG_OVERWORLD})
    wall1: Tile.create(tileset.wall, true, true, {FLAG_OVERWORLD})
    wall2: Tile.create(tileset.wall_alt, true, false)
}

OVERWORLD_TILESET = create_overworld_scheme(Tilesets.grass)

OVERWORLD_AMBIENT_ENEMIES = table.tconcat OldMaps.medium_animals, {
    {enemy: "Sheep", chance: 45, group_chance: 35, group_size: 2}
    {enemy: "Black Mamba", chance: 20}
    {enemy: "Skeleton", chance: 18}
    {enemy: "Cloud Elemental", chance: 12, group_chance: 25, group_size: 2}
    {enemy: "Mouther", chance: 10}
}

OVERWORLD_VAULT_ENEMIES = table.tconcat OldMaps.medium_enemies, {
    {enemy: "Black Mamba", chance: 35}
    {enemy: "Mouther", chance: 25}
    {enemy: "Fire Bat", chance: 12}
    {enemy: "Centaur Hunter", chance: 6}
}

OVERWORLD_PROFILES = {
    {
        name: "open_meadow"
        region_base: 5
        region_rolls: {2, 2}
        connect_base: 3
        connect_rolls: {4, 4, 3}
        room_base: 8
        room_rolls: {4, 4}
        size_base: 84
        shell: 25
        ridges: {3, 5}
        small_vaults: {1, 2}
        mini_features: {Vaults.cavern, Vaults.stone_henge, Vaults.stone_henge}
        ambient_enemies: 7
        n_statues: 3
    }
    {
        name: "broken_grove"
        region_base: 7
        region_rolls: {3, 3}
        connect_base: 2
        connect_rolls: {3, 3, 2}
        room_base: 6
        room_rolls: {4, 3, 2}
        size_base: 88
        shell: 22
        ridges: {4, 6}
        small_vaults: {2, 4}
        mini_features: {Vaults.cavern, Vaults.cavern, Vaults.stone_henge, Vaults.stone_henge}
        ambient_enemies: 8
        n_statues: 4
    }
    {
        name: "old_stones"
        region_base: 8
        region_rolls: {3, 3, 2}
        connect_base: 1
        connect_rolls: {3, 2, 2}
        room_base: 5
        room_rolls: {3, 3, 2}
        size_base: 92
        shell: 20
        ridges: {2, 4}
        small_vaults: {3, 5}
        mini_features: {Vaults.stone_henge, Vaults.stone_henge, Vaults.stone_henge, Vaults.cavern}
        ambient_enemies: 9
        n_statues: 6
    }
}

profile_by_name = (name) ->
    return nil unless name and name != ""
    for profile in *OVERWORLD_PROFILES
        if profile.name == name
            return profile
    return nil

roll_sum = (rng, base, rolls) ->
    value = base
    for roll in *(rolls or {})
        value += rng\random(0, roll)
    return value

random_range = (rng, range) ->
    return rng\random(range[1], range[2])

place_doors_and_statues = (region_set, profile) ->
    {:map, :regions} = region_set

    for region in *regions
        area = region\bbox()
        conf = rawget(region, "conf") or profile or {}
        for xy in *SourceMap.rectangle_match {:map, selector: {matches_none: {SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}, matches_all: {FLAG_DOOR_CANDIDATE}}}
            MapUtils.spawn_door(map, xy)
        for i=1,(conf.n_statues or 0)
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, Vaults.FLAG_HAS_VAULT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_SOLID, SourceMap.FLAG_HAS_OBJECT}, remove: SourceMap.FLAG_SEETHROUGH})
            MapUtils.spawn_decoration(map, OldMaps.statue, sqr, random(0,17))

overworld_items_and_enemies = (region_set, profile) ->
    {:map, :regions} = region_set

    for region in *regions
        area = region\bbox()
        for i=1,OldMaps.adjusted_item_amount(10) do
            sqr = MapUtils.random_square(map, area, {matches_none: {FLAG_INNER_PERIMETER, SourceMap.FLAG_HAS_OBJECT, SourceMap.FLAG_SOLID}})
            if not sqr
                break
            map\square_apply(sqr, {add: {SourceMap.FLAG_HAS_OBJECT}})
            item = ItemUtils.item_generate ItemGroups.basic_items
            MapUtils.spawn_item(map, item.type, item.amount, sqr)

        OldMaps.generate_from_enemy_entries(map, OVERWORLD_AMBIENT_ENEMIES, profile.ambient_enemies, area, {matches_none: {SourceMap.FLAG_SOLID, Vaults.FLAG_HAS_VAULT, FLAG_NO_ENEMY_SPAWN}})
    return true

-----------------------------
overworld_features = (region_set, profile) ->
    -------------------------
    -- Place ridges: --
    place_outdoor_ridges = () ->
        door_placer = (map, xy) ->
            -- nil is passed for the default open sprite
            MapUtils.spawn_door(map, xy)
        item_placer = (map, xy) ->
            item = ItemUtils.item_generate ItemGroups.basic_items
            MapUtils.spawn_item(map, item.type, item.amount, xy)
        if not place_vault_in region_set, Vaults.ridge_dungeon {
            dungeon_placer: item_placer
            :door_placer
            tileset: Tilesets.pebble
        }
            return true -- Dont reject
        return true

    for i=1,random_range(region_set.map.rng, profile.ridges)
        if not place_outdoor_ridges()
            print "RETRY: place_outdoor_ridges()"
            return false
    -------------------------

    -------------------------
    -- Place small_random_vaults:  --
    place_small_vaults = () ->
        for i=1,random_range(region_set.map.rng, profile.small_vaults)
            enemy_placer = (map, xy) ->
                enemy = OldMaps.enemy_generate(OVERWORLD_VAULT_ENEMIES)
                MapUtils.spawn_enemy(map, enemy, xy)
            door_placer = (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy, nil, xy)
            store_placer = (map, xy) ->
                Region1.generate_store(map, xy)
            item_placer = (map, xy) ->
                if map.rng\chance(.1)
                    MapUtils.spawn_item(map, "Scroll of Experience", 1, xy)
                else
                    item = ItemUtils.item_generate ItemGroups.basic_items
                    MapUtils.spawn_item(map, item.type, item.amount, xy)
            gold_placer = (map, xy) ->
                MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            tileset = Tilesets.snake
            if not place_vault_in region_set, Vaults.small_random_vault {
                rng: region_set.map.rng
                :item_placer
                :enemy_placer
                :gold_placer
                :store_placer
                :tileset
                :door_placer
            }
                continue
        return true
    if not place_small_vaults()
        print "RETRY: place_small_vaults()"
        return nil
    -------------------------

    ---------------------------------
    -- Place big vaults            --
    -- place_big_vaults = () ->
    --     for template in *{Vaults.big_encounter1}--, Vaults.big_encounter2}
    --         enemy_placer = (map, xy) ->
    --             enemy = OldMaps.enemy_generate(OldMaps.medium_enemies)
    --             MapUtils.spawn_enemy(map, enemy, xy)
    --         item_placer = (map, xy) ->
    --             item = ItemUtils.item_generate ItemGroups.basic_items
    --             MapUtils.spawn_item(map, item.type, item.amount, xy)
    --         gold_placer = (map, xy) ->
    --             if map.rng\chance(.7)
    --                 MapUtils.spawn_item(map, "Gold", random(2,10), xy)
    --         door_placer = (map, xy) ->
    --             -- nil is passed for the default open sprite
    --             MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, 'Azurite Key')
    --         vault = SourceMap.area_template_create(template {:enemy_placer, :item_placer, :gold_placer, :door_placer})
    --         if not place_feature(map, vault)
    --             return true
    -- if place_big_vaults() then return nil
    ---------------------------------

    -- ---------------------------------
    -- -- Place centaur challenge     --
    -- place_centaur_challenge = () ->
    --     enemy_placer = (map, xy) ->
    --         enemy = OldMaps.enemy_generate(OldMaps.medium_enemies)
    --         MapUtils.spawn_enemy(map, enemy, xy)
    --     boss_placer = (map, xy) ->
    --         --if map.rng\randomf() < .5
    --         --    enemy = OldMaps.enemy_generate(OldMaps.strong_hell)
    --         --    MapUtils.spawn_enemy(map, enemy, xy)
    --         --else
    --         MapUtils.spawn_enemy(map, "Centaur Hunter", xy)
    --     n_items_placed = 0
    --     item_placer = (map, xy) ->
    --         item = ItemUtils.item_generate ItemGroups.basic_items, false, 1, (if n_items_placed == 0 then 100 else 2)
    --         MapUtils.spawn_item(map, item.type, item.amount, xy)
    --         n_items_placed += 1
    --     gold_placer = (map, xy) ->
    --         if map.rng\chance(.7)
    --             MapUtils.spawn_item(map, "Gold", random(2,10), xy)
    --     door_placer = (map, xy) ->
    --         -- nil is passed for the default open sprite
    --         MapUtils.spawn_door(map, xy, nil, Vaults._door_key1, "Azurite Key")
    --     vault = SourceMap.area_template_create(Vaults.anvil_encounter {:enemy_placer, :boss_placer, :item_placer, :gold_placer, :door_placer})
    --     if not place_feature(map, vault, regions)
    --         return true
    -- -- if place_centaur_challenge() then return nil
    -- ---------------------------------

    ---------------------------------
    place_mini_features = () ->
        for i, template in ipairs profile.mini_features
        --for template in *{Vaults.stone_henge, Vaults.stone_henge, Vaults.stone_henge, Vaults.stone_henge, Vaults.stone_henge}
            enemy_placer = (map, xy) ->
                enemy = OldMaps.enemy_generate(OVERWORLD_VAULT_ENEMIES)
                MapUtils.spawn_enemy(map, enemy, xy)
            local store_placer
            if i ~= 1 and i ~= 4
                store_placer = do_nothing
            -- TODO evaluate if I want the 4 stores:
            elseif i == 4
                store_placer = (map, xy) ->
                    MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            else
                store_placer = (map, xy) ->
                    Region1.generate_store(map, xy)

            item_placer = (map, xy) ->
                item = ItemUtils.item_generate ItemGroups.basic_items
                MapUtils.spawn_item(map, item.type, item.amount, xy)
            gold_placer = (map, xy) ->
                MapUtils.spawn_item(map, "Gold", random(2,10), xy)
            door_placer = (map, xy) ->
                -- nil is passed for the default open sprite
                MapUtils.spawn_door(map, xy)
            if not place_vault_in region_set, template {
                :enemy_placer
                :store_placer
                :item_placer
                :gold_placer
                :door_placer
            }
                continue
        return true

    if not place_mini_features()
        print "RETRY: place_mini_features()"
        return nil

    return true

return MapDesc.create {
    map_label: "Plain Valley"
    size: {120, 120}
    default_content: Tilesets.grass.wall
    default_flags: {SourceMap.FLAG_SOLID, SourceMap.FLAG_SEETHROUGH}
    arc_chance: 0.05
    children: {
        MapNode.create {
            place: () =>
                requested_profile = @desc.generate_hooks and @desc.generate_hooks.overworld_profile
                profile = profile_by_name(requested_profile) or @rng\random_choice(OVERWORLD_PROFILES)
                number_regions = roll_sum(@rng, profile.region_base, profile.region_rolls)
                connect_line_width = () -> roll_sum(@rng, profile.connect_base, profile.connect_rolls)
                room_radius = () -> roll_sum(@rng, profile.room_base, profile.room_rolls)
                factor = math.sqrt(number_regions / 5)
                size = {profile.size_base * factor, profile.size_base * factor}
                template = nilprotect {
                    default_wall: Tile.create(Tilesets.grass.wall, true, true, {FLAG_OVERWORLD})
                    subtemplates: {nilprotect {
                        is_overworld: true
                        :size
                        :number_regions
                        floor1: OVERWORLD_TILESET.floor1
                        floor2: OVERWORLD_TILESET.floor2
                        wall1: OVERWORLD_TILESET.wall1
                        wall2: OVERWORLD_TILESET.wall2
                        rect_room_num_range: {0,0} -- disable
                        rect_room_size_range: {10,15}
                        rvo_iterations: 100
                        :connect_line_width
                        :room_radius
                        region_delta_func: spread_region_delta_func
                        -- Dungeon objects/features
                        n_statues: profile.n_statues
                    }}
                    outer_conf: nilprotect {
                        floor1: OVERWORLD_TILESET.floor1
                        floor2: OVERWORLD_TILESET.floor2
                        :connect_line_width
                    }
                    shell: profile.shell
                }
                -- TODO, handle links to overworld...
                if not NewMaps.map_try_create(@map, @map.rng, template)
                    return false
                if not overworld_features(@region_set, profile)
                    return false
                append @map.post_map, () ->
                    if not overworld_items_and_enemies(@region_set, profile)
                        return false
                    return true
                return true
        }
    }
}
