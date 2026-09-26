-- Ereshkigal, Queen Below: ruler of the dead, for a party of about level 9 (her arena: crypt).
--   Raise the Dead: graves burst up around the party; after a wind-up each opens and a Skeleton,
--     Zombie or Mummy climbs out.
--   Soul Chains: a chain to a player that drains their life into her, until it runs out or they
--     get out of its reach.
--   Wail: the voices of the dead gather into her, then a ring of wailing spirits sweeps out,
--     hurting and frightening (slowing) whoever it passes.
--   Below half health she turns spectral: faster, see-through, and some blows pass through her.
-- Her sprites and spell effects are built by tools/bosses/ereshkigal.py.
DataW = require "DataWrapped"
GameObject = require "core.GameObject"
GameState = require "core.GameState"
Map = require "core.Map"
Display = require "core.Display"
EventLog = require "ui.EventLog"
ObjectUtils = require "objects.ObjectUtils"
LuaGameObject = require "objects.LuaGameObject"
TypeEffectUtils = require "spells.TypeEffectUtils"
{:boss_name_drawer} = require "enemies.bosses.BossUtils"

draw_name = boss_name_drawer(58) -- clear of her crown

GREEN = {156, 240, 122}
PALE = {232, 255, 224}

-- Tuning, in steps (60 a second) and pixels
RAISE = {first: 150, cooldown: 780, windup: 40, graves: 1, spectral_graves: 2, max_alive: 2, lifetime: 420}
GRAVE = {rise: 36, open_at: 110, open_time: 36, fade: 50}
CHAIN = {first: 90, cooldown: 600, windup: 24, range: 260, snap: 320, extend: 20, duration: 180, tick: 20, damage: 5, heal: 1.25}
WAIL = {first: 330, cooldown: 720, windup: 60, speed: 3.5, max_radius: 300, damage: 38, slow: 70}
SPECTRAL = {speed: 1.4, cooldowns: 0.8, miss: 0.3, spell_rate: 1.35}
UNDEAD = {"Skeleton", "Skeleton", "Skeleton", "Zombie", "Mummy"}

sprites = {}
sprite = (name) ->
    sprites[name] or= tosprite("spr_bosses.#{name}")
    return sprites[name]

-- Frame k of a spr_bosses effect (they animate at 0.1 frames a step)
draw_frame = (name, xy, k, alpha = 1) ->
    ObjectUtils.screen_draw(sprite(name), xy, alpha, k * 10)

say = (obj, text) ->
    if Map.object_visible(obj)
        EventLog.add(text, PALE)

tell_player = (player, text) ->
    for _ in screens()
        if player\is_local_player()
            EventLog.add(text, PALE)

distance = (a, b) -> math.sqrt((a.x - b.x)^2 + (a.y - b.y)^2)

alive = (obj) -> obj ~= nil and not obj.destroyed and obj.stats.hp > 0 and not obj.is_ghost

-- A black magic hit from her, resisted like other death magic
death_damage = (caster, target, damage) ->
    power = caster\effective_stats().magic
    before = target.stats.hp
    target\damage(damage, power, 1, caster, TypeEffectUtils.get_resistance(target, "Black"))
    return math.max(0, before - target.stats.hp)

O = {} -- her spell objects

-- A grave bursting up out of the ground; after a wind-up it opens and one of the dead climbs out
O.Grave = LuaGameObject.type_create {
    init: (args) =>
        O.Grave.parent_init(@, args.xy, 12, false, 50)
        @caster = args.caster
        @monster = args.monster
        @n_steps = 0
    on_step: () =>
        @n_steps += 1
        if @n_steps == GRAVE.open_at and alive(@caster)
            mon = GameObject.enemy_create {type: @monster, xy: {@x, @y + 8}, team: @caster.team}
            mon.raised_at = GameState.frame
            append @caster.raised, mon
        if @n_steps >= GRAVE.open_at + GRAVE.open_time + GRAVE.fade
            GameObject.destroy(@)
    on_draw: () =>
        if not Map.object_visible(@)
            return
        n = @n_steps
        xy = {@x, @y - 10}
        if n < GRAVE.rise
            draw_frame("ereshkigal_grave", xy, math.floor(n / GRAVE.rise * 6))
        elseif n < GRAVE.open_at
            -- waiting to open: the grave-light pulses, faster as it nears
            draw_frame("ereshkigal_grave", xy, 5)
            t = (n - GRAVE.rise) / (GRAVE.open_at - GRAVE.rise)
            pulse = 0.5 + 0.5 * math.sin(n * (0.15 + 0.25 * t))
            Display.draw_circle(with_alpha(GREEN, (0.08 + 0.17 * t) * pulse), Display.to_screen_xy({@x, @y + 4}), 14 + 4 * t)
        else
            k = math.min(5, math.floor((n - GRAVE.open_at) / GRAVE.open_time * 6))
            fade = n - GRAVE.open_at - GRAVE.open_time
            alpha = if fade > 0 then math.max(0, 1 - fade / GRAVE.fade) else 1
            draw_frame("ereshkigal_grave_open", xy, k, alpha)
}

-- A chain from her to a player, draining their life into her
O.SoulChain = LuaGameObject.type_create {
    init: (args) =>
        O.SoulChain.parent_init(@, args.caster.xy, 8, false, -60)
        @caster = args.caster
        @target = args.target
        @n_steps = 0
        @souls = {} -- how far along the chain each drained soul has come
        @ending = false
    stop: (message) =>
        if not @ending
            @ending = 0
            if message
                tell_player(@target, message)
    on_step: () =>
        @n_steps += 1
        for i = #@souls, 1, -1
            @souls[i] += 0.035
            if @souls[i] >= 1
                table.remove(@souls, i)
        if @ending
            @ending += 1
            if @ending > 20
                GameObject.destroy(@)
            return
        {:caster, :target} = @
        if not alive(caster) or not alive(target)
            @stop()
            return
        @xy = caster.xy
        if distance(caster, target) > CHAIN.snap
            @stop("You tear free of the soul chain!")
            return
        drain_steps = @n_steps - CHAIN.extend
        if drain_steps > CHAIN.duration
            @stop()
        elseif drain_steps > 0 and drain_steps % CHAIN.tick == 0
            drained = death_damage(caster, target, CHAIN.damage)
            caster\heal_hp(drained * CHAIN.heal)
            append @souls, 0
    on_draw: () =>
        {:caster, :target} = @
        if caster.destroyed or target.destroyed
            return
        if not Map.object_visible(caster) and not Map.object_visible(target)
            return
        origin = {caster.x, caster.y - 18}
        dx, dy = target.x - origin[1], target.y - 4 - origin[2]
        len = math.sqrt(dx * dx + dy * dy)
        reach = math.min(1, @n_steps / CHAIN.extend)
        alpha = if @ending then math.max(0, 1 - @ending / 20) else 1
        sag = math.min(22, len * 0.12) * (0.75 + 0.25 * math.sin(@n_steps * 0.07))
        point = (t) -> {origin[1] + dx * t, origin[2] + dy * t + math.sin(t * math.pi) * sag}
        n = math.max(1, math.floor(len * reach / 7))
        pulse = math.floor(@n_steps / 8)
        for i = 0, n
            draw_frame("ereshkigal_chain", point(i / n * reach), (i + pulse) % 2, alpha)
        for s in *@souls
            draw_frame("ereshkigal_soul", point(1 - s), math.floor(@n_steps / 5) % 4, alpha)
}

-- The wail: a ring of wailing spirits sweeping out from her
O.WailRing = LuaGameObject.type_create {
    init: (args) =>
        O.WailRing.parent_init(@, args.caster.xy, 8, false, -60)
        @caster = args.caster
        @radius = 20
        @hit = {}
        @n_steps = 0
    on_step: () =>
        @n_steps += 1
        @radius += WAIL.speed
        if @radius > WAIL.max_radius
            GameObject.destroy(@)
            return
        if not alive(@caster)
            return
        for p in *Map.enemies_list(@caster)
            if alive(p) and not @hit[p.id] and distance(@, p) <= @radius + p.radius
                @hit[p.id] = true
                death_damage(@caster, p, WAIL.damage)
                p\add_effect("Slowed", WAIL.slow)
                tell_player(p, "The Queen's wail chills you to the bone!")
    on_draw: () =>
        if not Map.object_visible(@caster) and not Map.object_visible(@)
            return
        fade = math.max(0, 1 - (@radius / WAIL.max_radius) ^ 2)
        xy = Display.to_screen_xy(@xy)
        Display.draw_circle_outline(with_alpha(GREEN, 0.45 * fade), xy, @radius, 3)
        Display.draw_circle_outline(with_alpha(PALE, 0.3 * fade), xy, @radius - 6, 1)
        n = 14
        for i = 1, n
            a = i / n * 2 * math.pi + @n_steps * 0.012
            draw_frame("ereshkigal_wail", {@x + math.cos(a) * @radius, @y + math.sin(a) * @radius}, (math.floor(@n_steps / 5) + i) % 4, fade)
}

DataW.effect_create {
    name: "Ereshkigal Channeling"
    stat_func: (obj, old, new) =>
        new.speed = 0
}

DataW.effect_create {
    name: "Ereshkigal Spectral"
    effected_colour: {175, 255, 210, 150}
    stat_func: (obj, old, new) =>
        new.speed *= SPECTRAL.speed
        new.cooldown_mult *= SPECTRAL.cooldowns
    on_receive_damage_func: (attacker, defender, damage) =>
        if chance(SPECTRAL.miss)
            return 0
        return damage
}

-- Spells: wind up (her cast clip, standing still), then release
grave_spot = (boss, near, graves) ->
    for _ = 1, 30
        a = random(0, 359) * math.pi / 180
        d = random(40, 88)
        xy = {near.x + math.cos(a) * d, near.y + math.sin(a) * d}
        if Map.radius_tile_check(boss.map, xy, 16) == nil
            clear = true
            for g in *graves
                if distance({x: xy[1], y: xy[2]}, {x: g[1], y: g[2]}) < 40
                    clear = false
            if clear
                return xy
    return nil

SPELLS = {
    raise: {
        windup: RAISE.windup
        start: (boss) => say(boss, "Ereshkigal calls the dead up from their graves!")
        release: (boss, targets) =>
            graves = {}
            count = math.min(RAISE.max_alive - #boss.raised, if boss.spectral then RAISE.spectral_graves else RAISE.graves)
            for i = 1, count
                xy = grave_spot(boss, boss, graves)
                if xy
                    append graves, xy
                    GameObject.add_to_level O.Grave.create {:xy, caster: boss, monster: random_choice(UNDEAD)}
    }
    chain: {
        windup: CHAIN.windup
        start: (boss) => say(boss, "Ereshkigal casts a soul chain!")
        release: (boss, targets) =>
            if alive(boss.chain_target) and distance(boss, boss.chain_target) <= CHAIN.range
                chain = O.SoulChain.create {caster: boss, target: boss.chain_target}
                GameObject.add_to_level(chain)
                append boss.chains, chain
    }
    wail: {
        windup: WAIL.windup
        start: (boss) => say(boss, "Ereshkigal draws in a terrible breath...")
        release: (boss, targets) =>
            say(boss, "Ereshkigal wails!")
            GameObject.add_to_level O.WailRing.create {caster: boss}
    }
}

cast_start = (boss, name) ->
    boss.casting = name
    boss.cast_steps = 0
    boss\add_effect("Ereshkigal Channeling", SPELLS[name].windup + 2)
    SPELLS[name].start(SPELLS[name], boss)

-- The player the chain can reach who is furthest from her (not already chained): the one out of
-- her grasp, who can break it by stepping back out of reach
chain_target = (boss, targets) ->
    choices = {}
    for p in *targets
        taken = false
        for c in *boss.chains
            if c.target == p
                taken = true
        if not taken and distance(boss, p) <= CHAIN.range and Map.line_tile_check(boss.map, boss.xy, p.xy) == nil
            append choices, p
    table.sort(choices, (a, b) -> distance(boss, a) > distance(boss, b))
    return choices[1]

become_spectral = (boss) ->
    boss.spectral = true
    boss\add_effect("Ereshkigal Spectral", 1000000)
    say(boss, "Ereshkigal casts off her flesh and walks as a shade!")
    boss.timers.wail = math.min(boss.timers.wail, 30)

DataW.enemy_create {
    name: "Ereshkigal"
    appear_message: "Ereshkigal, Queen Below, rises to claim the living!"
    defeat_message: "Ereshkigal sinks back into the dark below!"
    sprite: "spr_bosses.ereshkigal"
    radius: 20
    xpaward: 400
    unique: true
    types: {"Black"}
    resistances: {
        Black: 2
        Green: 1.5
        White: -1.25
    }
    weapon: {cooldown: 80, damage: 3, damage_type: {magic: 0.75, physical: 0.25}} -- a death-cold touch
    stats: {
        hp: 520
        hpregen: 0
        movespeed: 2
        strength: 10
        magic: 20
        defence: 14
        willpower: 16
    }
    init_func: () =>
        @raised = {}
        @chains = {}
        @timers = {raise: RAISE.first, chain: CHAIN.first, wail: WAIL.first}
        @casting = false
        @cast_steps = 0
        @spectral = false
    step_func: () =>
        @raised = [m for m in *@raised when not m.destroyed]
        -- the dead she raises crumble back to dust after a while
        for m in *@raised
            if GameState.frame - m.raised_at > RAISE.lifetime
                m\direct_damage(m.stats.hp + 1)
        @chains = [c for c in *@chains when not c.destroyed]
        if not @spectral and @stats.hp < @stats.max_hp / 2
            become_spectral(@)
        targets = [p for p in *Map.enemies_list(@) when alive(p)]
        if #targets == 0
            return
        if @casting
            @cast_steps += 1
            if @cast_steps % 40 == 1
                @mark_animation("cast")
            spell = SPELLS[@casting]
            if @cast_steps >= spell.windup
                @casting = false
                spell.release(spell, @, targets)
            return
        rate = if @spectral then SPECTRAL.spell_rate else 1
        for k, v in pairs @timers
            @timers[k] = v - rate
        nearest = math.huge
        for p in *targets
            nearest = math.min(nearest, distance(@, p))
        if @timers.raise <= 0 and #@raised < RAISE.max_alive
            @timers.raise = RAISE.cooldown
            cast_start(@, "raise")
        elseif @timers.wail <= 0 and nearest < WAIL.max_radius - 40
            @timers.wail = WAIL.cooldown
            cast_start(@, "wail")
        elseif @timers.chain <= 0 and #@chains < (if @spectral then 2 else 1)
            @chain_target = chain_target(@, targets)
            if @chain_target
                @timers.chain = CHAIN.cooldown
                cast_start(@, "chain")
    on_post_draw: () =>
        if Map.object_visible(@)
            if @spectral
                draw_frame("ereshkigal_shimmer", {@x, @y + 4}, math.floor(GameState.frame / 6) % 6, 0.8)
            if @casting == "wail"
                t = @cast_steps / WAIL.windup
                r = 32 + 90 * (1 - t)
                xy = Display.to_screen_xy(@xy)
                Display.draw_circle(with_alpha(GREEN, 0.15 * t), xy, r)
                Display.draw_circle_outline(with_alpha(PALE, 0.2 + 0.5 * t), xy, r, 2)
                for i = 1, 6
                    a = i / 6 * 2 * math.pi - t * 3
                    draw_frame("ereshkigal_wail", {@x + math.cos(a) * r, @y + math.sin(a) * r}, i % 4, 0.3 + 0.6 * t)
        draw_name(@)
    death_func: () =>
        -- her dead go back to rest with her
        for mon in *@raised
            if not mon.destroyed
                mon\direct_damage(mon.stats.hp + 1)
        ItemUtils = require "maps.ItemUtils"
        ObjectUtils.spawn_item_near(@, "Tomb Lanart", 1)
        ObjectUtils.spawn_item_near(@, "Ring of Vampirism", 1)
        {:type, :amount} = ItemUtils.randart_generate(2)
        ObjectUtils.spawn_item_near(@, type, amount)
}
