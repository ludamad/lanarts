local Display = require "core.Display"
local GameObject = require "core.GameObject"
local Map = require "core.Map"

local M = {}

local DIRECTOR_DEPTH = -200
local HAZARD_DEPTH = 90
local PLAYER_TEAM = 0
local HUD_X = 10
local HUD_Y = 58
local HUD_W = 190
local HUD_ROW_H = 18

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

local function normalize_intensity(intensity)
    intensity = intensity or {}
    return {
        id = intensity.id or "standard",
        label = intensity.label or "Standard",
        tempo = math.max(0.25, tonumber(intensity.tempo) or 1),
        damage = math.max(0.1, tonumber(intensity.damage) or 1),
        summons = math.max(0.1, tonumber(intensity.summons) or 1),
    }
end

local function scale_frames(value, fallback, intensity)
    local frames = tonumber(value or fallback) or fallback
    return math.max(1, math.floor(frames / intensity.tempo + 0.5))
end

local function scale_offset(value, fallback, interval, intensity)
    local raw_offset = tonumber(value or fallback) or fallback
    local offset = math.floor(raw_offset / intensity.tempo + 0.5)
    return math.max(0, math.min(interval - 1, offset))
end

local function scale_damage(value, fallback, intensity)
    return (tonumber(value or fallback) or fallback) * intensity.damage
end

local function scale_count(value, fallback, intensity)
    return math.max(1, math.ceil((tonumber(value or fallback) or fallback) * intensity.summons))
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
        if monster and not monster.destroyed and monster.is_enemy and monster.team ~= PLAYER_TEAM and not monster.content_lab_ally then
            if include_summons or not monster.content_lab_summon then
                monsters[#monsters + 1] = monster
            end
        end
    end
    return monsters
end

local function living_escorts()
    local escorts = {}
    for _, monster in ipairs(Map.monsters_list()) do
        if monster and not monster.destroyed and monster.content_lab_ally then
            escorts[#escorts + 1] = monster
        end
    end
    return escorts
end

local function living_player_team_targets(include_escorts)
    local targets = living_players()
    if include_escorts then
        for _, escort in ipairs(living_escorts()) do
            targets[#targets + 1] = escort
        end
    end
    return targets
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
    for _, player in ipairs(living_player_team_targets(hazard.affects_escorts)) do
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

local function next_delta(elapsed, interval, offset)
    local delta = offset - (elapsed % interval)
    if delta < 0 then
        delta = delta + interval
    end
    return delta
end

local function format_frames(frames)
    if frames <= 0 then
        return "now"
    end
    return tostring(frames) .. "f"
end

local function draw_text(color, origin, xy, text)
    Fonts.small:draw({color = color, origin = origin}, xy, text)
end

local function draw_hud_bar(x, y, w, h, pct, color)
    pct = math.max(0, math.min(1, pct or 0))
    local fill_w = math.floor(w * pct + 0.5)
    Display.draw_rectangle(with_alpha(COL_BLACK, 0.46), {x, y, x + w, y + h})
    if fill_w > 0 then
        Display.draw_rectangle(with_alpha(color, 0.52), {x, y, x + fill_w, y + h})
    end
    Display.draw_rectangle_outline(with_alpha(color, 0.62), {x, y, x + w, y + h}, 1)
end

local function draw_hud_row(y, label, value, color, pct)
    local text_y = y + 2
    draw_text(color, Display.LEFT_TOP, {HUD_X + 8, text_y}, label)
    draw_text(COL_WHITE, Display.RIGHT_TOP, {HUD_X + HUD_W - 8, text_y}, value)
    draw_hud_bar(HUD_X + 70, y + 14, HUD_W - 82, 3, pct, color)
end

local function scaled_interval_and_offset(entry, fallback_interval, fallback_offset, intensity)
    local interval = scale_frames(entry.interval, fallback_interval, intensity)
    local offset = scale_offset(entry.offset, fallback_offset, interval, intensity)
    return interval, offset
end

local function escort_health()
    local hp, max_hp = 0, 0
    local count = 0
    for _, escort in ipairs(living_escorts()) do
        count = count + 1
        hp = hp + math.max(0, escort.stats.hp or 0)
        local estats = escort:effective_stats()
        max_hp = max_hp + math.max(1, escort.content_lab_max_hp or estats.max_hp or escort.stats.hp or 1)
    end
    return count, hp, max_hp
end

local function draw_escort_markers()
    for _, escort in ipairs(living_escorts()) do
        local estats = escort:effective_stats()
        local max_hp = math.max(1, escort.content_lab_max_hp or estats.max_hp or escort.stats.hp or 1)
        local hp = math.max(0, escort.stats.hp or 0)
        local pct = math.max(0, math.min(1, hp / max_hp))
        local xy = Display.to_screen_xy({escort.x, escort.y - (escort.radius or 14) - 13})
        local label = string.format("ALLY %d/%d", math.ceil(hp), math.ceil(max_hp))
        local color = pct <= 0.35 and COL_PALE_RED or COL_PALE_GREEN
        draw_world_circle(color, xy_of(escort), (escort.radius or 14) + 5, 0.02, 0.52)
        draw_text(color, Display.CENTER, xy, label)
    end
end

local summon_count

local function draw_package_hud(director)
    local package = director.package or {}
    local intensity = director.intensity
    local row_count = 1
    if package.orbit then
        row_count = row_count + 1
    end
    if package.pulse then
        row_count = row_count + 1
    end
    if package.summon then
        row_count = row_count + 1
    end
    if package.escort then
        row_count = row_count + 1
    end

    local panel_h = 12 + row_count * HUD_ROW_H
    Display.draw_rectangle(with_alpha(COL_BLACK, 0.56), {HUD_X, HUD_Y, HUD_X + HUD_W, HUD_Y + panel_h})
    Display.draw_rectangle_outline(with_alpha(COL_PALE_BLUE, 0.34), {HUD_X, HUD_Y, HUD_X + HUD_W, HUD_Y + panel_h}, 1)
    local title = "Package " .. (intensity.label or intensity.id or "Standard")
    draw_text(COL_PALE_BLUE, Display.LEFT_TOP, {HUD_X + 8, HUD_Y + 5}, title)

    local y = HUD_Y + 24
    if package.orbit then
        local orbit = package.orbit
        local interval, offset = scaled_interval_and_offset(orbit, 300, 40, intensity)
        local delta = next_delta(director.elapsed, interval, offset)
        draw_hud_row(y, "ORBS", format_frames(delta), COL_PALE_BLUE, 1 - delta / interval)
        y = y + HUD_ROW_H
    end
    if package.pulse then
        local pulse = package.pulse
        local interval, offset = scaled_interval_and_offset(pulse, 150, 70, intensity)
        local delta = next_delta(director.elapsed, interval, offset)
        draw_hud_row(y, "AOE", format_frames(delta), COL_PALE_RED, 1 - delta / interval)
        y = y + HUD_ROW_H
    end
    if package.summon then
        local summon = package.summon
        local interval, offset = scaled_interval_and_offset(summon, 360, 120, intensity)
        local delta = next_delta(director.elapsed, interval, offset)
        local current = summon_count()
        local max_alive = scale_count(summon.max_alive, 4, intensity)
        draw_hud_row(y, "ADDS", format_frames(delta) .. " " .. current .. "/" .. max_alive, COL_PALE_YELLOW, 1 - delta / interval)
        y = y + HUD_ROW_H
    end
    if package.escort then
        local count, hp, max_hp = escort_health()
        local pct = max_hp > 0 and hp / max_hp or 0
        local value = count > 0 and string.format("%d/%d", math.ceil(hp), math.ceil(max_hp)) or "down"
        draw_hud_row(y, "ALLY", value, pct <= 0.35 and COL_PALE_RED or COL_PALE_GREEN, pct)
    end

    draw_escort_markers()
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
        affects_escorts = args.affects_escorts,
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
        affects_escorts = args.affects_escorts,
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
    local intensity = director.intensity
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
            duration = scale_frames(orbit.duration, 280, intensity),
            damage = scale_damage(orbit.damage, 3, intensity),
            power = scale_damage(orbit.power, 24, intensity),
            affects_escorts = orbit.affects_escorts,
            redamage_cooldown = orbit.redamage_cooldown or 20,
        }
    end
end

local function spawn_player_pulses(director, package)
    local pulse = package.pulse or {}
    local intensity = director.intensity
    local source = primary_monster(package.primary)
    for _, player in ipairs(living_players()) do
        M.create_pulse {
            map = director.map,
            source = source,
            xy = xy_of(player),
            windup = scale_frames(pulse.windup, 46, intensity),
            active = scale_frames(pulse.active, 24, intensity),
            linger = pulse.linger or 16,
            radius = pulse.radius or 66,
            damage = scale_damage(pulse.damage, 4, intensity),
            power = scale_damage(pulse.power, 28, intensity),
            affects_escorts = pulse.affects_escorts,
            redamage_cooldown = pulse.redamage_cooldown or 18,
        }
    end
end

local function spawn_escorts(director, package)
    local escort = package.escort or {}
    if not escort.monster then
        return
    end
    local center = center_of_players() or director.xy
    local count = escort.count or 1
    local radius = escort.radius or 120
    local hp = escort.hp
    for idx = 1, count do
        local angle = escort.angle or ((idx - 1) * (math.pi * 2 / count))
        local xy = {
            center[1] + math.cos(angle) * radius,
            center[2] + math.sin(angle) * radius,
        }
        local ally = GameObject.enemy_create {
            map = director.map,
            type = escort.monster,
            xy = xy,
            team = PLAYER_TEAM,
            content_lab_ally = true,
        }
        ally.content_lab_ally = true
        ally.xpworth = 0
        if hp then
            ally.stats.hp = hp
            ally.content_lab_max_hp = hp
        end
    end
end

function summon_count()
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
    local intensity = director.intensity
    local max_alive = scale_count(summon.max_alive, 4, intensity)
    if summon_count() >= max_alive then
        return
    end
    local anchor = primary_monster(package.primary)
    local center = anchor and xy_of(anchor) or center_of_players() or director.xy
    local monsters = summon.monsters or {}
    if #monsters == 0 then
        return
    end
    local count = scale_count(summon.count, 1, intensity)
    local radius = summon.radius or 150
    for idx = 1, count do
        if summon_count() >= max_alive then
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

function M.add_director(game_map, package, intensity)
    package = package or {}
    if not (package.orbit or package.pulse or package.summon or package.escort) then
        return nil
    end
    intensity = normalize_intensity(intensity)
    local center = center_of_players() or {320, 320}
    local director = GameObject.object_create {
        map = game_map,
        xy = center,
        radius = 1,
        solid = false,
        depth = DIRECTOR_DEPTH,
        elapsed = 0,
        package = package,
        intensity = intensity,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if #living_monsters(false) == 0 then
                GameObject.destroy(self)
                return
            end
            local orbit = self.package.orbit
            if orbit then
                local interval = scale_frames(orbit.interval, 300, self.intensity)
                local offset = scale_offset(orbit.offset, 40, interval, self.intensity)
                if self.elapsed % interval == offset then
                    spawn_orb_wave(self, self.package)
                end
            end
            local pulse = self.package.pulse
            if pulse then
                local interval = scale_frames(pulse.interval, 150, self.intensity)
                local offset = scale_offset(pulse.offset, 70, interval, self.intensity)
                if self.elapsed % interval == offset then
                    spawn_player_pulses(self, self.package)
                end
            end
            local summon = self.package.summon
            if summon then
                local interval = scale_frames(summon.interval, 360, self.intensity)
                local offset = scale_offset(summon.offset, 120, interval, self.intensity)
                if self.elapsed % interval == offset then
                    spawn_summons(self, self.package)
                end
            end
        end,
        on_draw = function(self)
            draw_package_hud(self)
        end,
    }
    spawn_escorts(director, package)
    return director
end

return M
