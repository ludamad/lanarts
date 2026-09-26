-- Lom Lobon, the Storm Eye: a floating eye wrapped in a storm (party level 7).
--   Spark Spiral: rotating arms of slow sparks to weave through.
--   Chain Lightning: marks a party member, then a bolt leaps between them (keep apart).
--   Blink: melee fighters that stay on it too long get a discharge; it teleports away.
--   Below half health: storm orbs circle it and zap whoever comes near.
-- Art and effects: tools/bosses/storm_eye.py.
DataW = require "DataWrapped"
GameObject = require "core.GameObject"
Map = require "core.Map"
EventLog = require "ui.EventLog"
GameState = require "core.GameState"
ObjectUtils = require "objects.ObjectUtils"
LuaGameObject = require "objects.LuaGameObject"
SpellUtils = require "spells.SpellUtils"
{:boss_name_drawer} = require "enemies.bosses.BossUtils"

NAME = "Lom Lobon"
TYPES = {"White"}
EFFECT_DEPTH = -100 -- over the fighters

-- Tuning (steps are 1/60 s)
SPIRAL_WINDUP = 40
SPIRAL_LENGTH = 150
SPIRAL_EVERY = 8       -- a spark from each arm this often
SPIRAL_TURN = 0.035    -- radians per step the arms turn
SPIRAL_FAN_EVERY = 20  -- a fan of three sparks at the farthest party member this often
SPARK_SPEED = 1.9
SPARK_LIFE = 190
SPARK_DAMAGE = 3
SPARK_GRACE = 30      -- steps after a spark hit before another can hit the same one
CHAIN_MARK = 50        -- steps its target is marked before the bolt
CHAIN_HOPS = 4
CHAIN_REACH = 190      -- how far the bolt leaps between party members
CHAIN_DAMAGE = 32
BLINK_CONTACT = 120    -- steps of melee contact before it blinks
BLINK_WINDUP = 24
BLINK_COOLDOWN = 360
BLINK_DAMAGE = 9
ORB_COUNT = 3
ORB_RADIUS = 64
ORB_REACH = 105
ORB_EVERY = 110
ORB_DAMAGE = 2.8

_sprites = nil
sprites = () ->
    _sprites or= {
        spark: tosprite "spr_bosses.storm_spark"
        arc: tosprite "spr_bosses.storm_arc"
        orb: tosprite "spr_bosses.storm_orb"
        blink: tosprite "spr_bosses.storm_blink"
        mark: tosprite "spr_bosses.storm_mark"
    }
    return _sprites

-- The party members the boss can hit, on its map
targets = (boss) ->
    return for p in *Map.enemies_list(boss)
        if p.destroyed or p.map ~= boss.map or (not p.is_enemy and p.is_ghost)
            continue
        p

dist = (a, b) -> math.sqrt((a.x - b.x)^2 + (a.y - b.y)^2)

zap = (boss, target, damage) ->
    if not target.destroyed and not boss.destroyed
        SpellUtils.do_damage(boss, TYPES, target, damage, 0, 1)

-- Draws a lightning bolt from a to b out of the 16-direction arc segments (storm_arc frames:
-- 16 directions, then the same 16 with a fork), flickering between shapes
draw_bolt = (a, b, alpha, seed) ->
    {ax, ay}, {bx, by} = a, b
    dx, dy = bx - ax, by - ay
    d = math.sqrt(dx * dx + dy * dy)
    if d < 4
        return
    k = math.floor(math.atan2(dy, dx) / (2 * math.pi) * 16 + 0.5) % 16
    n = math.max(1, math.ceil(d / 26))
    for i=0,n-1
        t = (i + 0.5) / n
        variant = (seed + i * 7) % 3 == 0 and 1 or 0
        ObjectUtils.screen_draw(sprites().arc, {ax + dx * t, ay + dy * t}, alpha, (k + 16 * variant) * 10)

local Bolt, Flash, Mark, Spark, Orb -- the effect objects refer to their own types

-- A bolt shown for a moment between two points (or objects, followed as they move)
Bolt = LuaGameObject.type_create {
    init: (args) =>
        @src, @dst = args.src, args.dst
        Bolt.parent_init(@, @_xy(@src), 1, false, EFFECT_DEPTH)
        @duration = args.duration or 16
        @n_steps = 0
        @seed = random(0, 100)
    _xy: (p) => if p.x then {p.x, p.y} else p
    on_step: () =>
        @n_steps += 1
        if @n_steps % 3 == 0
            @seed += 1
        if @n_steps >= @duration
            GameObject.destroy(@)
    on_draw: () =>
        if Map.object_visible(@)
            draw_bolt(@_xy(@src), @_xy(@dst), 1 - (@n_steps / @duration)^2, @seed)
}

-- A flash that plays once (the blink)
Flash = LuaGameObject.type_create {
    init: (args) =>
        Flash.parent_init(@, args.xy, 1, false, EFFECT_DEPTH)
        @n_steps = 0
    on_step: () =>
        @n_steps += 1
        if @n_steps >= 25
            GameObject.destroy(@)
    on_draw: () =>
        if Map.object_visible(@)
            ObjectUtils.screen_draw(sprites().blink, @xy, 1, math.floor(@n_steps / 5) * 10)
}

-- The marker over the first target of a Chain Lightning, closing in until the bolt
Mark = LuaGameObject.type_create {
    init: (args) =>
        {:target, :duration} = args
        Mark.parent_init(@, {target.x, target.y}, 1, false, EFFECT_DEPTH)
        @target, @duration, @n_steps = target, duration, 0
    on_step: () =>
        @n_steps += 1
        if @target.destroyed or @n_steps >= @duration
            GameObject.destroy(@)
            return
        @x, @y = @target.x, @target.y
    on_draw: () =>
        if Map.object_visible(@)
            frame = math.min(3, math.floor(@n_steps / @duration * 4))
            ObjectUtils.screen_draw(sprites().mark, @xy, 0.6 + 0.4 * (@n_steps % 8 < 4 and 1 or 0), frame * 10)
}

-- A slow spark of the Spark Spiral: hits once, stopped by walls
Spark = LuaGameObject.type_create {
    init: (args) =>
        {:boss, :xy, :angle} = args
        Spark.parent_init(@, xy, 5, false, EFFECT_DEPTH)
        @boss = boss
        @vx, @vy = math.cos(angle) * SPARK_SPEED, math.sin(angle) * SPARK_SPEED
        @n_steps = 0
    on_step: () =>
        @n_steps += 1
        @x += @vx
        @y += @vy
        if @boss.destroyed or @n_steps > SPARK_LIFE or Map.radius_tile_check(@map, {@x, @y}, 4)
            GameObject.destroy(@)
            return
        for p in *targets(@boss)
            if dist(@, p) < p.radius + 5
                -- a burst of sparks only stings once in a while (they pass through meanwhile)
                last = @boss.spark_hits[p.id]
                if not last or GameState.frame - last >= SPARK_GRACE
                    @boss.spark_hits[p.id] = GameState.frame
                    zap(@boss, p, SPARK_DAMAGE)
                    GameObject.destroy(@)
                    return
    on_draw: () =>
        if Map.object_visible(@)
            alpha = math.min(1, (SPARK_LIFE - @n_steps) / 20)
            ObjectUtils.screen_draw(sprites().spark, @xy, alpha, @n_steps * 2)
}

-- A storm orb circling the boss in its second phase, zapping whoever comes near
Orb = LuaGameObject.type_create {
    init: (args) =>
        {:boss, :angle, :delay} = args
        @boss, @angle = boss, angle
        Orb.parent_init(@, @_place(), 8, false, EFFECT_DEPTH)
        @cooldown = delay
        @n_steps = 0
    _place: () => {@boss.x + math.cos(@angle) * ORB_RADIUS, @boss.y + math.sin(@angle) * ORB_RADIUS * 0.8}
    on_step: () =>
        if @boss.destroyed or @boss.map ~= @map
            GameObject.destroy(@)
            return
        @n_steps += 1
        @angle += 0.03
        xy = @_place()
        @x, @y = xy[1], xy[2]
        @cooldown -= 1
        if @cooldown <= 0
            -- anyone in reach, not just the nearest
            near = [p for p in *targets(@boss) when dist(@, p) < ORB_REACH]
            if #near > 0
                best = random_choice(near)
                GameObject.add_to_level Bolt.create {src: @, dst: best, duration: 14}
                zap(@boss, best, ORB_DAMAGE)
                @cooldown = ORB_EVERY
    on_draw: () =>
        if Map.object_visible(@)
            -- brightens as its next zap nears
            alpha = math.min(1, 0.55 + 0.45 * math.max(0, 1 - @cooldown / 30))
            ObjectUtils.screen_draw(sprites().orb, @xy, alpha, @n_steps)
}

-- Abilities: each is a coroutine-free little state machine run from step_func
farthest = (boss) ->
    best, best_d = nil, -1
    for p in *targets(boss)
        d = dist(boss, p)
        if d > best_d and d < 420
            best, best_d = p, d
    return best

start_spiral = (boss) ->
    boss\mark_animation("cast")
    boss.ability = {kind: "spiral", n_steps: 0, angle: random(0, 628) / 100}

step_spiral = (boss, a) ->
    a.n_steps += 1
    t = a.n_steps - SPIRAL_WINDUP
    if t < 0
        return true
    arms = boss.phase2 and 4 or 3
    turn = boss.phase2 and -SPIRAL_TURN or SPIRAL_TURN
    a.angle += turn
    if t % SPIRAL_EVERY == 0
        for i=1,arms
            angle = a.angle + (i - 1) * 2 * math.pi / arms
            xy = {boss.x + math.cos(angle) * 20, boss.y + math.sin(angle) * 20}
            GameObject.add_to_level Spark.create {:boss, :xy, :angle}
    -- a fan aimed at whoever hangs back, so keeping distance is no escape
    if t % SPIRAL_FAN_EVERY == SPIRAL_FAN_EVERY / 2
        far = farthest(boss)
        if far
            aim = math.atan2(far.y - boss.y, far.x - boss.x)
            for k=-1,1
                angle = aim + k * 0.22
                xy = {boss.x + math.cos(angle) * 20, boss.y + math.sin(angle) * 20}
                GameObject.add_to_level Spark.create {:boss, :xy, :angle}
    return t < SPIRAL_LENGTH

start_chain = (boss) ->
    choices = [p for p in *targets(boss) when dist(boss, p) < 360 and Map.object_visible(p, p.xy)]
    if #choices == 0
        return false
    -- it strikes first at whoever stands furthest back; the bolt leaps on from there
    table.sort(choices, (a, b) -> dist(boss, a) > dist(boss, b))
    target = choices[1]
    boss\mark_animation("cast")
    GameObject.add_to_level Mark.create {:target, duration: CHAIN_MARK}
    boss.ability = {kind: "chain", n_steps: 0, :target}
    return true

step_chain = (boss, a) ->
    a.n_steps += 1
    if a.n_steps == CHAIN_MARK
        if a.target.destroyed
            return false
        a.src, a.hit, a.hops, a.damage = boss, a.target, 0, CHAIN_DAMAGE
    if a.n_steps >= CHAIN_MARK and (a.n_steps - CHAIN_MARK) % 9 == 0
        target = a.hit
        if not target or target.destroyed
            return false
        GameObject.add_to_level Bolt.create {src: a.src, dst: target, duration: 18}
        zap(boss, target, a.damage)
        a.hops += 1
        -- leap on to the nearest other party member in reach
        nxt, best_d = nil, CHAIN_REACH
        for p in *targets(boss)
            if p ~= target and p ~= a.src
                d = dist(p, target)
                if d < best_d
                    nxt, best_d = p, d
        if not nxt and a.src ~= boss
            if dist(a.src, target) < CHAIN_REACH and not a.src.destroyed
                nxt = a.src
        if not nxt or a.hops >= CHAIN_HOPS
            return false
        a.src, a.hit, a.damage = target, nxt, a.damage * 0.8
    return true

try_blink = (boss) ->
    if boss.blink_cooldown > 0 or boss.contact < BLINK_CONTACT
        return false
    boss\mark_animation("cast")
    boss.ability = {kind: "blink", n_steps: 0}
    return true

step_blink = (boss, a) ->
    a.n_steps += 1
    if a.n_steps < BLINK_WINDUP
        return true
    -- a discharge where it stood, then it reappears away from the party
    GameObject.add_to_level Flash.create {xy: {boss.x, boss.y}}
    for p in *targets(boss)
        if dist(boss, p) < boss.radius + p.radius + 24
            GameObject.add_to_level Bolt.create {src: {boss.x, boss.y}, dst: p, duration: 12}
            zap(boss, p, BLINK_DAMAGE)
    party = targets(boss)
    -- somewhere open (not against a wall), out of the party's reach but still in the fight
    best, best_score = nil, -math.huge
    for _=1,60
        angle = random(0, 628) / 100
        r = random(140, 230)
        x, y = boss.x + math.cos(angle) * r, boss.y + math.sin(angle) * r
        if Map.radius_tile_check(boss.map, {x, y}, boss.radius + 4)
            continue
        nearest = math.huge
        for p in *party
            nearest = math.min(nearest, math.sqrt((p.x - x)^2 + (p.y - y)^2))
        score = -math.abs(nearest - 170)
        if Map.radius_tile_check(boss.map, {x, y}, boss.radius + 44)
            score -= 100
        if score > best_score
            best, best_score = {x, y}, score
    if best
        boss.x, boss.y = best[1], best[2]
        GameObject.add_to_level Flash.create {xy: best}
    boss.blink_cooldown = BLINK_COOLDOWN
    boss.contact = 0
    return false

STEPS = {spiral: step_spiral, chain: step_chain, blink: step_blink}

enter_phase2 = (boss) ->
    boss.phase2 = true
    for i=1,ORB_COUNT
        GameObject.add_to_level Orb.create {:boss, angle: (i - 1) * 2 * math.pi / ORB_COUNT, delay: 40 + i * 25}
    if Map.object_visible(boss)
        EventLog.add("The storm around #{NAME} breaks loose!", {180, 160, 255})

DataW.enemy_create {
    name: NAME
    appear_message: "#{NAME}, the Storm Eye, turns its gaze on you!"
    defeat_message: "#{NAME}'s storm blows itself out!"
    sprite: "spr_bosses.storm_eye"
    radius: 20
    xpaward: 220
    unique: true
    types: TYPES
    -- storm-born, it shrugs off some lightning; dark magic unravels it
    resistances: {White: 0.5, Black: -1}
    -- a weak crackle to the touch: staying on it is punished by its Blink instead
    weapon: {damage: 3, cooldown: 70}
    stats: {
        hp: 750
        hpregen: 0.1
        movespeed: 1.6
        strength: 4
        magic: 12
        defence: 14
        willpower: 8
    }
    init_func: () =>
        @ability = nil
        @next_ability = 90
        @rotation = 0
        @contact = 0
        @blink_cooldown = 120
        @phase2 = false
        @spark_hits = {}
    step_func: () =>
        if not @phase2 and @stats.hp < @stats.max_hp / 2
            enter_phase2(@)
        @blink_cooldown -= 1
        near = false
        for p in *targets(@)
            if dist(@, p) < @radius + p.radius + 12
                near = true
        @contact = if near then @contact + 1 else math.max(0, @contact - 2)
        if @ability
            if STEPS[@ability.kind](@, @ability)
                return
            @ability = nil
            @next_ability = @phase2 and 70 or 110
            return
        if try_blink(@)
            return
        @next_ability -= 1
        if @next_ability > 0 or #targets(@) == 0 or not Map.object_visible(@)
            return
        @rotation += 1
        if @rotation % 2 == 1
            start_spiral(@)
        elseif not start_chain(@)
            start_spiral(@)
    on_post_draw: boss_name_drawer(50) -- above the eye, which floats high over its shadow
    death_func: () =>
        ItemUtils = require "maps.ItemUtils"
        ObjectUtils.spawn_item_near(@, "Amulet of Light", 1)
        {:type, :amount} = ItemUtils.randart_generate(2)
        ObjectUtils.spawn_item_near(@, type, amount)
        item = random_choice {"Will Scroll", "Magic Scroll"}
        ObjectUtils.spawn_item_near(@, item, 1)
}
