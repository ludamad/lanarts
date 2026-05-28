local Display = require "core.Display"
local GameState = require "core.GameState"
local Map = require "core.Map"

local M = {}

local TWO_PI = math.pi * 2

local palettes = {
    boss = {
        inner = {255, 220, 120, 34},
        outer = {255, 70, 30, 0},
        arc = COL_PALE_YELLOW,
        spoke = COL_PALE_RED,
    },
    red = {
        inner = {255, 90, 30, 42},
        outer = {255, 20, 0, 0},
        arc = COL_PALE_RED,
        spoke = COL_PALE_YELLOW,
    },
    black = {
        inner = {150, 80, 255, 38},
        outer = {30, 0, 80, 0},
        arc = {190, 145, 255},
        spoke = COL_PALE_BLUE,
    },
    green = {
        inner = {130, 255, 120, 32},
        outer = {10, 80, 25, 0},
        arc = COL_PALE_GREEN,
        spoke = COL_PALE_YELLOW,
    },
    blue = {
        inner = {120, 190, 255, 34},
        outer = {20, 40, 120, 0},
        arc = COL_PALE_BLUE,
        spoke = COL_WHITE,
    },
}

local function palette_for(name)
    return palettes[name or ""] or palettes.boss
end

local function screen_xy(xy)
    return Display.to_screen_xy(xy)
end

local function alpha(color, amount)
    return with_alpha(color, math.max(0, math.min(1, amount or 1)))
end

local function clamp(value, min_value, max_value)
    value = tonumber(value) or 0
    if value < min_value then
        return min_value
    elseif value > max_value then
        return max_value
    end
    return value
end

function M.draw_world_rune(args)
    local xy = screen_xy(args.xy)
    local frame = args.frame or GameState.frame or 0
    local radius = args.radius or 80
    local pulse = 0.5 + 0.5 * math.sin(frame * (args.pulse_speed or 0.055))
    local pal = palette_for(args.palette)
    local intensity = args.intensity or 1

    Display.draw_radial_gradient(
        pal.inner,
        pal.outer,
        xy,
        radius * (1.05 + pulse * 0.08),
        args.gradient_steps or 8
    )

    local arcs = args.arcs or 3
    for idx = 1, arcs do
        local base = frame * (args.rotation_speed or 0.024) * (idx % 2 == 0 and -0.7 or 1)
        local start_angle = base + idx * TWO_PI / arcs
        local length = args.arc_length or (math.pi * 0.62)
        local arc_radius = radius * (0.68 + idx * 0.12)
        Display.draw_arc(
            alpha(pal.arc, (0.34 + pulse * 0.34) * intensity),
            xy,
            arc_radius,
            start_angle,
            start_angle + length,
            args.line_width or 2,
            args.segments or 24
        )
    end

    local spokes = args.spokes or 6
    for idx = 1, spokes do
        local angle = frame * (args.spoke_speed or -0.012) + idx * TWO_PI / spokes
        local inner = radius * (args.spoke_inner or 0.30)
        local outer = radius * (args.spoke_outer or 0.92)
        local from_xy = {
            xy[1] + math.cos(angle) * inner,
            xy[2] + math.sin(angle) * inner,
        }
        local to_xy = {
            xy[1] + math.cos(angle) * outer,
            xy[2] + math.sin(angle) * outer,
        }
        Display.draw_line(alpha(pal.spoke, (0.12 + pulse * 0.18) * intensity), from_xy, to_xy, args.spoke_width or 1)
    end
end

function M.draw_world_pressure_field(args)
    args = args or {}
    local xy = screen_xy(args.xy)
    local frame = args.frame or GameState.frame or 0
    local radius = args.radius or 96
    local pressure = math.max(1, math.floor(tonumber(args.pressure or args.stack or 1) or 1))
    local active = math.max(0, math.floor(tonumber(args.active or pressure) or pressure))
    local pal = palette_for(args.palette)
    local intensity = clamp(args.intensity or 1, 0, 1.6)
    local beat = 0.5 + 0.5 * math.sin(frame * (args.beat_speed or 0.070) + pressure * 0.63)
    local breath = 0.5 + 0.5 * math.sin(frame * (args.breath_speed or 0.031))

    Display.draw_radial_gradient(
        alpha(pal.arc, (0.08 + beat * 0.08) * intensity),
        pal.outer,
        xy,
        radius * (0.98 + beat * 0.08),
        args.gradient_steps or 7
    )

    local rings = args.rings or math.min(5, pressure + 2)
    for idx = 1, rings do
        local pct = idx / rings
        local ring_radius = radius * (0.35 + pct * 0.62)
            + math.sin(frame * 0.045 + idx * 1.7) * radius * 0.018
        local ring_alpha = (0.11 + beat * 0.13) * (1 - pct * 0.32) * intensity
        local color = idx <= active and pal.spoke or pal.arc
        Display.draw_circle_outline(alpha(color, ring_alpha), xy, ring_radius, idx == rings and 2 or 1)
    end

    local arcs = args.arcs or math.min(7, pressure + 3)
    for idx = 1, arcs do
        local direction = idx % 2 == 0 and -0.76 or 1
        local base = frame * (args.rotation_speed or 0.026) * direction
        local start_angle = base + idx * TWO_PI / arcs + breath * 0.24
        local length = args.arc_length or math.min(math.pi * 0.78, math.pi * (0.26 + pressure * 0.08))
        local arc_radius = radius * (0.44 + (idx % 4) * 0.13)
        local arc_alpha = (0.18 + beat * 0.18) * intensity
        Display.draw_arc(
            alpha(pal.arc, arc_alpha),
            xy,
            arc_radius,
            start_angle,
            start_angle + length,
            args.line_width or 2,
            args.segments or 24
        )
    end

    local spokes = args.spokes or math.min(10, 4 + pressure * 2)
    for idx = 1, spokes do
        local angle = frame * (args.spoke_speed or -0.018) + idx * TWO_PI / spokes
        local wobble = 1 + math.sin(frame * 0.050 + idx) * 0.05
        local inner = radius * (0.20 + beat * 0.04)
        local outer = radius * (0.86 + 0.08 * wobble)
        Display.draw_line(
            alpha(pal.spoke, (0.08 + beat * 0.12) * intensity),
            {xy[1] + math.cos(angle) * inner, xy[2] + math.sin(angle) * inner},
            {xy[1] + math.cos(angle) * outer, xy[2] + math.sin(angle) * outer},
            args.spoke_width or 1
        )
    end
end

function M.draw_boss_pressure_aura(obj, args)
    if not obj or obj.destroyed or not Map.object_visible(obj) then
        return
    end
    args = args or {}
    args.xy = {obj.x, obj.y}
    args.radius = args.radius or ((obj.radius or 16) + 72)
    M.draw_world_pressure_field(args)
end

function M.draw_boss_aura(obj, args)
    if not obj or obj.destroyed or not Map.object_visible(obj) then
        return
    end
    args = args or {}
    local frame = args.frame or GameState.frame or 0
    local radius = args.radius or ((obj.radius or 16) + 36)
    M.draw_world_rune {
        xy = {obj.x, obj.y},
        frame = frame,
        radius = radius,
        palette = args.palette or "boss",
        arcs = args.arcs or 4,
        spokes = args.spokes or 8,
        rotation_speed = args.rotation_speed or 0.018,
        spoke_speed = args.spoke_speed or -0.009,
        intensity = args.intensity or 1,
    }
end

function M.draw_world_burst(args)
    args = args or {}
    local xy = screen_xy(args.xy)
    local frame = args.frame or 0
    local duration = math.max(1, args.duration or 36)
    local pct = math.max(0, math.min(1, frame / duration))
    local fade = math.max(0, 1 - pct)
    local color = args.color or COL_WHITE
    local radius = args.radius or 72
    local pulse_radius = radius * (0.22 + pct * 0.92)

    Display.draw_radial_gradient(
        alpha(color, 0.18 * fade),
        {color[1] or 255, color[2] or 255, color[3] or 255, 0},
        xy,
        pulse_radius,
        args.gradient_steps or 5
    )
    Display.draw_circle_outline(alpha(color, 0.82 * fade), xy, pulse_radius, args.line_width or 2)
    Display.draw_circle_outline(alpha(COL_WHITE, 0.34 * fade), xy, math.max(4, pulse_radius * 0.52), 1)

    local spokes = args.spokes or 8
    for idx = 1, spokes do
        local angle = idx * TWO_PI / spokes + frame * (args.rotation_speed or 0.08)
        local inner = pulse_radius * 0.24
        local outer = pulse_radius * (0.82 + 0.12 * math.sin(frame * 0.3 + idx))
        Display.draw_line(
            alpha(color, 0.34 * fade),
            {xy[1] + math.cos(angle) * inner, xy[2] + math.sin(angle) * inner},
            {xy[1] + math.cos(angle) * outer, xy[2] + math.sin(angle) * outer},
            1
        )
    end
end

function M.draw_screen_edges(args)
    args = args or {}
    local w, h = unpack(Display.display_size)
    local color = args.color or COL_WHITE
    local alpha_value = math.max(0, math.min(1, args.alpha or args.intensity or 0.18))
    local thickness = math.max(1, args.thickness or 18)
    local passes = args.passes or 3

    for idx = 1, passes do
        local pct = idx / passes
        local a = alpha_value * (1 - (idx - 1) / passes)
        local t = thickness * pct
        Display.draw_rectangle(alpha(color, a), {0, 0, w, t})
        Display.draw_rectangle(alpha(color, a), {0, h - t, w, h})
        Display.draw_rectangle(alpha(color, a), {0, 0, t, h})
        Display.draw_rectangle(alpha(color, a), {w - t, 0, w, h})
    end
end

local function draw_die_face(center, roll, size, color, opacity)
    roll = math.floor(tonumber(roll) or 0)
    if roll <= 0 then
        return
    end
    opacity = clamp(opacity or 1, 0, 1)
    local half = size / 2
    local box = {center[1] - half, center[2] - half, center[1] + half, center[2] + half}
    Display.draw_rectangle(alpha(COL_BLACK, 0.44 * opacity), box)
    Display.draw_rectangle_outline(alpha(color, 0.92 * opacity), box, 1)

    local off = size * 0.25
    local pips = {}
    if roll == 1 then
        pips = {{0, 0}}
    elseif roll == 2 then
        pips = {{-off, -off}, {off, off}}
    elseif roll == 3 then
        pips = {{-off, -off}, {0, 0}, {off, off}}
    elseif roll == 4 then
        pips = {{-off, -off}, {off, -off}, {-off, off}, {off, off}}
    elseif roll == 5 then
        pips = {{-off, -off}, {off, -off}, {0, 0}, {-off, off}, {off, off}}
    else
        pips = {{-off, -off}, {off, -off}, {-off, 0}, {off, 0}, {-off, off}, {off, off}}
    end

    local pip_radius = math.max(1.6, size * 0.075)
    for _, pip in ipairs(pips) do
        Display.draw_circle(
            alpha(COL_WHITE, 0.88 * opacity),
            {center[1] + pip[1], center[2] + pip[2]},
            pip_radius
        )
    end
end

function M.draw_die_badge(obj, args)
    if not obj or obj.destroyed or not Map.object_visible(obj) then
        return
    end
    args = args or {}
    local roll = args.roll or 0
    local remaining = math.max(0, args.remaining or 0)
    local interval = math.max(1, args.interval or 1)
    local pct = remaining / interval
    local rolling = args.rolling == true
    local rolling_remaining = math.max(0, args.rolling_remaining or 0)
    local rolling_total = math.max(1, args.rolling_total or 1)
    local rolling_pct = clamp(1 - rolling_remaining / rolling_total, 0, 1)
    local lock_total = math.max(1, args.lock_total or 3)
    local lock_stage = math.max(0, math.min(lock_total, args.lock_stage or 0))
    local surge_total = math.max(1, args.surge_total or 5)
    local surge_stage = math.max(0, math.min(surge_total, args.surge_stage or 0))
    local label = args.label or ""
    local buff_remaining = math.max(0, args.buff_remaining or 0)
    local buff_duration = math.max(1, args.buff_duration or 1)
    local buff_pct = clamp(buff_remaining / buff_duration, 0, 1)
    local color = args.color or COL_PALE_BLUE
    local frame = GameState.frame or 0
    local near_drop = rolling and rolling_pct >= 0.78 and rolling_pct < 0.91
    local final_lock = rolling and rolling_pct >= 0.91
    local tension = rolling and clamp((lock_stage + surge_stage) / (lock_total + surge_total), 0, 1) or 0
    local base_xy = screen_xy({obj.x, obj.y - (obj.radius or 14) - 30})
    local shake = rolling and (0.8 + rolling_pct * 2.0 + surge_stage * 0.18 + (final_lock and 1.4 or near_drop and 0.7 or 0)) or 0
    local xy = {
        base_xy[1] + math.sin(frame * 0.73) * shake,
        base_xy[2] + math.cos(frame * 0.61) * shake * 0.45,
    }
    local pulse = 0.5 + 0.5 * math.sin(frame * 0.32)
    local w = rolling and (final_lock and 102 or near_drop and 98 or 92) or label ~= "" and 72 or 48
    local h = rolling and (final_lock and 43 or near_drop and 40 or 38) or label ~= "" and 30 or 20
    local box = {xy[1] - w / 2, xy[2] - h / 2, xy[1] + w / 2, xy[2] + h / 2}
    Display.draw_radial_gradient(
        rolling and alpha(color, 0.20 + pulse * 0.12 + tension * 0.07 + (final_lock and 0.10 or 0)) or {120, 170, 255, 35},
        {20, 30, 70, 0},
        xy,
        rolling and (43 + pulse * 5 + tension * 9 + (final_lock and 7 or 0)) or label ~= "" and 39 or 34,
        5
    )
    if rolling then
        local trail_spin = frame * 0.16
        for idx = 1, 6 do
            local trail_pct = (rolling_pct + idx * 0.15) % 1
            local direction = idx % 2 == 0 and -0.72 or 1
            local angle = trail_spin * direction + idx * TWO_PI / 6 + rolling_pct * 2.7
            local dist = 31 + trail_pct * 20 + surge_stage * 1.3
            local trail_roll = ((roll + idx - 2) % 6) + 1
            local trail_xy = {xy[1] + math.cos(angle) * dist, xy[2] + math.sin(angle) * dist * 0.58}
            draw_die_face(trail_xy, trail_roll, 9 + trail_pct * 5, color, 0.14 + trail_pct * 0.24)
        end
    end
    Display.draw_rectangle(alpha(COL_BLACK, rolling and 0.58 or 0.48), box)
    if near_drop then
        local drop_box = {box[1] - 3, box[2] - 3, box[3] + 3, box[4] + 3}
        Display.draw_rectangle_outline(alpha(COL_WHITE, 0.24 + pulse * 0.28), drop_box, 1)
    end
    Display.draw_rectangle_outline(alpha(color, rolling and (0.82 + pulse * 0.16 + tension * 0.12) or 0.78), box, rolling and 2 or 1)
    if rolling then
        local rail_y = box[4] + 9
        for face = 1, 6 do
            local face_x = xy[1] - 25 + (face - 1) * 10
            local locked = face <= lock_stage * 2
            local active = face == math.max(1, roll)
            draw_die_face(
                {face_x, rail_y},
                face,
                active and 8.5 or 6.5,
                active and color or COL_WHITE,
                active and 0.58 or locked and 0.34 or 0.18
            )
        end
        local surge_y = box[2] - 8
        for stage = 1, surge_total do
            local stage_x = xy[1] - (surge_total - 1) * 4.5 + (stage - 1) * 9
            local active = stage <= surge_stage
            Display.draw_circle(
                alpha(active and color or COL_WHITE, active and 0.58 or 0.18),
                {stage_x, surge_y},
                active and 2.6 or 1.7
            )
        end
    end
    local has_die = roll > 0 and label ~= ""
    local text_x = xy[1]
    if has_die then
        local die_size = rolling and (final_lock and 26 or near_drop and 23 or 21) or 18
        if rolling then
            draw_die_face({box[1] + 17 + 3, xy[2] + 3}, roll, die_size, COL_BLACK, 0.38)
        end
        draw_die_face({box[1] + 17, xy[2]}, roll, die_size, color, rolling and 1 or 0.88)
        text_x = xy[1] + (final_lock and 16 or 11)
    end
    if label ~= "" then
        Fonts.small:draw({color = alpha(COL_WHITE, 0.95), origin = Display.CENTER}, {text_x, xy[2] - 7}, "D6 " .. tostring(roll))
        Fonts.small:draw({color = alpha(color, 0.95), origin = Display.CENTER}, {text_x, xy[2] + 7}, label)
    else
        Fonts.small:draw({color = alpha(COL_WHITE, 0.95), origin = Display.CENTER}, {xy[1], xy[2] - 1}, "D6 " .. tostring(roll))
    end
    local outer_radius = rolling and 40 or label ~= "" and 36 or 31
    local outer_pct = rolling and rolling_pct or 1 - pct
    Display.draw_arc(alpha(color, rolling and 0.95 or 0.88), xy, outer_radius, -math.pi / 2, -math.pi / 2 + TWO_PI * outer_pct, rolling and 3 or 2, 18)
    if rolling then
        local spin = (GameState.frame or 0) * 0.19
        Display.draw_arc(alpha(COL_WHITE, 0.26 + pulse * 0.28), xy, outer_radius + 5, spin, spin + math.pi * 0.74, 2, 18)
        Display.draw_arc(alpha(color, 0.20 + pulse * 0.24), xy, outer_radius - 8, -spin * 0.7, -spin * 0.7 + math.pi * 1.18, 2, 18)
        if surge_stage > 0 then
            local surge_pct = surge_stage / surge_total
            Display.draw_arc(alpha(color, 0.26 + surge_pct * 0.44), xy, outer_radius + 10, -math.pi / 2, -math.pi / 2 + TWO_PI * surge_pct, 2, 20)
            Display.draw_arc(alpha(COL_WHITE, 0.16 + surge_pct * 0.20), xy, outer_radius + 14, spin * -0.42, spin * -0.42 + math.pi * (0.38 + surge_pct), 1, 18)
        end
        if near_drop or final_lock then
            local burst_alpha = final_lock and 0.46 or 0.30
            Display.draw_arc(alpha(COL_WHITE, burst_alpha), xy, outer_radius + 18, spin * 0.52, spin * 0.52 + math.pi * 0.86, 2, 20)
            Display.draw_arc(alpha(color, burst_alpha + 0.10), xy, outer_radius + 22, -spin * 0.37, -spin * 0.37 + math.pi * 0.62, final_lock and 3 or 2, 20)
        end
        for idx = 1, 6 do
            local angle = spin + idx * TWO_PI / 6
            local inner = outer_radius - 3
            local outer = outer_radius + 3 + pulse * 2
            local pip_xy = {xy[1] + math.cos(angle) * (outer_radius + 4), xy[2] + math.sin(angle) * (outer_radius + 4)}
            Display.draw_line(
                alpha(color, 0.18 + pulse * 0.18),
                {xy[1] + math.cos(angle) * inner, xy[2] + math.sin(angle) * inner},
                {xy[1] + math.cos(angle) * outer, xy[2] + math.sin(angle) * outer},
                1
            )
            Display.draw_circle(
                alpha(idx <= math.max(1, roll) and color or COL_WHITE, 0.20 + pulse * 0.22),
                pip_xy,
                idx <= math.max(1, roll) and 2.2 or 1.4
            )
        end
        if lock_stage > 0 then
            local locked_pips = math.min(6, lock_stage * 2)
            local lock_radius = outer_radius - 13
            for idx = 1, locked_pips do
                local angle = -math.pi / 2 + idx * TWO_PI / 6 + spin * 0.28
                Display.draw_circle(
                    alpha(color, 0.42 + pulse * 0.28),
                    {xy[1] + math.cos(angle) * lock_radius, xy[2] + math.sin(angle) * lock_radius},
                    2.4 + lock_stage * 0.25
                )
            end
            Display.draw_arc(
                alpha(color, 0.35 + lock_stage / lock_total * 0.38),
                xy,
                outer_radius - 13,
                -math.pi / 2,
                -math.pi / 2 + TWO_PI * (lock_stage / lock_total),
                2,
                12
            )
        end
    end
    if buff_pct > 0 then
        Display.draw_arc(alpha(COL_WHITE, 0.70), xy, rolling and 27 or label ~= "" and 26 or 23, -math.pi / 2, -math.pi / 2 + TWO_PI * buff_pct, 2, 16)
    end
end

return M
