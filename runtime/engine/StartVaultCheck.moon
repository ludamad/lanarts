-- Checks every vault of maps/vaults (see maps/vaults/Kit.moon) without generating a map, and with
-- --export writes them all as JSON for tools/vaults/render.py:
--   ./lanarts engine.StartVaultCheck [--export vaults.json]
-- Each layout must build, be entered from its '+' squares with every open square reachable, and fit
-- its kind's size. An entrance also needs exactly one portal, room for four players, no enemies,
-- and doors that seal the portal off from outside (some entrances get locked doors).
-- tools/vaults/check.sh then places each vault in generated worlds.
argparse = require "argparse"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"

MAX_SIZE = {entrance: {20, 16}, default: {30, 30}}
MIN_PLAYER_SQUARES = 4

json_encode = (value) ->
    switch type(value)
        when "table"
            if #value > 0 or next(value) == nil
                return "[" .. table.concat([json_encode(v) for v in *value], ",") .. "]"
            keys = [k for k in pairs value]
            table.sort keys
            return "{" .. table.concat(["#{json_encode(k)}:#{json_encode(value[k])}" for k in *keys], ",") .. "}"
        when "string"
            return '"' .. value\gsub('[%c"\\]', (c) -> string.format("\\u%04x", c\byte())) .. '"'
        else
            return tostring(value)

-- Problems with layout i of def, as strings
check_layout = (Kit, def, i) ->
    problems = {}
    ok, err = pcall Kit.template, def, {}, i
    if not ok
        return {tostring(err)}
    rows = Kit.layout_rows(def.layouts[i])
    h, w = #rows, #rows[1]
    limit = MAX_SIZE[def.kind] or MAX_SIZE.default
    if w > limit[1] or h > limit[2]
        append problems, "#{w}x#{h} is over the #{limit[1]}x#{limit[2]} limit for #{def.kind}"
    at = (x, y) -> rows[y]\sub(x, x)
    spec = (x, y) -> Kit.spec_of(def, at(x, y))
    open = (x, y) -> not spec(x, y).keep and not spec(x, y).solid and not (spec(x, y).decor and spec(x, y).decor ~= "skull")
    count = {}
    for y=1,h
        for x=1,w
            s = spec(x, y)
            key = s.spawn or (s.need == "walkable" and "entry") or "other"
            count[key] = (count[key] or 0) + 1

    -- Flood fill from starts through squares passing ok(x, y); returns the set of reached keys
    flood = (starts, ok) ->
        seen, stack = {}, {}
        for {x, y} in *starts
            seen[y * 100 + x] = true
            append stack, {x, y}
        while #stack > 0
            {x, y} = table.remove(stack)
            for {dx, dy} in *{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
                nx, ny = x + dx, y + dy
                if nx >= 1 and ny >= 1 and nx <= w and ny <= h and not seen[ny * 100 + nx] and ok(nx, ny)
                    seen[ny * 100 + nx] = true
                    append stack, {nx, ny}
        return seen

    entries = {}
    for y=1,h
        for x=1,w
            if spec(x, y).need == "walkable"
                append entries, {x, y}
            -- Open squares on the rim touch the ground around the vault (the overworld joins any
            -- pocket up afterwards); entrances also go in dungeons, which don't, so they need '+'
            elseif def.kind ~= "entrance" and open(x, y) and (x == 1 or y == 1 or x == w or y == h)
                append entries, {x, y}
    if #entries == 0
        append problems, "no way in: no '+' square"
    reached = flood(entries, (x, y) -> open(x, y) or spec(x, y).need == "walkable")
    unreached = 0
    for y=1,h
        for x=1,w
            if open(x, y) and not reached[y * 100 + x]
                unreached += 1
    if unreached > 0
        append problems, "#{unreached} open square(s) can't be reached from outside"

    -- A '.' walled in by the vault becomes a sealed pocket wherever it lands on open ground
    rim = {}
    for y=1,h
        for x=1,w
            if (x == 1 or y == 1 or x == w or y == h) and (spec(x, y).keep or open(x, y))
                append rim, {x, y}
    outer = flood(rim, (x, y) -> spec(x, y).keep or open(x, y))
    enclosed = 0
    for y=1,h
        for x=1,w
            if spec(x, y).keep and not outer[y * 100 + x]
                enclosed += 1
    if enclosed > 0
        append problems, "#{enclosed} '.' square(s) walled in by the vault; make them solid or open"

    if def.kind == "entrance"
        if (count.portal or 0) ~= 1
            append problems, "#{count.portal or 0} portals, needs exactly 1"
        if (count.player or 0) < MIN_PLAYER_SQUARES
            append problems, "#{count.player or 0} player squares, needs #{MIN_PLAYER_SQUARES}"
        if count.enemy or count.brute
            append problems, "enemies in an entrance"
        -- Sealed: from anything outside the vault, the portal can't be reached without a door
        outside = {}
        for y=1,h
            for x=1,w
                if spec(x, y).keep or ((x == 1 or y == 1 or x == w or y == h) and open(x, y))
                    append outside, {x, y}
        walked = flood(outside, (x, y) -> (spec(x, y).keep or open(x, y)) and spec(x, y).spawn ~= "door")
        for y=1,h
            for x=1,w
                if spec(x, y).spawn == "portal" and walked[y * 100 + x]
                    append problems, "the portal can be reached without passing a door"
    else
        if count.portal
            append problems, "a portal outside an entrance"
    return problems

-- def as plain data for the gallery
export_def = (Kit, Library, def) ->
    -- Entrances for any dungeon are shown in the temple's palette
    dungeon = if def.kind == "entrance" then (if def.themes[1] == "any" then "temple" else def.themes[1])
    palette = Kit.palette_of(def, {palette: Library.DUNGEON_PALETTES[dungeon]})
    legend = {}
    for layout in *def.layouts
        for c in layout\gmatch("[^\n]")
            spec = Kit.spec_of(def, c)
            if spec and not legend[c]
                legend[c] = {
                    tile: (if spec.keep then nil else Kit.resolve_ref(spec.tile or def.base or "floor", palette))
                    keep: spec.keep, need: spec.need, solid: spec.solid, seethrough: spec.seethrough
                    spawn: spec.spawn, decor: spec.decor, chance: spec.chance
                }
    return {
        name: def.name, title: def.title, blurb: def.blurb, kind: def.kind, themes: def.themes, file: def.file
        palette: def.palette, enemies: def.enemies, brutes: def.brutes, :legend
        layouts: [Kit.layout_rows(layout) for layout in *def.layouts]
    }

run = (raw_args) ->
    parser = argparse("lanarts", "Vault check")
    parser\option("--export", "Write every vault as JSON to this file.")
    args = parser\parse(raw_args)
    check = () ->
        return ResourceLoading.ensure_resources_before () ->
            Kit = require "maps.vaults.Kit"
            Library = require "maps.vaults.Library"
            n_layouts, n_bad = 0, 0
            kinds = {}
            for def in *Library.ALL
                kinds[def.kind] = (kinds[def.kind] or 0) + 1
                for field in *{"name", "title", "blurb", "kind", "themes", "layouts"}
                    if not def[field]
                        print "BAD #{def.name or '?'}: no #{field}"
                        n_bad += 1
                for i=1,#def.layouts
                    n_layouts += 1
                    for problem in *check_layout(Kit, def, i)
                        print "BAD #{def.name}:#{i}: #{problem}"
                        n_bad += 1
            for theme in *Library.THEMES
                for kind in *{"store", "treasure", "encounter", "landmark"}
                    if #Library.matching(kind, theme, false) == 0
                        print "BAD theme #{theme}: no #{kind} vault"
                        n_bad += 1
            for dungeon in *Library.DUNGEONS
                if #Library.matching("entrance", dungeon, false) == 0
                    print "BAD dungeon #{dungeon}: no entrance"
                    n_bad += 1
            summary = [k .. "=" .. v for k, v in pairs kinds]
            table.sort summary
            print "VAULTS #{#Library.ALL} layouts=#{n_layouts} #{table.concat(summary, ' ')} problems=#{n_bad}"
            if args.export
                f = io.open(args.export, "w")
                f\write(json_encode {
                    vaults: [export_def(Kit, Library, def) for def in *Library.ALL]
                    themes: Library.THEMES, dungeons: Library.DUNGEONS
                    palettes: Kit.PALETTES, default_palette: Kit.DEFAULT_PALETTE
                })
                f\close()
            os.exit(n_bad == 0 and 0 or 1)

    return StartEngine.start_engine {
        settings: {
            fullscreen: false
            view_width: 640
            view_height: 480
            font: 'fonts/Gudea-Regular.ttf'
            menu_font: 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            time_per_step: 0
            invincible: true
        },
        entry_point: check,
        on_exit: () -> nil
    }

return run
