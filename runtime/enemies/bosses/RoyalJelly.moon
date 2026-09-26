-- The Royal Jelly: the hive's queen, a quivering mass of royal jelly with a queen curled in its core.
-- Sprites and effects: tools/bosses/royal_jelly.py. Fought at party level 5.
--   Honey Glob   lobs a glob at a player (a ring marks where it lands); it splashes and leaves a
--                sticky honey pool that slows anyone standing in it
--   Brood Burst  winds up (cast clip) and bees burst out of the jelly
--   Engulf       a slow, heavy slam around itself, telegraphed by a closing ring and its attack clip
--   Split        below half health it sheds slimes, turns angrier and faster, and keeps shedding
DataW = require "DataWrapped"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
Display = require "core.Display"
EventLog = require "ui.EventLog"
ObjectUtils = require "objects.ObjectUtils"
LuaGameObject = require "objects.LuaGameObject"
{:boss_name_drawer} = require "enemies.bosses.BossUtils"

-- Effect sprites, looked up on first use (royal_jelly_glob, _pool, _brood, _slam, _splash)
SPRITES = setmetatable {}, __index: (k) =>
    sprite = tosprite "spr_bosses.royal_jelly_#{k}"
    rawset(@, k, sprite)
    return sprite

GLOB_FLIGHT = 50     -- steps a glob is in the air
GLOB_RADIUS = 26     -- splash radius
POOL_DURATION = 300
BEE_LIFESPAN = 300   -- the brood are short-lived: steps before a bee dies off
SLIME_LIFESPAN = 900
POOL_RX, POOL_RY = 30, 18
ENGULF_WINDUP = 36   -- steps the ring closes in before the attack clip starts
ENGULF_STRIKE = 28   -- steps from the attack clip starting to its impact frame
ENGULF_RADIUS = 60
BROOD_WINDUP = 40    -- steps of the cast clip before the bees burst out


message = (text, colour) ->
    for _ in screens()
        EventLog.add(text, colour)

distance = (a, b) -> math.sqrt((a.x - b.x)^2 + (a.y - b.y)^2)

-- Players (and their allies) near xy within radius; squash scales the vertical distance
foes_near = (boss, xy, radius, squash = 1) ->
    found = {}
    for foe in *Map.enemies_list(boss)
        dx, dy = foe.x - xy[1], (foe.y - xy[2]) * squash
        if dx * dx + dy * dy <= (radius + foe.radius)^2
            append found, foe
    return found

-- A short-lived effect sprite, drawn once through its frames (0.1 frames a step)
FxBurst = LuaGameObject.type_create()
FxBurst.init = (args) =>
    FxBurst.parent_init(@, args.xy, 1, false, -60)
    @sprite, @duration, @n_steps = SPRITES[args.sprite], args.duration, 0
FxBurst.on_step = () =>
    @n_steps += 1
    if @n_steps >= @duration
        GameObject.destroy(@)
FxBurst.on_draw = () =>
    if Display.object_within_view(@)
        ObjectUtils.screen_draw(@sprite, @xy, 1, @n_steps)

-- Sticky honey on the ground: slows the party standing in it
HoneyPool = LuaGameObject.type_create()
HoneyPool.init = (args) =>
    HoneyPool.parent_init(@, args.xy, POOL_RX, false, 95)
    @n_steps = 0
    @boss = args.boss
HoneyPool.on_step = () =>
    @n_steps += 1
    if @n_steps >= POOL_DURATION
        GameObject.destroy(@)
        return
    if @boss.destroyed or @n_steps % 5 ~= 0
        return
    for foe in *foes_near(@boss, @xy, POOL_RX - 4, POOL_RX / POOL_RY)
        if not foe\has_effect("Sticky Honey")
            message("You are stuck in the sticky honey!", {255, 210, 120})
        foe\add_effect("Sticky Honey", 20)
HoneyPool.on_draw = () =>
    if Display.object_within_view(@)
        left = POOL_DURATION - @n_steps
        alpha = math.min(1, @n_steps / 10, left / 60)
        grow = math.min(1, 0.5 + @n_steps / 16)
        SPRITES.pool\draw({origin: Display.CENTER, color: with_alpha(COL_WHITE, alpha * 0.9), frame: @n_steps, scale: {grow, grow}}, Display.to_screen_xy(@xy))

-- A glob of jelly lobbed in an arc at where its target stood; a ring marks the landing spot
HoneyGlob = LuaGameObject.type_create()
HoneyGlob.init = (args) =>
    HoneyGlob.parent_init(@, args.from, 8, false, -50)
    @from, @to, @boss = args.from, args.to, args.boss
    @n_steps = -(args.delay or 0)
HoneyGlob.on_step = () =>
    @n_steps += 1
    if @n_steps >= GLOB_FLIGHT
        if not @boss.destroyed
            eff = @boss\effective_stats()
            for foe in *foes_near(@boss, @to, GLOB_RADIUS, 1.3)
                foe\damage(random(4, 6) + eff.magic * 0.25, eff.magic, 0.5, @boss)
            GameObject.add_to_level HoneyPool.create {xy: @to, boss: @boss}
        GameObject.add_to_level FxBurst.create {xy: @to, sprite: "splash", duration: 28}
        GameObject.destroy(@)
HoneyGlob.on_draw = () =>
    if @n_steps < 0
        return
    t = @n_steps / GLOB_FLIGHT
    -- the landing marker, tightening as the glob comes down
    s = 0.75 - 0.3 * t
    SPRITES.slam\draw({origin: Display.CENTER, color: with_alpha(COL_WHITE, 0.35 + 0.5 * t), scale: {s, s}}, Display.to_screen_xy(@to))
    x = @from[1] + (@to[1] - @from[1]) * t
    y = @from[2] + (@to[2] - @from[2]) * t - math.sin(math.pi * t) * 70 - 30 * (1 - t)
    ObjectUtils.screen_draw(SPRITES.glob, {x, y}, 1, @n_steps)

-- Engulf's telegraph: a ring around the boss closing in until the slam lands
EngulfRing = LuaGameObject.type_create()
EngulfRing.init = (args) =>
    EngulfRing.parent_init(@, {args.boss.x, args.boss.y}, 1, false, 94)
    @boss, @duration, @n_steps = args.boss, args.duration, 0
EngulfRing.on_step = () =>
    @n_steps += 1
    if @boss.destroyed or @n_steps >= @duration
        GameObject.destroy(@)
        return
    @xy = {@boss.x, @boss.y + 8}
EngulfRing.on_draw = () =>
    t = @n_steps / @duration
    s = (ENGULF_RADIUS * 2 + 24) / 64 * (1.25 - 0.25 * t)
    pulse = 0.5 + 0.5 * math.abs(math.sin(@n_steps * 0.25))
    SPRITES.slam\draw({origin: Display.CENTER, color: with_alpha(COL_WHITE, 0.4 + 0.5 * t * pulse), scale: {s, s}}, Display.to_screen_xy(@xy))

DataW.effect_create {
    name: "Royal Jelly Fury"
    effected_colour: {255, 150, 110}
    stat_func: (obj, old, new) =>
        new.speed *= 1.2
        new.melee_cooldown_multiplier *= 0.85
        new.magic += 3
}

-- Standing in a honey pool: movement at half speed (attacks are unhindered)
DataW.effect_create {
    name: "Sticky Honey"
    effected_sprite: "spr_effects.slowed"
    effected_colour: {255, 220, 130}
    fade_out: 5
    stat_func: (obj, old, new) =>
        new.speed /= 2
}

-- The newly hatched brood and shed slimes: weaker than their grown kin, tinted with jelly
DataW.effect_create {
    name: "Royal Brood"
    effected_colour: {255, 225, 140}
    stat_func: (obj, old, new) =>
        new.strength *= 0.25
        new.melee_cooldown_multiplier *= 1.5
}

DataW.enemy_create {
    name: "Royal Jelly"
    appear_message: "The Royal Jelly quivers with rage!"
    defeat_message: "The Royal Jelly splatters apart!"
    description: "The hive's queen, a quivering mass of royal jelly. Something curls in its core."
    sprite: "spr_bosses.royal_jelly"
    death_sprite: "green blood"
    radius: 20
    xpaward: 150
    unique: true
    types: {"Green"}
    weapon: {cooldown: 100, damage_multiplier: 0.35}
    stats: {
        hp: 600
        hpregen: 0.1
        movespeed: 2
        strength: 13
        magic: 16
        defence: 8
        willpower: 10
    }
    methods: {
        _target: () =>
            -- whoever keeps their distance (the farthest foe in sight); with everyone in its
            -- face, take turns
            best, best_d, other = nil, -1, nil
            for foe in *Map.enemies_list(@)
                d = distance(@, foe)
                if d < 400 and Map.object_visible(foe)
                    if d > best_d
                        best, best_d = foe, d
                    if foe ~= @last_target
                        other = foe
            if best_d < 80 and other
                best = other
            @last_target = best
            return best
        _alive: (list) =>
            return [mon for mon in *list when not mon.destroyed]
        _honey_glob: (target) =>
            @mark_animation("cast")
            -- a volley: one at the target, the rest splattering around it
            for i=1,3
                to = {target.x, target.y}
                if i > 1
                    angle = random(0, 628) / 100
                    to = {target.x + math.cos(angle) * random(40, 70), target.y + math.sin(angle) * random(30, 50)}
                    if Map.radius_tile_check(@map, to, 8)
                        continue
                GameObject.add_to_level HoneyGlob.create {
                    from: {@x, @y - 20}, :to, boss: @, delay: (i - 1) * 8
                }
            @glob_timer = if @fury then 130 else 180
        _brood_burst: () =>
            @brood_target = @_target()
            @brood = @_alive(@brood)
            if #@brood >= (if @fury then 3 else 2)
                @brood_timer = 120
                return
            @mark_animation("cast")
            @pending_brood = BROOD_WINDUP
            @brood_timer = if @fury then 480 else 540
        _burst_out: () =>
            GameObject.add_to_level FxBurst.create {xy: {@x, @y - 16}, sprite: "brood", duration: 48}
            bees = if @fury then {"Poison Bee", "Giant Bee"} else {"Giant Bee"}
            -- the brood swarm out at whoever keeps their distance, landing just short of them
            target = @brood_target
            if target and target.destroyed
                target = nil
            for i, bee in ipairs bees
                angle = i * 2 * math.pi / #bees + random(0, 100) / 100
                xy = {@x + math.cos(angle) * 30, @y + math.sin(angle) * 24}
                if target
                    d = math.max(1, distance(@, target))
                    along = math.max(0, d - 40) / d
                    xy = {@x + (target.x - @x) * along + math.cos(angle) * 16, @y + (target.y - @y) * along + math.sin(angle) * 12}
                if Map.radius_tile_check(@map, xy, 10)
                    xy = {@x, @y}
                GameObject.add_to_level FxBurst.create {:xy, sprite: "splash", duration: 28}
                mon = GameObject.enemy_create {type: bee, :xy, team: @team}
                append @expires, {mon, @t + BEE_LIFESPAN}
                mon\add_effect("Royal Brood", BEE_LIFESPAN + 60)
                append @brood, mon
            message("Bees burst out of the Royal Jelly!", {255, 220, 100})
        _engulf: () =>
            GameObject.add_to_level EngulfRing.create {boss: @, duration: ENGULF_WINDUP + ENGULF_STRIKE}
            @pending_engulf = ENGULF_WINDUP + ENGULF_STRIKE
            @engulf_timer = if @fury then 240 else 300
        _slam: () =>
            eff = @effective_stats()
            hit = false
            for foe in *foes_near(@, {@x, @y + 8}, ENGULF_RADIUS, 1.3)
                foe\damage(random(6, 9) + eff.strength * 0.35, eff.strength, 0.5, @)
                hit = true
            GameObject.add_to_level FxBurst.create {xy: {@x, @y + 8}, sprite: "splash", duration: 28}
            if hit
                message("The Royal Jelly engulfs you!", {255, 170, 90})
        _split: () =>
            @slimes = @_alive(@slimes)
            n = math.min(if @fury then 1 else 2, 2 - #@slimes)
            for i=1,n
                angle = random(0, 628) / 100
                xy = {@x + math.cos(angle) * 34, @y + math.sin(angle) * 26}
                if Map.radius_tile_check(@map, xy, 14)
                    xy = {@x, @y}
                mon = GameObject.enemy_create {type: "Green Slime", :xy, team: @team}
                append @expires, {mon, @t + SLIME_LIFESPAN}
                mon\add_effect("Royal Brood", SLIME_LIFESPAN + 60)
                append @slimes, mon
            @split_timer = 600
    }
    init_func: () =>
        @glob_timer, @brood_timer, @engulf_timer, @split_timer = 90, 240, 60, 0
        @pending_brood, @pending_engulf = 0, 0
        @brood, @slimes, @expires = {}, {}, {}
        @t = 0
        @fury = false
    step_func: () =>
        -- scripted abilities in flight
        if @pending_brood > 0
            @pending_brood -= 1
            if @pending_brood == 0
                @_burst_out()
        if @pending_engulf > 0
            @pending_engulf -= 1
            if @pending_engulf == ENGULF_STRIKE
                @mark_animation("attack")
            if @pending_engulf == 0
                @_slam()
        -- below half health: split and turn furious
        if not @fury and @stats.hp < @stats.max_hp / 2
            @fury = true
            @add_effect("Royal Jelly Fury", 1000000)
            message("The Royal Jelly splits, and its brood seethes with fury!", {255, 140, 90})
            GameObject.add_to_level FxBurst.create {xy: {@x, @y - 16}, sprite: "brood", duration: 48}
            @_split()
        @t += 1
        -- the brood and slimes are short-lived (an array, for a deterministic order)
        for {mon, t} in *@expires
            if not mon.destroyed and @t >= t
                mon\direct_damage(mon.stats.hp + 1)
        @expires = [e for e in *@expires when not e[1].destroyed]
        if @fury
            @split_timer -= 1
            if @split_timer <= 0
                @_split()
        if not Map.object_visible(@) or @pending_brood > 0 or @pending_engulf > 0
            return
        @glob_timer -= 1
        @brood_timer -= 1
        @engulf_timer -= 1
        -- Engulf when someone is close, else a glob or the brood
        near = foes_near(@, {@x, @y + 8}, ENGULF_RADIUS - 10, 1.3)
        if @engulf_timer <= 0 and #near > 0
            @_engulf()
        elseif @brood_timer <= 0
            @_brood_burst()
        elseif @glob_timer <= 0
            target = @_target()
            if target
                @_honey_glob(target)
    on_post_draw: boss_name_drawer(58) -- above the tall jelly
    death_func: () =>
        -- its brood and slimes melt away with it
        for {mon} in *@expires
            if not mon.destroyed
                mon\direct_damage(mon.stats.hp + 1)
        ItemUtils = require "maps.ItemUtils"
        ObjectUtils.spawn_item_near(@, "Honeycomb", 2)
        item = random_choice {"Will Scroll", "Strength Scroll", "Defence Scroll", "Magic Scroll"}
        ObjectUtils.spawn_item_near(@, item, 1)
        {:type, :amount} = ItemUtils.randart_generate(1)
        ObjectUtils.spawn_item_near(@, type, amount)
}
