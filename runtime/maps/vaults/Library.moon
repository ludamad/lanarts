----
-- Every vault of the kit (see Kit.moon), and picking them for overworld features and dungeon
-- entrances. engine.StartVaultCheck checks them all; tools/vaults renders them.
----

Kit = require "maps.vaults.Kit"

FILES = {
    "Common", "Classic", "Ringwood", "Farmland", "Crossroads", "Craters", "Spiral", "Rootways", "Patchwork"
    "Entrances", "TempleEntrances", "SnakeEntrances", "HiveEntrances", "UnderdungeonEntrances", "OutpostEntrances"
}

-- Overworld themes: each layout in the rotation has its own, and the layouts kept for other
-- dungeons borrow one (see the create() call closing each map_descs/Overworld*.moon)
THEMES = {"classic", "ringwood", "farmland", "crossroads", "craters", "spiral", "rootways", "patchwork"}
-- Dungeons with entrances of their own, and the palette their entrances are drawn in unless the
-- vault sets one
DUNGEONS = {"temple", "snake", "hive", "underdungeon", "outpost", "ogre"}
DUNGEON_PALETTES = {temple: "temple", snake: "snake", hive: "hive", underdungeon: "underdungeon", outpost: "snake", ogre: "orc"}

ALL = {}
BY_NAME = {}
for file in *FILES
    for def in *require("maps.vaults.#{file}")
        assert not BY_NAME[def.name], "Duplicate vault #{def.name}"
        def.file = file
        BY_NAME[def.name] = def
        append ALL, def

has = (list, value) ->
    for v in *list
        if v == value
            return true
    return false

-- The vaults of kind for theme (or dungeon); with exclusive, not those for "any"
matching = (kind, theme, exclusive) ->
    return for def in *ALL
        if def.kind ~= kind or not (has(def.themes, theme) or (not exclusive and has(def.themes, "any")))
            continue
        def

-- Picks a vault of kind for theme: most often one of the theme's own, not in used (a set of
-- names) if that leaves any. A forced vault (Kit.force_name) of that kind and theme always wins.
pick = (rng, kind, theme, used = {}) ->
    forced = BY_NAME[Kit.force_name]
    if forced and forced.kind == kind and (has(forced.themes, theme) or has(forced.themes, "any"))
        return forced
    own = matching(kind, theme, true)
    pool = if #own > 0 and rng\random(0, 4) > 0 then own else matching(kind, theme, false)
    fresh = [def for def in *pool when not used[def.name]]
    if #fresh > 0
        pool = fresh
    if #pool == 0
        return nil
    return rng\random_choice(pool)

return {:ALL, :BY_NAME, :THEMES, :DUNGEONS, :DUNGEON_PALETTES, :matching, :pick}
