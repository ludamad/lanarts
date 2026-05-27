local Display = require "core.Display"
local GameObject = require "core.GameObject"
local Map = require "core.Map"

local M = {}

local DIRECTOR_DEPTH = -200
local HAZARD_DEPTH = 90

local function sqr(value)
    return value * value
end

local function dist_sqr(a, b)
    return sqr(a[1] - b[1]) + sqr(a[2] - b[2])
end

local function xy_of(obj)
    return {obj.x, obj.y}
end

local function copy_xy(xy)
    return {xy[1], xy[2]}
end

local function living_players()
    local players = {}
    for _, player in ipairs(Map.players_list()) do
        if player and not player.destroyed and player.is_enemy == false and not player.is_ghost then
            players[#players + 1] = player
        end
    end
    return players
end

local function living_monsters(include_summons)
    local monsters = {}
    for _, monster in ipairs(Map.monsters_list()) do
        if monster and not monster.destroyed and monster.is_enemy then
            if include_summons or not monster.content_lab_summon then
                monsters[#monsters + 1] = monster
            end
        end
    end
    return monsters
end

local function primary_monster(primary_name)
    local fallback
    for _, monster in ipairs(living_monsters(false)) do
        fallback = fallback or monster
        if primary_name and monster.name == primary_name then
            return monster
        end
    end
    return fallback
end

local function center_of_players()
    local players = living_players()
    if #players == 0 then
        return nil
    end
    local x, y = 0, 0
    for _, player in ipairs(players) do
        x = x + player.x
        y = y + player.y
    end
    return {x / #players, y / #players}
end

local function step_cooldowns(cooldowns)
    for id, cooldown in pairs(cooldowns) do
        if cooldown <= 1 then
            cooldowns[id] = nil
        else
            cooldowns[id] = cooldown - 1
        end
    end
end

local function damage_player(hazard, player)
    local source = hazard.source
    if source and source.destroyed then
        source = nil
    end
    if source then
        player:damage(hazard.damage, hazard.power, hazard.magic_percentage, source)
    else
        player:damage(hazard.damage, hazard.power, hazard.magic_percentage)
    end
end

local function damage_overlapping_players(hazard, xy, radius)
    step_cooldowns(hazard.hit_cooldowns)
    for _, player in ipairs(living_players()) do
        local player_radius = player.target_radius or player.radius or 12
        if dist_sqr(xy, xy_of(player)) <= sqr(radius + player_radius) and not hazard.hit_cooldowns[player.id] then
            hazard.hit_cooldowns[player.id] = hazard.redamage_cooldown
            damage_player(hazard, player)
        end
    end
end

local function draw_world_circle(color, xy, radius, fill_alpha, outline_alpha)
    local screen_xy = Display.to_screen_xy(xy)
    Display.draw_circle(with_alpha(color, fill_alpha), screen_xy, radius)
    Display.draw_circle_outline(with_alpha(color, outline_alpha), screen_xy, radius, 2)
end

function M.create_pulse(args)
    local windup = args.windup or 45
    local active = args.active or 24
    local linger = args.linger or 16
    local radius = args.radius or 70
    return GameObject.object_create {
        map = args.map,
        xy = copy_xy(args.xy),
        radius = 1,
        solid = false,
        depth = HAZARD_DEPTH,
        elapsed = 0,
        source = args.source,
        damage = args.damage or 4,
        power = args.power or 22,
        magic_percentage = args.magic_percentage or 1,
        redamage_cooldown = args.redamage_cooldown or 18,
        hit_cooldowns = {},
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if self.elapsed > windup and self.elapsed <= windup + active then
                damage_overlapping_players(self, self.xy, radius)
            elseif self.elapsed > windup + active + linger then
                GameObject.destroy(self)
            end
        end,
        on_draw = function(self)
            if self.elapsed <= windup then
                local progress = math.max(0, math.min(1, self.elapsed / windup))
                draw_world_circle(COL_PALE_RED, self.xy, radius * progress, 0.10 + progress * 0.12, 0.35 + progress * 0.35)
            elseif self.elapsed <= windup + active then
                draw_world_circle(COL_RED, self.xy, radius, 0.22, 0.75)
            else
                local fade = 1 - (self.elapsed - windup - active) / linger
                draw_world_circle(COL_PALE_RED, self.xy, radius, 0.08 * fade, 0.25 * fade)
            end
        end,
    }
end

function M.create_orb(args)
    return GameObject.object_create {
        map = args.map,
        xy = copy_xy(args.center_xy),
        radius = args.hit_radius or 18,
        solid = false,
        depth = HAZARD_DEPTH,
        elapsed = 0,
        source = args.source,
        center_xy = copy_xy(args.center_xy),
        anchor = args.anchor,
        orbit_radius = args.orbit_radius or 120,
        angle = args.angle or 0,
        angular_velocity = args.angular_velocity or 0.045,
        duration = args.duration or 300,
        hit_radius = args.hit_radius or 18,
        damage = args.damage or 3,
        power = args.power or 20,
        magic_percentage = args.magic_percentage or 1,
        redamage_cooldown = args.redamage_cooldown or 20,
        hit_cooldowns = {},
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            self.angle = self.angle + self.angular_velocity
            local anchor = self.anchor
            local center = self.center_xy
            if anchor and not anchor.destroyed then
                center = xy_of(anchor)
            end
            self.xy = {
                center[1] + math.cos(self.angle) * self.orbit_radius,
                center[2] + math.sin(self.angle) * self.orbit_radius,
            }
            damage_overlapping_players(self, self.xy, self.hit_radius)
            if self.elapsed >= self.duration then
                GameObject.destroy(self)
            end
        end,
        on_draw = function(self)
            local fade_in = math.min(1, self.elapsed / 20)
            local fade_out = math.min(1, (self.duration - self.elapsed) / 30)
            local alpha = math.max(0, math.min(fade_in, fade_out))
            draw_world_circle(COL_PALE_BLUE, self.xy, self.hit_radius, 0.20 * alpha, 0.78 * alpha)
        end,
    }
end

local function spawn_orb_wave(director, package)
    local orbit = package.orbit or {}
    local anchor = primary_monster(package.primary)
    local center = anchor and xy_of(anchor) or center_of_players() or director.xy
    local count = orbit.count or 4
    for idx = 1, count do
        M.create_orb {
            map = director.map,
            source = anchor,
            anchor = anchor,
            center_xy = center,
            angle = (idx - 1) * (math.pi * 2 / count) + (director.elapsed or 0) * 0.01,
            angular_velocity = orbit.speed or 0.042,
            orbit_radius = orbit.radius or 118,
            hit_radius = orbit.hit_radius or 18,
            duration = orbit.duration or 280,
            damage = orbit.damage or 3,
            power = orbit.power or 24,
            redamage_cooldown = orbit.redamage_cooldown or 20,
        }
    end
end

local function spawn_player_pulses(director, package)
    local pulse = package.pulse or {}
    local source = primary_monster(package.primary)
    for _, player in ipairs(living_players()) do
        M.create_pulse {
            map = director.map,
            source = source,
            xy = xy_of(player),
            windup = pulse.windup or 46,
            active = pulse.active or 24,
            linger = pulse.linger or 16,
            radius = pulse.radius or 66,
            damage = pulse.damage or 4,
            power = pulse.power or 28,
            redamage_cooldown = pulse.redamage_cooldown or 18,
        }
    end
end

local function summon_count()
    local count = 0
    for _, monster in ipairs(living_monsters(true)) do
        if monster.content_lab_summon then
            count = count + 1
        end
    end
    return count
end

local function spawn_summons(director, package)
    local summon = package.summon or {}
    if summon_count() >= (summon.max_alive or 4) then
        return
    end
    local anchor = primary_monster(package.primary)
    local center = anchor and xy_of(anchor) or center_of_players() or director.xy
    local monsters = summon.monsters or {}
    if #monsters == 0 then
        return
    end
    local count = summon.count or 1
    local radius = summon.radius or 150
    for idx = 1, count do
        if summon_count() >= (summon.max_alive or 4) then
            return
        end
        local angle = (idx - 1) * (math.pi * 2 / count) + director.elapsed * 0.03
        local xy = {
            center[1] + math.cos(angle) * radius,
            center[2] + math.sin(angle) * radius,
        }
        GameObject.enemy_create {
            map = director.map,
            type = monsters[((idx - 1) % #monsters) + 1],
            xy = xy,
            content_lab_summon = true,
        }
    end
end

function M.add_director(game_map, package)
    package = package or {}
    if not (package.orbit or package.pulse or package.summon) then
        return nil
    end
    local center = center_of_players() or {320, 320}
    return GameObject.object_create {
        map = game_map,
        xy = center,
        radius = 1,
        solid = false,
        depth = DIRECTOR_DEPTH,
        elapsed = 0,
        package = package,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if #living_monsters(false) == 0 then
                GameObject.destroy(self)
                return
            end
            local orbit = self.package.orbit
            if orbit and self.elapsed % (orbit.interval or 300) == (orbit.offset or 40) then
                spawn_orb_wave(self, self.package)
            end
            local pulse = self.package.pulse
            if pulse and self.elapsed % (pulse.interval or 150) == (pulse.offset or 70) then
                spawn_player_pulses(self, self.package)
            end
            local summon = self.package.summon
            if summon and self.elapsed % (summon.interval or 360) == (summon.offset or 120) then
                spawn_summons(self, self.package)
            end
        end,
    }
end

return M
