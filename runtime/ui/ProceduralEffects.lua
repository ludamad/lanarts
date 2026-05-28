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

function M.draw_die_badge(obj, args)
    if not obj or obj.destroyed or not Map.object_visible(obj) then
        return
    end
    args = args or {}
    local roll = args.roll or 0
    local remaining = math.max(0, args.remaining or 0)
    local interval = math.max(1, args.interval or 1)
    local pct = remaining / interval
    local color = args.color or COL_PALE_BLUE
    local xy = screen_xy({obj.x, obj.y - (obj.radius or 14) - 30})
    local w = 48
    local h = 20
    local box = {xy[1] - w / 2, xy[2] - h / 2, xy[1] + w / 2, xy[2] + h / 2}
    Display.draw_radial_gradient({120, 170, 255, 35}, {20, 30, 70, 0}, xy, 34, 5)
    Display.draw_rectangle(alpha(COL_BLACK, 0.48), box)
    Display.draw_rectangle_outline(alpha(color, 0.78), box, 1)
    Fonts.small:draw({color = alpha(COL_WHITE, 0.95), origin = Display.CENTER}, {xy[1], xy[2] - 1}, "D6 " .. tostring(roll))
    Display.draw_arc(alpha(color, 0.88), xy, 31, -math.pi / 2, -math.pi / 2 + TWO_PI * (1 - pct), 2, 18)
end

return M
