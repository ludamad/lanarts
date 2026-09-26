-- The Bone Wyrm: an ancient dragon of bleached bone, cold soulfire in its eyes and ribs (party level 12).
--   Bone Breath: soulfire gathers in its jaws, then two volleys of bone shards fan out in a cone
--     at whoever keeps furthest away, then at the nearest.
--   Tail Sweep: its reach is marked on the ground, then its tail hits everyone close and throws them back.
--   Bone Rain: markers appear under and around the party; shards fall on them a moment later.
--   Below half health: a roar that slows the party, and lines of bone spikes erupting towards them.
-- Art and effects: tools/bosses/bone_wyrm.py.
DataW = require "DataWrapped"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
Display = require "core.Display"
EventLog = require "ui.EventLog"
ObjectUtils = require "objects.ObjectUtils"
LuaGameObject = require "objects.LuaGameObject"
SpellUtils = require "spells.SpellUtils"
{:boss_name_drawer} = require "enemies.bosses.BossUtils"

NAME = "Bone Wyrm"
TYPES = {"Black"}
OVER = -100  -- drawn over the fighters
GROUND = 100 -- drawn under them

-- Tuning (steps are 1/60 s)
BREATH_WINDUP = 50
BREATH_SHARDS = 11      -- a volley; the second fills the gaps of the first
BREATH_CONE = 0.95      -- radians across
BREATH_GAP = 14         -- steps between the volleys
SHARD_SPEED = 5.5
SHARD_RANGE = 380
SHARD_DAMAGE = 12
SHARD_HITS = 2          -- shards of one breath that can hurt a party member
SWEEP_WINDUP = 44
SWEEP_REACH = 92
SWEEP_DAMAGE = 15
SWEEP_THROW = 13        -- Thrown steps (distance ~ n^2 / 2)
RAIN_WINDUP = 30
RAIN_DELAY = 58         -- steps from a marker appearing to its shard landing
RAIN_EXTRA = 2          -- markers near each party member, besides the one under them
RAIN_RADIUS = 24
RAIN_DAMAGE = 22
ROAR_WINDUP = 36
ROAR_REACH = 300
ROAR_SLOW = 150
SPIKE_WINDUP = 30
SPIKE_EVERY = 26        -- pixels between spikes along a line
SPIKE_REACH = 330
SPIKE_DELAY = 34        -- steps from the first marker to the first spike
SPIKE_RIPPLE = 4        -- steps later for each spike further out
SPIKE_RADIUS = 18
SPIKE_DAMAGE = 19
MAGIC_SHARE = 0.8        -- of the spells' damage (soulfire), the rest is bone
GAP = {90, 60}          -- steps between abilities, by phase

_sprites = nil
sprites = () ->
    _sprites or= {
        shard: tosprite "spr_bosses.bone_wyrm_shard"
        marker: tosprite "spr_bosses.bone_wyrm_marker"
        impact: tosprite "spr_bosses.bone_wyrm_impact"
        ring: tosprite "spr_bosses.bone_wyrm_ring"
        sweep_mark: tosprite "spr_bosses.bone_wyrm_sweep_mark"
        spike: tosprite "spr_bosses.bone_wyrm_spike"
        gather: tosprite "spr_bosses.bone_wyrm_gather"
    }
    return _sprites

-- The party members the boss can hit, on its map
targets = (boss) ->
    return for p in *Map.enemies_list(boss)
        if p.destroyed or p.map ~= boss.map or (not p.is_enemy and p.is_ghost)
            continue
        p

dist = (a, b) -> math.sqrt((a.x - b.x)^2 + (a.y - b.y)^2)

DEBUG = os.getenv("WYRM_DEBUG") and {}
hit = (boss, target, damage, kind) ->
    if not target.destroyed and not boss.destroyed
        hp = target.stats.hp
        SpellUtils.do_damage(boss, TYPES, target, damage, 0, MAGIC_SHARE)
        if DEBUG
            DEBUG[kind] = (DEBUG[kind] or 0) + hp - target.stats.hp

announce = (boss, msg, color = {200, 220, 255}) ->
    if Map.object_visible(boss)
        for _ in screens()
            EventLog.add(msg, color)

-- The shard frame pointing along angle (16 directions, 0 = east, clockwise)
shard_frame = (angle) -> (math.floor(angle / (2 * math.pi) * 16 + 0.5) % 16) * 10

local Fx, Shard, Strike

-- A one-off animation: frames of one of the boss's sprites, optionally following an object,
-- scaled, fading out, or falling from the sky
Fx = LuaGameObject.type_create {
    init: (args) =>
        {:xy, :sprite, :follow, :offset} = args
        @follow, @offset = follow, offset or {0, 0}
        xy = if follow then {follow.x + @offset[1], follow.y + @offset[2]} else xy
        Fx.parent_init(@, xy, 1, false, args.depth or OVER)
        @sprite, @frames, @rate = sprite, args.frames or 1, args.rate or 4
        @duration = args.duration or @frames * @rate
        @scale, @fade, @loop, @frame = args.scale, args.fade, args.loop, args.frame
        @fall, @alpha = args.fall, args.alpha or 1
        @n_steps = 0
    on_step: () =>
        @n_steps += 1
        if @n_steps >= @duration or (@follow and @follow.destroyed)
            GameObject.destroy(@)
            return
        if @follow
            @x, @y = @follow.x + @offset[1], @follow.y + @offset[2]
    on_draw: () =>
        if not Map.object_visible(@)
            return
        frame = @frame or (if @loop then math.floor(@n_steps / @rate) % @frames else math.min(@frames - 1, math.floor(@n_steps / @rate)))
        alpha = @alpha
        if @fade
            alpha *= math.min(1, (@duration - @n_steps) / @fade)
        {x, y} = Display.to_screen_xy(@xy)
        if @fall
            y -= @fall * (1 - @n_steps / @duration)
        s = @scale or 1
        sprites()[@sprite]\draw {origin: Display.CENTER, color: with_alpha(COL_WHITE, alpha), frame: frame * 10, scale: {s, s}}, {x, y}
}

-- A bone shard of the breath: flies straight, hits the first party member it meets, stopped by walls
Shard = LuaGameObject.type_create {
    init: (args) =>
        {:boss, :xy, :angle, :hits} = args
        Shard.parent_init(@, xy, 6, false, OVER)
        @boss, @angle, @hits = boss, angle, hits
        @vx, @vy = math.cos(angle) * SHARD_SPEED, math.sin(angle) * SHARD_SPEED
        @n_steps = 0
    on_step: () =>
        @n_steps += 1
        @x += @vx
        @y += @vy
        if @boss.destroyed or @n_steps * SHARD_SPEED > SHARD_RANGE or Map.radius_tile_check(@map, {@x, @y}, 4)
            if not @boss.destroyed and @n_steps * SHARD_SPEED <= SHARD_RANGE
                GameObject.add_to_level Fx.create {xy: {@x, @y}, sprite: "impact", frames: 5, rate: 3, scale: 0.6}
            GameObject.destroy(@)
            return
        for p in *targets(@boss)
            if dist(@, p) < p.radius + 6
                @hits[p.id] = (@hits[p.id] or 0) + 1
                if @hits[p.id] <= SHARD_HITS
                    hit(@boss, p, SHARD_DAMAGE, "breath")
                GameObject.add_to_level Fx.create {xy: {@x, @y}, sprite: "impact", frames: 5, rate: 3, scale: 0.6}
                GameObject.destroy(@)
                return
    on_draw: () =>
        if Map.object_visible(@)
            alpha = math.min(1, (SHARD_RANGE / SHARD_SPEED - @n_steps) / 8)
            ObjectUtils.screen_draw(sprites().shard, @xy, alpha, shard_frame(@angle))
}

-- A marked spot on the ground that something lands on after a delay: a falling shard (Bone Rain)
-- or an erupting spike (the second phase)
Strike = LuaGameObject.type_create {
    init: (args) =>
        {:boss, :xy, :delay, :kind} = args
        Strike.parent_init(@, xy, 1, false, GROUND)
        @boss, @delay, @kind = boss, delay, kind
        @n_steps = 0
        @fell = false
    on_step: () =>
        @n_steps += 1
        if @boss.destroyed
            GameObject.destroy(@)
            return
        if @kind == "rain" and not @fell and @n_steps >= @delay - 16
            @fell = true
            GameObject.add_to_level Fx.create {xy: {@x, @y}, sprite: "shard", frame: 4, duration: 16, fall: 180}
        if @n_steps >= @delay
            if @kind == "rain"
                GameObject.add_to_level Fx.create {xy: {@x, @y + 2}, sprite: "impact", frames: 5, rate: 4}
                for p in *targets(@boss)
                    if dist(@, p) < RAIN_RADIUS + p.radius
                        hit(@boss, p, RAIN_DAMAGE, "rain")
            else
                GameObject.add_to_level Fx.create {xy: {@x, @y - 13}, sprite: "spike", frames: 6, rate: 7, depth: 0}
                for p in *targets(@boss)
                    if dist(@, p) < SPIKE_RADIUS + p.radius
                        hit(@boss, p, SPIKE_DAMAGE, "spikes")
            GameObject.destroy(@)
    on_draw: () =>
        if Map.object_visible(@)
            alpha = math.min(1, @n_steps / 10)
            frame = math.min(5, math.floor(@n_steps / @delay * 6))
            ObjectUtils.screen_draw(sprites().marker, @xy, alpha, frame * 10)
}

-- Abilities. Each start_ returns the running ability ({kind, n_steps, ...}); STEPS[kind] runs it a
-- step and returns false once it is over. The boss stands still (Bone Wyrm Windup) while it casts.
windup = (boss, steps) ->
    boss\add_effect("Bone Wyrm Windup", steps)
    boss\mark_animation("cast")

nearest = (boss) ->
    best, best_d = nil, math.huge
    for p in *targets(boss)
        d = dist(boss, p)
        if d < best_d
            best, best_d = p, d
    return best, best_d

-- The breath goes for whoever keeps furthest away, then the nearest, in turn
farthest = (boss) ->
    best, best_d = nil, -1
    for p in *targets(boss)
        d = dist(boss, p)
        if d > best_d
            best, best_d = p, d
    return best

start_breath = (boss) ->
    boss.n_breaths += 1
    target = if boss.n_breaths % 2 == 1 then farthest(boss) else nearest(boss)
    windup(boss, BREATH_WINDUP + BREATH_GAP + 4)
    angle = math.atan2(target.y - boss.y, target.x - boss.x)
    mouth = {math.cos(angle) * 46, math.sin(angle) * 30 - 20}
    GameObject.add_to_level Fx.create {follow: boss, offset: mouth, sprite: "gather", frames: 4, rate: math.floor(BREATH_WINDUP / 4), duration: BREATH_WINDUP}
    return {kind: "breath", n_steps: 0, :target, :angle, :mouth, hits: {}}

breath_volley = (boss, ab, shift) ->
    if not ab.target.destroyed  -- keep aiming at its target until the first volley
        ab.angle = math.atan2(ab.target.y - boss.y, ab.target.x - boss.x)
    for i=0,BREATH_SHARDS-1
        t = (i + shift) / (BREATH_SHARDS - 1) - 0.5
        a = ab.angle + t * BREATH_CONE + randomf(-0.03, 0.03)
        GameObject.add_to_level Shard.create {:boss, hits: ab.hits, xy: {boss.x + math.cos(a) * 18, boss.y + math.sin(a) * 14 - 8}, angle: a}

start_sweep = (boss) ->
    windup(boss, SWEEP_WINDUP + 6)
    GameObject.add_to_level Fx.create {follow: boss, offset: {0, 20}, sprite: "sweep_mark", duration: SWEEP_WINDUP, depth: GROUND, alpha: 0.9, scale: 1.25}
    return {kind: "sweep", n_steps: 0}

start_rain = (boss) ->
    windup(boss, RAIN_WINDUP)
    return {kind: "rain", n_steps: 0}

start_roar = (boss) ->
    windup(boss, ROAR_WINDUP + 10)
    announce(boss, "The Bone Wyrm draws a rattling breath...")
    return {kind: "roar", n_steps: 0}

start_spikes = (boss) ->
    windup(boss, SPIKE_WINDUP)
    return {kind: "spikes", n_steps: 0}

STEPS = {
    breath: (ab) =>
        ab.n_steps += 1
        if ab.n_steps == BREATH_WINDUP
            breath_volley(@, ab, 0)
        elseif ab.n_steps == BREATH_WINDUP + BREATH_GAP
            breath_volley(@, ab, 0.5)
        return ab.n_steps < BREATH_WINDUP + BREATH_GAP
    sweep: (ab) =>
        ab.n_steps += 1
        if ab.n_steps == SWEEP_WINDUP - 26  -- the attack clip winds up ~26 steps before its blow
            @mark_animation("attack")
        if ab.n_steps == SWEEP_WINDUP
            GameObject.add_to_level Fx.create {xy: {@x, @y + 20}, sprite: "ring", frames: 6, rate: 3, depth: GROUND, scale: 1.35}
            for p in *targets(@)
                if dist(@, p) < SWEEP_REACH + p.radius
                    hit(@, p, SWEEP_DAMAGE, "sweep")
                    if not p\has_effect("Thrown")
                        thrown = p\add_effect("Thrown", SWEEP_THROW)
                        thrown.angle = math.atan2(p.y - @y, p.x - @x)
        return ab.n_steps < SWEEP_WINDUP + 6
    rain: (ab) =>
        ab.n_steps += 1
        if ab.n_steps == RAIN_WINDUP
            for i, p in ipairs targets(@)
                spots = {{p.x, p.y}}
                for _=1,RAIN_EXTRA
                    a = randomf(0, 2 * math.pi)
                    r = randomf(36, 64)
                    append spots, {p.x + math.cos(a) * r, p.y + math.sin(a) * r}
                for j, xy in ipairs spots
                    if not Map.radius_tile_check(@map, xy, 4)
                        GameObject.add_to_level Strike.create {boss: @, :xy, kind: "rain", delay: RAIN_DELAY + 8 * (j - 1) + 4 * (i - 1)}
        return ab.n_steps < RAIN_WINDUP
    roar: (ab) =>
        ab.n_steps += 1
        if ab.n_steps == ROAR_WINDUP
            announce(@, "The Bone Wyrm roars! Your blood runs cold.", {170, 210, 255})
            GameObject.add_to_level Fx.create {xy: {@x, @y}, sprite: "ring", frames: 6, rate: 4, depth: GROUND, scale: 2.6}
            GameObject.add_to_level Fx.create {xy: {@x, @y}, sprite: "ring", frames: 6, rate: 5, depth: GROUND, scale: 1.5}
            for p in *targets(@)
                if dist(@, p) < ROAR_REACH
                    p\add_effect("Slowed", ROAR_SLOW)
        return ab.n_steps < ROAR_WINDUP + 10
    spikes: (ab) =>
        ab.n_steps += 1
        if ab.n_steps == SPIKE_WINDUP
            for p in *targets(@)
                a = math.atan2(p.y - @y, p.x - @x)
                k = 0
                for d=58,SPIKE_REACH,SPIKE_EVERY
                    xy = {@x + math.cos(a) * d, @y + 20 + math.sin(a) * d}
                    if Map.radius_tile_check(@map, xy, 6)
                        break
                    GameObject.add_to_level Strike.create {boss: @, :xy, kind: "spike", delay: SPIKE_DELAY + SPIKE_RIPPLE * k}
                    k += 1
        return ab.n_steps < SPIKE_WINDUP
}

-- Picks the next ability: the tail for whoever crowds it, else breath and rain in turn; in the
-- second phase spikes join in and it roars again every so often
choose = (boss) ->
    _, near_d = nearest(boss)
    if near_d < SWEEP_REACH and boss.sweep_cooldown <= 0
        boss.sweep_cooldown = 300
        return start_sweep(boss)
    if boss.phase2
        boss.roar_cooldown -= 1
        if boss.roar_cooldown <= 0
            boss.roar_cooldown = 5
            return start_roar(boss)
        boss.n_casts += 1
        return switch boss.n_casts % 3
            when 0 then start_breath(boss)
            when 1 then start_spikes(boss)
            else start_rain(boss)
    boss.n_casts += 1
    return if boss.n_casts % 2 == 0 then start_breath(boss) else start_rain(boss)

DataW.effect_create {
    name: "Bone Wyrm Windup"
    stat_func: (obj, old, new) =>
        new.speed = 0
        obj.stats.attack_cooldown = 2
}

DataW.enemy_create {
    name: NAME
    appear_message: "The Bone Wyrm rises, soulfire burning in its ribs!"
    defeat_message: "The Bone Wyrm collapses into a heap of bones, its soulfire guttering out."
    sprite: "spr_bosses.bone_wyrm"
    radius: 40
    xpaward: 400
    unique: true
    types: TYPES
    resistances: {
        Black: 3
        Green: 3
        Piercing: 2
        Blue: 1
        White: -2
        Bludgeon: -2
        Red: -1
    }
    weapon: {cooldown: 80, damage: 6}
    stats: {
        hp: 850
        hpregen: 0
        movespeed: 2
        strength: 38
        magic: 38
        defence: 22
        willpower: 22
    }
    init_func: () =>
        @ability = nil
        @next_ability = 60
        @n_casts = 0
        @n_breaths = 0
        @sweep_cooldown = 0
        @roar_cooldown = 0
        @phase2 = false
    step_func: () =>
        @sweep_cooldown -= 1
        if not @phase2 and @stats.hp < @stats.max_hp / 2
            @phase2 = true
            @roar_cooldown = 0
            @n_casts = 0  -- after the roar: spikes, then rain, then breath
            announce(@, "The Bone Wyrm's soulfire flares! Bone splinters from the ground!", {140, 200, 255})
        if @ability
            if STEPS[@ability.kind](@, @ability)
                return
            @ability = nil
            @next_ability = GAP[@phase2 and 2 or 1]
            return
        @next_ability -= 1
        if @next_ability > 0 or #targets(@) == 0 or not Map.object_visible(@)
            return
        @ability = choose(@)
    on_post_draw: boss_name_drawer(62)
    death_func: () =>
        if DEBUG
            print "WYRM", GameState.frame, table.concat(["#{k}=#{math.floor(v)}" for k, v in pairs DEBUG], " ")
            for k in pairs DEBUG
                DEBUG[k] = nil
        ItemUtils = require "maps.ItemUtils"
        for level in *{3, 2}
            {:type, :amount} = ItemUtils.randart_generate(level)
            ObjectUtils.spawn_item_near(@, type, amount)
        ObjectUtils.spawn_item_near(@, "Amulet of Fear", 1)
        item = random_choice {"Defence Scroll", "Will Scroll", "Strength Scroll", "Magic Scroll"}
        ObjectUtils.spawn_item_near(@, item, 1)
}
