local Display = require "core.Display"
local EventLog = require "ui.EventLog"
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
local RESULT_DEPTH = DIRECTOR_DEPTH - 1

local function sqr(value)
    return value * value
end

local function dist_sqr(a, b)
    return sqr(a[1] - b[1]) + sqr(a[2] - b[2])
end

local function dist_to_segment_sqr(point, a, b)
    local vx = b[1] - a[1]
    local vy = b[2] - a[2]
    local len_sqr = vx * vx + vy * vy
    if len_sqr <= 1 then
        return dist_sqr(point, a)
    end
    local t = ((point[1] - a[1]) * vx + (point[2] - a[2]) * vy) / len_sqr
    t = math.max(0, math.min(1, t))
    return dist_sqr(point, {a[1] + vx * t, a[2] + vy * t})
end

local function segment_point_at(a, b, pct)
    pct = math.max(0, math.min(1, pct or 0))
    return {
        a[1] + (b[1] - a[1]) * pct,
        a[2] + (b[2] - a[2]) * pct,
    }
end

local function nearest_segment_point(point, a, b)
    local vx = b[1] - a[1]
    local vy = b[2] - a[2]
    local len_sqr = vx * vx + vy * vy
    if len_sqr <= 1 then
        return {a[1], a[2]}, 0, dist_sqr(point, a)
    end
    local pct = ((point[1] - a[1]) * vx + (point[2] - a[2]) * vy) / len_sqr
    pct = math.max(0, math.min(1, pct))
    local xy = segment_point_at(a, b, pct)
    return xy, pct, dist_sqr(point, xy)
end

local function xy_of(obj)
    return {obj.x, obj.y}
end

local function copy_xy(xy)
    return {xy[1], xy[2]}
end

local function round_xy(xy)
    return {math.floor(xy[1] + 0.5), math.floor(xy[2] + 0.5)}
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

local function force_chase(source, target, frames)
    if not source or source.destroyed or not target or target.destroyed or not source.force_chase then
        return false
    end
    source:force_chase(target, frames)
    return true
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

local function draw_world_link(color, from_xy, to_xy, radius, alpha)
    local dx = to_xy[1] - from_xy[1]
    local dy = to_xy[2] - from_xy[2]
    local distance = math.sqrt(dx * dx + dy * dy)
    local steps = math.max(2, math.floor(distance / 18))
    for idx = 0, steps do
        if idx % 2 == 0 then
            local pct = idx / steps
            local xy = {
                from_xy[1] + dx * pct,
                from_xy[2] + dy * pct,
            }
            Display.draw_circle(with_alpha(color, alpha), Display.to_screen_xy(xy), radius)
        end
    end
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

local function safe_rate(num, denom)
    denom = tonumber(denom) or 0
    if denom <= 0 then
        return 0
    end
    return (tonumber(num) or 0) / denom
end

local function format_pct(value)
    return tostring(math.floor((value or 0) * 100 + 0.5)) .. "%"
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

local function package_title(package, intensity)
    return (package.primary or "Package") .. " " .. (intensity.label or intensity.id or "Standard")
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

local function primary_health(package)
    local primary = primary_monster(package and package.primary)
    if not primary then
        return 0, 0, 0, package and package.primary or ""
    end
    local hp = math.max(0, primary.stats.hp or 0)
    local estats = primary:effective_stats()
    local max_hp = math.max(1, estats.max_hp or primary.stats.max_hp or primary.stats.hp or 1)
    return 1, hp, max_hp, primary.name or package.primary or ""
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

local function aggro_role(target)
    if not target or target.destroyed then
        return "none"
    end
    if target.content_lab_ally then
        return "ally"
    end
    if target.is_enemy == false then
        return "player"
    end
    if target.team == PLAYER_TEAM then
        return "ally"
    end
    return "other"
end

local function target_name(target)
    if not target or target.destroyed then
        return ""
    end
    return target.name or target.class_name or "target"
end

local function aggro_state(package)
    local primary = primary_monster(package and package.primary)
    if not primary then
        return {
            role = "none",
            target = nil,
            target_name = "",
            primary = nil,
        }
    end
    local target = primary.target
    if target and target.destroyed then
        target = nil
    end
    return {
        role = aggro_role(target),
        target = target,
        target_name = target_name(target),
        primary = primary,
    }
end

local function aggro_color(role)
    if role == "player" then
        return COL_PALE_GREEN
    elseif role == "ally" then
        return COL_PALE_RED
    elseif role == "other" then
        return COL_PALE_YELLOW
    end
    return COL_LIGHT_GRAY
end

local function aggro_value(state, director)
    if (director.rescue_pull_active_frames or 0) > 0 then
        return "taunt " .. (director.rescue_pull_target or state.target_name or "target")
    end
    if state.role == "none" then
        return "none"
    end
    if state.target_name and state.target_name ~= "" then
        return state.role .. " " .. state.target_name
    end
    return state.role
end

local function draw_aggro_marker(director, package)
    local state = aggro_state(package)
    if not state.primary or not state.target then
        return
    end
    local color = aggro_color(state.role)
    local primary_xy = xy_of(state.primary)
    local target_xy = xy_of(state.target)
    local alpha = state.role == "ally" and 0.62 or 0.42
    draw_world_link(color, primary_xy, target_xy, state.role == "ally" and 4 or 3, alpha)
    draw_world_circle(color, primary_xy, (state.primary.radius or 16) + 6, 0.02, alpha)
    draw_world_circle(color, target_xy, (state.target.radius or 12) + 5, 0.02, alpha)
    local label_xy = Display.to_screen_xy({primary_xy[1], primary_xy[2] - (state.primary.radius or 16) - 28})
    local label = (director.rescue_pull_active_frames or 0) > 0 and "TAUNTED"
        or state.role == "ally" and "ON ALLY"
        or state.role == "player" and "ON PLAYER"
        or "AGGRO"
    draw_text(color, Display.CENTER, label_xy, label)
end

local summon_count
local active_summon_portals
local active_orbs
local active_pulses
local active_rescue_strikes
local rescue_window_state
local next_threat_state

local function draw_package_hud(director)
    local package = director.package or {}
    local intensity = director.intensity
    local row_count = 1
    local has_threat_rows = package.orbit or package.pulse or package.summon or package.rescue
    if has_threat_rows then
        row_count = row_count + 1
    end
    if package.orbit then
        row_count = row_count + 1
    end
    if package.pulse then
        row_count = row_count + 1
    end
    if package.summon then
        row_count = row_count + 1
    end
    if package.rescue then
        row_count = row_count + 3
    end
    if package.escort then
        row_count = row_count + 1
    end
    if package.primary then
        row_count = row_count + 2
    end

    local panel_h = 12 + row_count * HUD_ROW_H
    Display.draw_rectangle(with_alpha(COL_BLACK, 0.56), {HUD_X, HUD_Y, HUD_X + HUD_W, HUD_Y + panel_h})
    Display.draw_rectangle_outline(with_alpha(COL_PALE_BLUE, 0.34), {HUD_X, HUD_Y, HUD_X + HUD_W, HUD_Y + panel_h}, 1)
    local title = "Package " .. (intensity.label or intensity.id or "Standard")
    draw_text(COL_PALE_BLUE, Display.LEFT_TOP, {HUD_X + 8, HUD_Y + 5}, title)

    local y = HUD_Y + 24
    if has_threat_rows then
        local threat = next_threat_state(director, package)
        local value = threat.label
            and ((threat.state and threat.state ~= "next" and (threat.label .. " " .. threat.state .. " ") or (threat.label .. " ")) .. format_frames(threat.frames or 0))
            or "none"
        draw_hud_row(y, "NEXT", value, threat.color or COL_LIGHT_GRAY, threat.pct or 0)
        y = y + HUD_ROW_H
    end
    if package.orbit then
        local orbit = package.orbit
        local interval, offset = scaled_interval_and_offset(orbit, 300, 40, intensity)
        local delta = next_delta(director.elapsed, interval, offset)
        local active, remaining = active_orbs()
        if active > 0 then
            draw_hud_row(y, "ORBS", "active " .. active .. " " .. format_frames(remaining or 0), COL_PALE_BLUE, 1)
        else
            draw_hud_row(y, "ORBS", format_frames(delta), COL_PALE_BLUE, 1 - delta / interval)
        end
        y = y + HUD_ROW_H
    end
    if package.pulse then
        local pulse = package.pulse
        local interval, offset = scaled_interval_and_offset(pulse, 150, 70, intensity)
        local delta = next_delta(director.elapsed, interval, offset)
        local warning, active, warning_remaining, active_remaining = active_pulses()
        if active > 0 then
            draw_hud_row(y, "AOE", "active " .. format_frames(active_remaining or 0), COL_PALE_RED, 1)
        elseif warning > 0 then
            draw_hud_row(y, "AOE", "warn " .. format_frames(warning_remaining or 0), COL_PALE_YELLOW, 1)
        else
            draw_hud_row(y, "AOE", format_frames(delta), COL_PALE_RED, 1 - delta / interval)
        end
        y = y + HUD_ROW_H
    end
    if package.summon then
        local summon = package.summon
        local interval, offset = scaled_interval_and_offset(summon, 360, 120, intensity)
        local delta = next_delta(director.elapsed, interval, offset)
        local current = summon_count()
        local warning_count, warning_remaining = active_summon_portals()
        local max_alive = scale_count(summon.max_alive, 4, intensity)
        if warning_count > 0 then
            draw_hud_row(y, "ADDS", "warn " .. format_frames(warning_remaining or 0) .. " " .. current .. "/" .. max_alive, COL_PALE_YELLOW, 1)
        else
            draw_hud_row(y, "ADDS", format_frames(delta) .. " " .. current .. "/" .. max_alive, COL_PALE_YELLOW, 1 - delta / interval)
        end
        y = y + HUD_ROW_H
    end
    if package.rescue then
        local rescue = package.rescue
        local state = rescue_window_state(director, rescue)
        local delta = state.next_frames
        local value
        local color = delta <= 60 and COL_PALE_YELLOW or COL_PALE_GREEN
        local pct = 1 - delta / math.max(1, state.interval)
        if state.active_count > 0 then
            local prefix = state.late and "clutch " or (state.blocked and "hold " or "block ")
            value = prefix .. format_frames(state.remaining or 0)
            color = state.late and COL_PALE_YELLOW or COL_PALE_GREEN
            pct = 1
        elseif (director.rescue_pull_active_frames or 0) > 0 then
            value = "pull " .. format_frames(director.rescue_pull_active_frames)
            color = COL_PALE_YELLOW
            pct = 1
        else
            value = "next " .. format_frames(delta)
        end
        draw_hud_row(y, "SAVE", value, color, pct)
        y = y + HUD_ROW_H

        local read_value
        local read_color = COL_LIGHT_GRAY
        local read_pct = 0
        if (director.rescue_pull_active_frames or 0) > 0 then
            read_value = "taunt " .. (director.rescue_pull_target or "target")
            read_color = COL_PALE_YELLOW
            read_pct = 1
        elseif state.active_count > 0 then
            if state.blocked then
                read_value = state.late and "hold clutch" or "hold line"
                read_color = COL_PALE_GREEN
                read_pct = 1
            elseif state.late then
                read_value = "block now"
                read_color = COL_PALE_YELLOW
                read_pct = 1
            else
                read_value = "clutch in " .. format_frames(math.max(0, (state.remaining or 0) - (state.late_window or 0)))
                read_color = COL_LIGHT_GRAY
                read_pct = 1 - math.max(0, ((state.remaining or 0) - (state.late_window or 0))) / math.max(1, state.windup or 1)
            end
        else
            read_value = "next line"
            read_color = delta <= 60 and COL_PALE_YELLOW or COL_LIGHT_GRAY
            read_pct = pct
        end
        draw_hud_row(y, "READ", read_value, read_color, read_pct)
        y = y + HUD_ROW_H

        local saves = director.rescue_saves or 0
        local clutch_saves = director.rescue_clutch_saves or 0
        local pulls = director.rescue_pulls or 0
        local margin = director.rescue_last_margin
        local attempts = saves + (director.rescue_misses or 0)
        local stats_value = saves .. "/" .. attempts .. " c" .. clutch_saves .. " p" .. pulls .. (margin and (" " .. margin .. "f") or "")
        draw_hud_row(y, "RATE", stats_value, COL_PALE_GREEN, safe_rate(saves, math.max(1, attempts)))
        y = y + HUD_ROW_H
    end
    if package.escort then
        local count, hp, max_hp = escort_health()
        local pct = max_hp > 0 and hp / max_hp or 0
        local value = count > 0 and string.format("%d/%d", math.ceil(hp), math.ceil(max_hp)) or "down"
        draw_hud_row(y, "ALLY", value, pct <= 0.35 and COL_PALE_RED or COL_PALE_GREEN, pct)
        y = y + HUD_ROW_H
    end
    if package.primary then
        local count, hp, max_hp = primary_health(package)
        local pct = max_hp > 0 and hp / max_hp or 0
        local value = count > 0 and string.format("%d/%d", math.ceil(hp), math.ceil(max_hp)) or "down"
        draw_hud_row(y, "TARGET", value, pct <= 0.25 and COL_PALE_RED or COL_PALE_BLUE, pct)
        y = y + HUD_ROW_H

        local state = aggro_state(package)
        local color = aggro_color(state.role)
        local value_pct = state.role == "player" and 1 or state.role == "ally" and 1 or 0
        draw_hud_row(y, "AGGRO", aggro_value(state, director), color, value_pct)
    end

    draw_escort_markers()
    draw_aggro_marker(director, package)
end

local function result_color(outcome)
    if outcome == "clear" then
        return COL_PALE_GREEN
    elseif outcome == "ally down" then
        return COL_PALE_YELLOW
    end
    return COL_PALE_RED
end

local function result_label(outcome)
    if outcome == "clear" then
        return "PACKAGE CLEAR"
    elseif outcome == "ally down" then
        return "ALLY DOWN"
    end
    return "PACKAGE FAILED"
end

local function package_result(director, outcome)
    local count, hp, max_hp = escort_health()
    local primary_count, primary_hp, primary_max_hp, primary_name = primary_health(director.package or {})
    local frames = director.elapsed or 0
    local seconds = frames / 60
    local saves = director.rescue_saves or 0
    local clutch = director.rescue_clutch_saves or 0
    local misses = director.rescue_misses or 0
    local pulls = director.rescue_pulls or 0
    local opportunities = saves + misses
    local aggro = aggro_state(director.package or {})
    local save_rate = safe_rate(saves, opportunities)
    local clutch_rate = safe_rate(clutch, saves)
    local pull_rate = safe_rate(pulls, saves)
    local best = director.rescue_best_margin
    local escort_text = count > 0 and string.format("ally %d/%d", math.ceil(hp), math.ceil(max_hp)) or "ally down"
    local rescue_text = (saves > 0 or misses > 0)
        and string.format("saves %d/%d (%s), clutch %d (%s), pulls %d%s", saves, opportunities, format_pct(save_rate), clutch, format_pct(clutch_rate), pulls, best and (", best " .. best .. "f") or "")
        or "no rescue strikes"
    return {
        outcome = outcome,
        title = result_label(outcome),
        subtitle = package_title(director.package or {}, director.intensity or {}),
        detail = string.format("%.1fs, %s, %s", seconds, escort_text, rescue_text),
        color = result_color(outcome),
        frames = frames,
        seconds = seconds,
        escort_count = count,
        escort_hp = hp,
        escort_max_hp = max_hp,
        primary_name = primary_name,
        primary_count = primary_count,
        primary_hp = primary_hp,
        primary_max_hp = primary_max_hp,
        aggro_target_role = aggro.role,
        aggro_target_name = aggro.target_name,
        rescue_saves = saves,
        rescue_opportunities = opportunities,
        rescue_save_rate = save_rate,
        rescue_clutch_saves = clutch,
        rescue_clutch_rate = clutch_rate,
        rescue_pulls = pulls,
        rescue_pull_rate = pull_rate,
        rescue_misses = misses,
        rescue_best_margin = best,
        rescue_last_margin = director.rescue_last_margin,
        rescue_last_pull_frames = director.rescue_last_pull_frames,
    }
end

local function package_state_from_director(director)
    if not director or director.destroyed then
        return nil
    end
    local count, hp, max_hp = escort_health()
    local primary_count, primary_hp, primary_max_hp, primary_name = primary_health(director.package or {})
    local saves = director.rescue_saves or 0
    local misses = director.rescue_misses or 0
    local clutch = director.rescue_clutch_saves or 0
    local pulls = director.rescue_pulls or 0
    local opportunities = saves + misses
    local orb_count, orb_remaining = active_orbs()
    local summon_warning_count, summon_warning_remaining = active_summon_portals()
    local pulse_warning_count, pulse_active_count, pulse_warning_remaining, pulse_active_remaining = active_pulses()
    local aggro = aggro_state(director.package or {})
    local next_threat = next_threat_state(director, director.package or {})
    local rescue_state
    if director.package and director.package.rescue then
        rescue_state = rescue_window_state(director, director.package.rescue)
    end
    return {
        outcome = "",
        title = package_title(director.package or {}, director.intensity or {}),
        frames = director.elapsed or 0,
        seconds = (director.elapsed or 0) / 60,
        escort_count = count,
        escort_hp = hp,
        escort_max_hp = max_hp,
        primary_name = primary_name,
        primary_count = primary_count,
        primary_hp = primary_hp,
        primary_max_hp = primary_max_hp,
        aggro_target_role = aggro.role,
        aggro_target_name = aggro.target_name,
        next_threat_id = next_threat.id,
        next_threat_label = next_threat.label,
        next_threat_state = next_threat.state,
        next_threat_frames = next_threat.frames,
        summon_warning_count = summon_warning_count,
        summon_warning_remaining = summon_warning_remaining,
        orb_active_count = orb_count,
        orb_active_remaining = orb_remaining,
        pulse_warning_count = pulse_warning_count,
        pulse_warning_remaining = pulse_warning_remaining,
        pulse_active_count = pulse_active_count,
        pulse_active_remaining = pulse_active_remaining,
        rescue_saves = saves,
        rescue_opportunities = opportunities,
        rescue_save_rate = safe_rate(saves, opportunities),
        rescue_clutch_saves = clutch,
        rescue_clutch_rate = safe_rate(clutch, saves),
        rescue_pulls = pulls,
        rescue_pull_rate = safe_rate(pulls, saves),
        rescue_misses = misses,
        rescue_best_margin = director.rescue_best_margin,
        rescue_last_margin = director.rescue_last_margin,
        rescue_last_pull_frames = director.rescue_last_pull_frames,
        rescue_next_frames = rescue_state and rescue_state.next_frames or nil,
        rescue_active_count = rescue_state and rescue_state.active_count or 0,
        rescue_active_remaining = rescue_state and rescue_state.remaining or nil,
        rescue_active_blocked = rescue_state and rescue_state.blocked or false,
        rescue_active_late = rescue_state and rescue_state.late or false,
        rescue_pull_active_frames = director.rescue_pull_active_frames or 0,
        rescue_pull_target = director.rescue_pull_target,
    }
end

function M.create_result_banner(args)
    return GameObject.object_create {
        map = args.map,
        xy = copy_xy(args.xy or {0, 0}),
        radius = 1,
        solid = false,
        depth = RESULT_DEPTH,
        elapsed = 0,
        title = args.title or "PACKAGE COMPLETE",
        subtitle = args.subtitle or "",
        detail = args.detail or "",
        color = args.color or COL_PALE_GREEN,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if self.elapsed > 240 then
                GameObject.destroy(self)
            end
        end,
        on_draw = function(self)
            local w = unpack(Display.display_size)
            local x1 = math.floor(w / 2 - 150)
            local x2 = math.floor(w / 2 + 150)
            local y1 = 22
            local y2 = 84
            local alpha = math.min(1, self.elapsed / 12)
            Display.draw_rectangle(with_alpha(COL_BLACK, 0.64 * alpha), {x1, y1, x2, y2})
            Display.draw_rectangle_outline(with_alpha(self.color, 0.72 * alpha), {x1, y1, x2, y2}, 1)
            draw_text(with_alpha(self.color, alpha), Display.CENTER_TOP, {math.floor(w / 2), y1 + 8}, self.title)
            draw_text(with_alpha(COL_WHITE, alpha), Display.CENTER_TOP, {math.floor(w / 2), y1 + 26}, self.subtitle)
            draw_text(with_alpha(COL_LIGHT_GRAY, alpha), Display.CENTER_TOP, {math.floor(w / 2), y1 + 43}, self.detail)
        end,
    }
end

function M.create_feedback_label(args)
    return GameObject.object_create {
        map = args.map,
        xy = copy_xy(args.xy or {0, 0}),
        radius = 1,
        solid = false,
        depth = RESULT_DEPTH - 1,
        elapsed = 0,
        duration = args.duration or 72,
        drift = args.drift or 0.45,
        text = args.text or "",
        color = args.color or COL_WHITE,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if self.elapsed >= self.duration then
                GameObject.destroy(self)
            end
        end,
        on_draw = function(self)
            local fade = math.min(1, self.elapsed / 8)
            if self.elapsed > self.duration - 18 then
                fade = math.min(fade, math.max(0, (self.duration - self.elapsed) / 18))
            end
            local screen_xy = Display.to_screen_xy({self.xy[1], self.xy[2] - self.elapsed * self.drift})
            draw_text(with_alpha(self.color, fade), Display.CENTER, screen_xy, self.text)
        end,
    }
end

local function object_label(obj, fallback)
    if obj and obj.name and obj.name ~= "" then
        return obj.name
    end
    return fallback or "target"
end

local function rescue_feedback(strike, blocker)
    local game_map = strike.strike_map or strike.map
    if not game_map then
        return
    end
    local color = strike.saved and COL_PALE_GREEN or COL_PALE_RED
    local xy = strike.saved and xy_of(blocker) or xy_of(strike.target)
    local text
    if strike.saved then
        text = strike.was_clutch and ("CLUTCH SAVE " .. strike.clutch_margin .. "f") or ("SAVE " .. strike.clutch_margin .. "f")
        if strike.pulled then
            text = text .. " PULL"
        end
    else
        text = "ALLY HIT"
    end
    M.create_feedback_label {
        map = game_map,
        xy = xy,
        text = text,
        color = color,
        duration = strike.saved and 86 or 72,
        drift = strike.saved and 0.52 or 0.42,
    }
    if strike.saved then
        local source_name = object_label(strike.source, "enemy")
        local blocker_name = object_label(blocker, "rescuer")
        EventLog.add(text .. ": " .. source_name .. " retargeted to " .. blocker_name .. ".", color)
    else
        EventLog.add("Rescue missed: " .. object_label(strike.target, "ally") .. " was hit.", color)
    end
end

local function finish_package(director, outcome)
    if director.content_lab_finished then
        return
    end
    director.content_lab_finished = true
    local result = package_result(director, outcome)
    if director.map then
        director.map.content_lab_package_result = result
        director.map.content_lab_director = nil
    end
    EventLog.add(result.title .. ": " .. result.subtitle .. " (" .. result.detail .. ")", result.color)
    M.create_result_banner {
        map = director.map,
        xy = director.xy,
        title = result.title,
        subtitle = result.subtitle,
        detail = result.detail,
        color = result.color,
    }
    GameObject.destroy(director)
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
        content_lab_pulse = true,
        windup_frames = windup,
        active_frames = active,
        linger_frames = linger,
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
        content_lab_orb = true,
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

function M.create_summon_portal(args)
    return GameObject.object_create {
        map = args.map,
        xy = copy_xy(args.xy),
        radius = 1,
        solid = false,
        depth = HAZARD_DEPTH + 1,
        elapsed = 0,
        windup = args.windup or 24,
        monster_type = args.monster_type,
        director = args.director,
        max_alive = args.max_alive or 4,
        portal_radius = args.portal_radius or 24,
        resolved = false,
        content_lab_summon_portal = true,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if self.director and self.director.destroyed then
                GameObject.destroy(self)
                return
            end
            if not self.resolved and self.elapsed >= self.windup then
                self.resolved = true
                if self.monster_type and summon_count() < self.max_alive then
                    GameObject.enemy_create {
                        map = self.map,
                        type = self.monster_type,
                        xy = round_xy(self.xy),
                        content_lab_summon = true,
                    }
                end
                GameObject.destroy(self)
            end
        end,
        on_draw = function(self)
            local remaining = math.max(0, self.windup - self.elapsed)
            local progress = math.max(0.15, math.min(1, self.elapsed / math.max(1, self.windup)))
            local radius = self.portal_radius * (0.55 + progress * 0.45)
            local pulse = 1 + math.sin(self.elapsed * 0.42) * 0.08
            draw_world_circle(COL_PALE_YELLOW, self.xy, radius * pulse, 0.08 + progress * 0.10, 0.42 + progress * 0.32)
            draw_world_circle(COL_PALE_RED, self.xy, math.max(6, radius * 0.42), 0.05, 0.35 + progress * 0.28)
            local label_xy = Display.to_screen_xy({self.xy[1], self.xy[2] - self.portal_radius - 12})
            draw_text(COL_PALE_YELLOW, Display.CENTER, label_xy, "ADD " .. format_frames(remaining))
        end,
    }
end

local function rescue_target()
    local escorts = living_escorts()
    return escorts[1]
end

local function rescue_source_xy(strike)
    if strike.source and not strike.source.destroyed then
        return xy_of(strike.source)
    end
    return copy_xy(strike.source_xy)
end

local function rescue_blocker(strike)
    local target = strike.target
    if not target or target.destroyed then
        return nil
    end
    local from_xy = rescue_source_xy(strike)
    local to_xy = xy_of(target)
    local best
    local best_dist
    for _, player in ipairs(living_players()) do
        local player_xy = xy_of(player)
        local radius = (strike.block_radius or 30) + (player.target_radius or player.radius or 12)
        if dist_to_segment_sqr(player_xy, from_xy, to_xy) <= sqr(radius) then
            local target_dist = dist_sqr(player_xy, to_xy)
            if not best_dist or target_dist < best_dist then
                best = player
                best_dist = target_dist
            end
        end
    end
    return best
end

local function rescue_intercept_hint(strike)
    local target = strike.target
    if not target or target.destroyed then
        return nil
    end
    local from_xy = rescue_source_xy(strike)
    local to_xy = xy_of(target)
    local best_xy
    local best_player
    local best_dist
    for _, player in ipairs(living_players()) do
        local xy, _, dist = nearest_segment_point(xy_of(player), from_xy, to_xy)
        if not best_dist or dist < best_dist then
            best_xy = xy
            best_player = player
            best_dist = dist
        end
    end
    return best_xy, best_player, best_dist
end

function M.create_pull_tether(args)
    return GameObject.object_create {
        map = args.map,
        xy = copy_xy(args.xy or (args.source and xy_of(args.source)) or {0, 0}),
        radius = 1,
        solid = false,
        depth = HAZARD_DEPTH + 2,
        elapsed = 0,
        duration = args.duration or 90,
        source = args.source,
        target = args.target,
        director = args.director,
        content_lab_pull_tether = true,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            local remaining = math.max(0, self.duration - self.elapsed)
            if not self.source or self.source.destroyed or not self.target or self.target.destroyed or self.elapsed >= self.duration then
                if self.director and not self.director.destroyed then
                    if not self.director.rescue_pull_active_id or self.director.rescue_pull_active_id == self.id then
                        self.director.rescue_pull_active_frames = 0
                        self.director.rescue_pull_active_id = nil
                    end
                end
                GameObject.destroy(self)
                return
            end
            self.xy = xy_of(self.source)
            if self.director and not self.director.destroyed then
                self.director.rescue_pull_active_frames = remaining
                self.director.rescue_pull_active_id = self.id
                self.director.rescue_pull_target = self.target.name
            end
        end,
        on_draw = function(self)
            if not self.source or self.source.destroyed or not self.target or self.target.destroyed then
                return
            end
            local remaining = math.max(0, self.duration - self.elapsed)
            local pct = remaining / math.max(1, self.duration)
            local from_xy = xy_of(self.source)
            local to_xy = xy_of(self.target)
            local alpha = 0.18 + pct * 0.45
            draw_world_link(COL_PALE_YELLOW, from_xy, to_xy, 4, alpha)
            draw_world_circle(COL_PALE_YELLOW, from_xy, (self.source.radius or 14) + 7, 0.03, 0.48 + pct * 0.26)
            draw_world_circle(COL_PALE_GREEN, to_xy, (self.target.radius or 12) + 6, 0.02, 0.40 + pct * 0.24)
            local source_label = Display.to_screen_xy({from_xy[1], from_xy[2] - (self.source.radius or 14) - 20})
            local target_label = Display.to_screen_xy({to_xy[1], to_xy[2] - (self.target.radius or 12) - 20})
            draw_text(COL_PALE_YELLOW, Display.CENTER, source_label, "PULL " .. remaining .. "f")
            draw_text(COL_PALE_GREEN, Display.CENTER, target_label, "TAUNTED")
        end,
    }
end

function M.create_rescue_strike(args)
    return GameObject.object_create {
        map = args.map,
        strike_map = args.map,
        xy = copy_xy(args.target and xy_of(args.target) or args.xy),
        radius = 1,
        solid = false,
        depth = HAZARD_DEPTH + 1,
        elapsed = 0,
        content_lab_rescue_strike = true,
        source = args.source,
        source_xy = args.source and xy_of(args.source) or copy_xy(args.xy),
        target = args.target,
        director = args.director,
        windup = args.windup or 68,
        late_window = args.late_window or 22,
        linger = args.linger or 18,
        strike_radius = args.radius or 42,
        block_radius = args.block_radius or 30,
        ally_damage = args.ally_damage or 10,
        block_damage = args.block_damage or 4,
        pull_frames = args.pull_frames or 90,
        power = args.power or 24,
        magic_percentage = args.magic_percentage or 1,
        resolved = false,
        saved = false,
        pulled = false,
        was_clutch = false,
        current_blocker = nil,
        block_started_elapsed = nil,
        clutch_margin = nil,
        result_elapsed = 0,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if not self.target or self.target.destroyed then
                GameObject.destroy(self)
                return
            end
            self.xy = xy_of(self.target)
            if not self.resolved then
                local blocker = rescue_blocker(self)
                if blocker then
                    if not self.current_blocker or self.current_blocker.id ~= blocker.id then
                        self.block_started_elapsed = self.elapsed
                    end
                    self.current_blocker = blocker
                else
                    self.current_blocker = nil
                    self.block_started_elapsed = nil
                end
            end
            if not self.resolved and self.elapsed >= self.windup then
                local blocker = self.current_blocker
                if blocker then
                    local margin = math.max(0, self.windup - (self.block_started_elapsed or self.elapsed))
                    self.clutch_margin = margin
                    self.was_clutch = margin <= self.late_window
                    self.saved = true
                    self.damage = self.block_damage
                    damage_player(self, blocker)
                    self.pulled = force_chase(self.source, blocker, self.pull_frames)
                    if self.pulled then
                        M.create_pull_tether {
                            map = self.strike_map or self.map,
                            xy = xy_of(self.source),
                            source = self.source,
                            target = blocker,
                            director = self.director,
                            duration = self.pull_frames,
                        }
                    end
                    if self.director and not self.director.destroyed then
                        self.director.rescue_saves = (self.director.rescue_saves or 0) + 1
                        self.director.rescue_last_margin = margin
                        if self.pulled then
                            self.director.rescue_pulls = (self.director.rescue_pulls or 0) + 1
                            self.director.rescue_last_pull_frames = self.pull_frames
                        end
                        if self.was_clutch then
                            self.director.rescue_clutch_saves = (self.director.rescue_clutch_saves or 0) + 1
                            self.director.rescue_best_margin = math.min(self.director.rescue_best_margin or margin, margin)
                        end
                    end
                    rescue_feedback(self, blocker)
                else
                    self.saved = false
                    self.damage = self.ally_damage
                    damage_player(self, self.target)
                    if self.director and not self.director.destroyed then
                        self.director.rescue_misses = (self.director.rescue_misses or 0) + 1
                    end
                    rescue_feedback(self)
                end
                self.resolved = true
            elseif self.resolved then
                self.result_elapsed = self.result_elapsed + 1
                if self.result_elapsed >= self.linger then
                    GameObject.destroy(self)
                end
            end
        end,
        on_draw = function(self)
            if not self.target or self.target.destroyed then
                return
            end
            local from_xy = rescue_source_xy(self)
            local to_xy = xy_of(self.target)
            local remaining = math.max(0, self.windup - self.elapsed)
            local last_call = remaining <= self.late_window
            local clutch_pct = last_call and (1 - remaining / math.max(1, self.late_window)) or 0
            local color = self.resolved and (self.saved and COL_PALE_GREEN or COL_PALE_RED)
                or (self.current_blocker and COL_PALE_GREEN or (last_call and COL_PALE_YELLOW or COL_PALE_GREEN))
            local progress = self.resolved and 1 or math.max(0.2, math.min(1, self.elapsed / self.windup))
            draw_world_link(color, from_xy, to_xy, last_call and 5 or 3, last_call and 0.58 or 0.28)
            if not self.resolved then
                local threat_xy = segment_point_at(from_xy, to_xy, math.min(1, self.elapsed / math.max(1, self.windup)))
                draw_world_circle(last_call and COL_PALE_YELLOW or COL_WHITE, threat_xy, last_call and 7 or 5, 0.10, last_call and 0.82 or 0.58)
                local hint_xy, hint_player, hint_dist = rescue_intercept_hint(self)
                if hint_xy and hint_player then
                    local block_radius = self.block_radius or 30
                    local player_radius = hint_player.target_radius or hint_player.radius or 12
                    local in_block_zone = hint_dist <= sqr(block_radius + player_radius)
                    local hint_color = in_block_zone and COL_PALE_GREEN or (last_call and COL_PALE_YELLOW or COL_LIGHT_GRAY)
                    draw_world_circle(hint_color, hint_xy, block_radius, in_block_zone and 0.05 or 0.02, in_block_zone and 0.72 or 0.42)
                    local hint_label_xy = Display.to_screen_xy({hint_xy[1], hint_xy[2] - block_radius - 10})
                    local hint_label = in_block_zone and (last_call and "CLUTCH HOLD" or "HOLD") or (last_call and "BLOCK NOW" or "INTERCEPT")
                    draw_text(hint_color, Display.CENTER, hint_label_xy, hint_label)
                end
            end
            draw_world_circle(color, to_xy, self.strike_radius * progress, self.resolved and 0.14 or 0.07, self.resolved and 0.70 or 0.45)
            if last_call and not self.resolved then
                local pulse = 1 + math.sin(self.elapsed * 0.55) * 0.08
                draw_world_circle(COL_PALE_YELLOW, to_xy, (self.strike_radius + 9 + clutch_pct * 12) * pulse, 0.02, 0.58 + clutch_pct * 0.24)
            end
            local label_xy = Display.to_screen_xy({to_xy[1], to_xy[2] - self.strike_radius - 14})
            local label
            if self.resolved then
                label = self.saved
                    and (self.was_clutch and ("CLUTCH " .. self.clutch_margin .. "f") or (self.pulled and "PULLED" or "SAVED"))
                    or "HIT"
            elseif self.current_blocker then
                label = last_call and ("CLUTCH HOLD " .. remaining .. "f") or ("HOLD " .. remaining .. "f")
            else
                label = last_call and ("BLOCK NOW " .. remaining .. "f") or ("RESCUE " .. remaining .. "f")
            end
            draw_text(color, Display.CENTER, label_xy, label)
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

local function spawn_rescue_strike(director, package)
    local rescue = package.rescue or {}
    local target = rescue_target()
    if not target then
        return
    end
    local source = primary_monster(package.primary)
    local intensity = director.intensity
    M.create_rescue_strike {
        map = director.map,
        source = source,
        target = target,
        director = director,
        xy = xy_of(target),
        windup = scale_frames(rescue.windup, 68, intensity),
        late_window = scale_frames(rescue.late_window, 22, intensity),
        linger = rescue.linger or 18,
        radius = rescue.radius or 42,
        block_radius = rescue.block_radius or 30,
        ally_damage = scale_damage(rescue.ally_damage, 10, intensity),
        block_damage = scale_damage(rescue.block_damage, 4, intensity),
        pull_frames = scale_frames(rescue.pull_frames, 90, intensity),
        power = scale_damage(rescue.power, 24, intensity),
    }
end

local function spawn_escorts(director, package)
    local escort = package.escort or {}
    if not escort.monster then
        return
    end
    local center = director.xy
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
            ally.stats.max_hp = hp
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

function active_summon_portals()
    local count = 0
    local best_remaining
    for _, obj in ipairs(Map.objects_list()) do
        if obj and not obj.destroyed and obj.content_lab_summon_portal and not obj.resolved then
            count = count + 1
            local remaining = math.max(0, (obj.windup or 0) - (obj.elapsed or 0))
            best_remaining = math.min(best_remaining or remaining, remaining)
        end
    end
    return count, best_remaining
end

function active_orbs()
    local count = 0
    local best_remaining
    for _, obj in ipairs(Map.objects_list()) do
        if obj and not obj.destroyed and obj.content_lab_orb then
            count = count + 1
            local remaining = math.max(0, (obj.duration or 0) - (obj.elapsed or 0))
            best_remaining = math.min(best_remaining or remaining, remaining)
        end
    end
    return count, best_remaining
end

function active_pulses()
    local warning_count = 0
    local active_count = 0
    local warning_remaining
    local active_remaining
    for _, obj in ipairs(Map.objects_list()) do
        if obj and not obj.destroyed and obj.content_lab_pulse then
            local elapsed = obj.elapsed or 0
            local windup = obj.windup_frames or 0
            local active = obj.active_frames or 0
            if elapsed <= windup then
                warning_count = warning_count + 1
                local remaining = math.max(0, windup - elapsed)
                warning_remaining = math.min(warning_remaining or remaining, remaining)
            elseif elapsed <= windup + active then
                active_count = active_count + 1
                local remaining = math.max(0, windup + active - elapsed)
                active_remaining = math.min(active_remaining or remaining, remaining)
            end
        end
    end
    return warning_count, active_count, warning_remaining, active_remaining
end

function active_rescue_strikes()
    local count = 0
    local blocked = false
    local late = false
    local best_remaining
    for _, obj in ipairs(Map.objects_list()) do
        if obj and not obj.destroyed and obj.content_lab_rescue_strike and not obj.resolved then
            count = count + 1
            local remaining = math.max(0, obj.windup - obj.elapsed)
            best_remaining = math.min(best_remaining or remaining, remaining)
            if remaining <= (obj.late_window or 0) then
                late = true
            end
            if obj.current_blocker then
                blocked = true
            end
        end
    end
    return count, blocked, best_remaining, late
end

function rescue_window_state(director, rescue)
    local intensity = director and director.intensity or normalize_intensity(nil)
    local interval, offset = scaled_interval_and_offset(rescue or {}, 205, 54, intensity)
    local active_count, blocked, remaining, late = active_rescue_strikes()
    local windup = scale_frames((rescue or {}).windup, 68, intensity)
    local late_window = scale_frames((rescue or {}).late_window, 22, intensity)
    return {
        interval = interval,
        offset = offset,
        next_frames = next_delta(director and director.elapsed or 0, interval, offset),
        windup = windup,
        late_window = late_window,
        active_count = active_count,
        blocked = blocked,
        remaining = remaining,
        late = late,
    }
end

local function upcoming_candidate(package, intensity, elapsed, id, label, entry, fallback_interval, fallback_offset)
    if not entry then
        return nil
    end
    local interval, offset = scaled_interval_and_offset(entry, fallback_interval, fallback_offset, intensity)
    local frames = next_delta(elapsed, interval, offset)
    return {
        id = id,
        label = label,
        state = "next",
        frames = frames,
        interval = interval,
        pct = 1 - frames / math.max(1, interval),
        color = COL_LIGHT_GRAY,
    }
end

function next_threat_state(director, package)
    director = director or {}
    package = package or {}
    local intensity = director.intensity or normalize_intensity(nil)
    local elapsed = director.elapsed or 0

    local rescue = package.rescue and rescue_window_state(director, package.rescue) or nil
    if (director.rescue_pull_active_frames or 0) > 0 then
        return {
            id = "pull",
            label = "PULL",
            state = "active",
            frames = director.rescue_pull_active_frames or 0,
            pct = 1,
            color = COL_PALE_YELLOW,
        }
    elseif rescue and rescue.active_count and rescue.active_count > 0 then
        return {
            id = "rescue",
            label = rescue.late and "SAVE" or "LINE",
            state = rescue.blocked and "hold" or rescue.late and "clutch" or "block",
            frames = rescue.remaining or 0,
            pct = 1,
            color = rescue.blocked and COL_PALE_GREEN or rescue.late and COL_PALE_YELLOW or COL_PALE_GREEN,
        }
    end

    local pulse_warning, pulse_active, pulse_warning_remaining, pulse_active_remaining = active_pulses()
    if pulse_active > 0 then
        return {
            id = "pulse",
            label = "AOE",
            state = "active",
            frames = pulse_active_remaining or 0,
            pct = 1,
            color = COL_PALE_RED,
        }
    elseif pulse_warning > 0 then
        return {
            id = "pulse",
            label = "AOE",
            state = "warn",
            frames = pulse_warning_remaining or 0,
            pct = 1,
            color = COL_PALE_YELLOW,
        }
    end

    local orb_count, orb_remaining = active_orbs()
    if orb_count > 0 then
        return {
            id = "orbit",
            label = "ORBS",
            state = "active",
            frames = orb_remaining or 0,
            pct = 1,
            color = COL_PALE_BLUE,
        }
    end

    local summon_warning_count, summon_warning_remaining = active_summon_portals()
    if summon_warning_count > 0 then
        return {
            id = "summon",
            label = "ADDS",
            state = "warn",
            frames = summon_warning_remaining or 0,
            pct = 1,
            color = COL_PALE_YELLOW,
        }
    end

    local candidates = {}
    local function add(candidate)
        if candidate then
            candidates[#candidates + 1] = candidate
        end
    end
    add(upcoming_candidate(package, intensity, elapsed, "orbit", "ORBS", package.orbit, 300, 40))
    add(upcoming_candidate(package, intensity, elapsed, "pulse", "AOE", package.pulse, 150, 70))
    add(upcoming_candidate(package, intensity, elapsed, "summon", "ADDS", package.summon, 360, 120))
    add(upcoming_candidate(package, intensity, elapsed, "rescue", "SAVE", package.rescue, 205, 54))

    local best
    for _, candidate in ipairs(candidates) do
        if not best or (candidate.frames or 0) < (best.frames or 0) then
            best = candidate
        end
    end
    if best then
        if (best.frames or 0) <= 45 then
            best.color = best.id == "pulse" and COL_PALE_RED or COL_PALE_YELLOW
        else
            best.color = best.id == "orbit" and COL_PALE_BLUE or best.id == "rescue" and COL_PALE_GREEN or COL_LIGHT_GRAY
        end
        return best
    end
    return {
        id = "",
        label = "",
        state = "",
        frames = "",
        pct = 0,
        color = COL_LIGHT_GRAY,
    }
end

function M.package_state(game_map)
    if not game_map then
        return nil
    end
    if game_map.content_lab_package_result then
        return game_map.content_lab_package_result
    end
    return package_state_from_director(game_map.content_lab_director)
end

local function in_spawn_bounds(game_map, xy, radius)
    local bounds = game_map and game_map.content_lab_spawn_bounds
    if not bounds then
        return xy[1] >= radius and xy[2] >= radius
    end
    return xy[1] >= bounds[1] + radius
        and xy[2] >= bounds[2] + radius
        and xy[1] <= bounds[3] - radius
        and xy[2] <= bounds[4] - radius
end

local function free_spawn_xy(game_map, xy, radius)
    local rounded = round_xy(xy)
    radius = radius or 12
    if not in_spawn_bounds(game_map, rounded, radius) then
        return nil
    end
    if Map.radius_tile_check(game_map, rounded, radius) then
        return nil
    end
    return rounded
end

local function summon_spawn_xy(director, center, base_radius, idx, count)
    local base_angle = (idx - 1) * (math.pi * 2 / count) + director.elapsed * 0.03
    local radii = {
        base_radius,
        base_radius * 0.75,
        base_radius * 0.5,
        48,
        28,
    }
    for _, ring_radius in ipairs(radii) do
        if ring_radius >= 10 then
            for turn = 0, 7 do
                local angle = base_angle + turn * (math.pi / 4)
                local xy = {
                    center[1] + math.cos(angle) * ring_radius,
                    center[2] + math.sin(angle) * ring_radius,
                }
                local free_xy = free_spawn_xy(director.map, xy, 12)
                if free_xy then
                    return free_xy
                end
            end
        end
    end
    return free_spawn_xy(director.map, center_of_players() or director.xy, 12)
        or free_spawn_xy(director.map, director.xy, 12)
        or round_xy(director.xy)
end

local function spawn_summons(director, package)
    local summon = package.summon or {}
    local intensity = director.intensity
    local max_alive = scale_count(summon.max_alive, 4, intensity)
    if summon_count() + active_summon_portals() >= max_alive then
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
    local windup = scale_frames(summon.windup, 24, intensity)
    for idx = 1, count do
        if summon_count() + active_summon_portals() >= max_alive then
            return
        end
        local xy = summon_spawn_xy(director, center, radius, idx, count)
        M.create_summon_portal {
            map = director.map,
            director = director,
            xy = xy,
            monster_type = monsters[((idx - 1) % #monsters) + 1],
            windup = windup,
            max_alive = max_alive,
            portal_radius = summon.portal_radius or 24,
        }
    end
end

function M.add_director(game_map, package, intensity)
    package = package or {}
    if not (package.orbit or package.pulse or package.summon or package.escort or package.rescue) then
        return nil
    end
    intensity = normalize_intensity(intensity)
    local center = game_map.content_lab_spawn_center or {320, 320}
    local director = GameObject.object_create {
        map = game_map,
        xy = center,
        radius = 1,
        solid = false,
        depth = DIRECTOR_DEPTH,
        elapsed = 0,
        package = package,
        intensity = intensity,
        rescue_saves = 0,
        rescue_misses = 0,
        rescue_clutch_saves = 0,
        rescue_pulls = 0,
        rescue_pull_active_frames = 0,
        escort_required = package.escort ~= nil,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if #living_players() == 0 then
                finish_package(self, "party down")
                return
            end
            if self.escort_required and #living_escorts() == 0 then
                finish_package(self, "ally down")
                return
            end
            if #living_monsters(false) == 0 then
                finish_package(self, "clear")
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
            local rescue = self.package.rescue
            if rescue then
                local interval = scale_frames(rescue.interval, 205, self.intensity)
                local offset = scale_offset(rescue.offset, 54, interval, self.intensity)
                if self.elapsed % interval == offset then
                    spawn_rescue_strike(self, self.package)
                end
            end
        end,
        on_draw = function(self)
            draw_package_hud(self)
        end,
    }
    game_map.content_lab_director = director
    spawn_escorts(director, package)
    return director
end

return M
