-- Stages a boss fight to watch or record: a bot party of the given class and level against the
-- given monsters in map_descs/BossArena, drawn every step (graphical builds only). With --out, saves every
-- --every'th frame there as frame_NNNNN.bmp (tools/bosses/record.sh turns them into a clip).
--   ./lanarts engine.StartBossShowcase --monsters "Red Dragon" --class Fighter --level 8 [--players 2]
--       [--steps 1500] [--out DIR --every 3]
-- The party take real damage but are topped up below a quarter of their health, so a recording
-- shows hits land without ending early; engine.StartCombatSim measures who would win unaided.
-- Run it with SDL_AUDIODRIVER=dummy for silence (tools/bosses/record.sh does).
argparse = require "argparse"
EngineInternal = require "core.EngineInternal"
ResourceLoading = require "engine.ResourceLoading"
StartEngine = require "engine.StartEngine"
EngineHooks = require "engine.EngineBase"
Map = require "core.Map"
World = require "core.World"

-- Bots: close in on the nearest boss (summons after), fighters swing, casters keep some distance
-- and cast their first spell, falling back on their weapon without mana. Each hero takes its own
-- side of the boss (slot), so the party spreads out as a real one would.
make_step = (class_name, bosses, slot) -> () =>
    player = @player
    best, best_d = nil, math.huge
    for pass in *{true, false}
        for e in *Map.enemies_list(player)
            if (bosses[e.id] ~= nil) == pass
                d = math.sqrt((e.x - player.x)^2 + (e.y - player.y)^2)
                if d < best_d
                    best, best_d = e, d
        if best
            break
    if not best
        return
    melee = (class_name == "Fighter" or class_name == "Rogue" or class_name == "Templar" or player.stats.mp < 10)
    want = if melee then (player.radius + best.radius + 4) else 150
    -- The spot to fight from: out from the boss towards the first hero, turned by this hero's slot
    lead = World.players[1].instance
    a = math.atan2(lead.y - best.y, lead.x - best.x) + (slot - 1) * 1.0
    tx, ty = best.x + math.cos(a) * want, best.y + math.sin(a) * want
    dx, dy = tx - player.x, ty - player.y
    dist = math.sqrt(dx * dx + dy * dy)
    if dist > 6
        @set("move_direction", {dx / dist, dy / dist})
    if melee
        @set("should_use_weapon", true)
    else
        @set("use_spell_slot", 0)

run = (raw_args) ->
    parser = argparse("lanarts", "Boss showcase")
    parser\option("--monsters", "Monsters to fight, as name[:count][;name[:count]...].")
    parser\option("--class", "Party class.", "Fighter")
    parser\option("--level", "Party level.", "1")
    parser\option("--players", "Party size.", "1")
    parser\option("--tileset", "Arena tileset (tiles/Tilesets.lua).", "pebble")
    parser\option("--seed", "Seed.", "1")
    parser\option("--steps", "Frames to run.", "1500")
    parser\option("--out", "Directory to save frames in.")
    parser\option("--every", "Save every n'th frame.", "3")
    args = parser\parse(raw_args)
    monsters = {}
    for part in args.monsters\gmatch("[^;]+")
        name, count = part\match("^(.-):(%d+)$")
        for _=1,tonumber(count or 1)
            append monsters, name or part
    bosses = {}
    local step

    game_start = () ->
        return ResourceLoading.ensure_resources_before () ->
            GameState = require "core.GameState"
            GameState.clear_players()
            for i=1,tonumber(args.players)
                input_source = (player) ->
                    for _=2,tonumber(args.level)
                        player\gain_level()
                    player.stats.hp = player.stats.max_hp
                    player.stats.mp = player.stats.max_mp
                    source = require("input.ProgrammableInputSource").create(player)
                    source.step = make_step(args.class, bosses, i)
                    return source
                GameState.register_player "Hero" .. i, args.class, input_source, true, i - 1, 0
            EngineInternal.init_gamestate()
            random_seed(tonumber(args.seed))
            Engine.first_map_create = () ->
                return require("map_descs.BossArena").generate_and_spawn(monsters, args.tileset)
            EngineInternal.start_game()
            GameState.input_capture()
            GameState.input_handle()
            GameState.frame = 0
            for e in *Map.enemies_list(World.players[1].instance)
                bosses[e.id] = true
            return step

    step = () ->
        GameState = require "core.GameState"
        GameLoop = require "GameLoop"
        Display = require "core.Display"
        frame = GameState.frame
        if frame >= tonumber(args.steps)
            print "SHOWCASE DONE frame=#{frame}"
            os.exit(0)
        for {:instance} in *World.players
            if instance.stats.hp < instance.stats.max_hp / 4
                instance.stats.hp = instance.stats.max_hp
        if args.out and frame % tonumber(args.every) == 0
            Display.capture_next_frame(string.format("%s/frame_%05d.bmp", args.out, frame))
        if not GameLoop.game_step(false)
            print "SHOWCASE DONE frame=#{frame} (game loop exited)"
            os.exit(0)
        return step

    EngineHooks.game_won = () -> nil
    return StartEngine.start_engine {
        settings: {
            fullscreen: false
            view_width: 800
            view_height: 600
            font: 'fonts/Gudea-Regular.ttf'
            menu_font: 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            time_per_step: 0
            invincible: false
            single_screen: true -- one view, of the first hero
        },
        entry_point: game_start,
        on_exit: () -> nil
    }

return run
