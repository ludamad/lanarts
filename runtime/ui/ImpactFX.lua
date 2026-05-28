local Display = require "core.Display"
local GameObject = require "core.GameObject"
local ProceduralEffects = require "ui.ProceduralEffects"

local M = {}

local WORLD_DEPTH = -230
local SCREEN_DEPTH = -260

local function copy_xy(xy)
    return {xy[1], xy[2]}
end

local function object_map(args)
    return args and (args.map or (args.obj and args.obj.map))
end

function M.world_burst(args)
    args = args or {}
    local map = object_map(args)
    if not map or not args.xy then
        return nil
    end
    return GameObject.object_create {
        map = map,
        xy = copy_xy(args.xy),
        radius = 1,
        solid = false,
        depth = args.depth or WORLD_DEPTH,
        elapsed = 0,
        duration = args.duration or 36,
        burst_radius = args.radius or 72,
        color = args.color or COL_WHITE,
        label = args.label or "",
        drift = args.drift or 0.45,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if self.elapsed >= self.duration then
                GameObject.destroy(self)
            end
        end,
        on_draw = function(self)
            local fade = math.max(0, 1 - self.elapsed / math.max(1, self.duration))
            ProceduralEffects.draw_world_burst {
                xy = self.xy,
                frame = self.elapsed,
                duration = self.duration,
                radius = self.burst_radius,
                color = self.color,
            }
            if self.label ~= "" then
                local screen_xy = Display.to_screen_xy({
                    self.xy[1],
                    self.xy[2] - self.burst_radius * 0.45 - self.elapsed * self.drift,
                })
                Fonts.small:draw({color = with_alpha(self.color, fade), origin = Display.CENTER}, screen_xy, self.label)
            end
        end,
    }
end

function M.screen_flash(args)
    args = args or {}
    local map = object_map(args)
    if not map then
        return nil
    end
    return GameObject.object_create {
        map = map,
        xy = copy_xy(args.xy or {0, 0}),
        radius = 1,
        solid = false,
        depth = args.depth or SCREEN_DEPTH,
        elapsed = 0,
        duration = args.duration or 16,
        color = args.color or COL_WHITE,
        alpha = args.alpha or 0.22,
        edge_only = args.edge_only ~= false,
        on_step = function(self)
            self.elapsed = self.elapsed + 1
            if self.elapsed >= self.duration then
                GameObject.destroy(self)
            end
        end,
        on_draw = function(self)
            local fade = math.max(0, 1 - self.elapsed / math.max(1, self.duration))
            local alpha = self.alpha * fade
            if self.edge_only then
                ProceduralEffects.draw_screen_edges {
                    color = self.color,
                    alpha = alpha,
                    thickness = 28,
                    passes = 4,
                }
            else
                local w, h = unpack(Display.display_size)
                Display.draw_rectangle(with_alpha(self.color, alpha), {0, 0, w, h})
            end
        end,
    }
end

function M.rescue_save(args)
    args = args or {}
    local map = args.map or (args.strike and (args.strike.strike_map or args.strike.map))
    if not map then
        return
    end
    local color = args.color or COL_PALE_GREEN
    local label = args.label or "SAVE"
    local radius = args.radius or 78
    M.screen_flash {
        map = map,
        color = color,
        alpha = args.flash_alpha or 0.24,
        duration = args.flash_duration or 18,
    }
    if args.blocker then
        M.world_burst {
            map = map,
            xy = args.blocker.xy,
            color = color,
            label = label,
            radius = radius,
            duration = args.duration or 42,
        }
    end
    if args.target then
        M.world_burst {
            map = map,
            xy = args.target.xy,
            color = args.target_color or COL_PALE_BLUE,
            label = args.target_label or "PULLED",
            radius = radius * 0.74,
            duration = args.duration or 42,
        }
    end
end

function M.rescue_miss(args)
    args = args or {}
    local map = args.map or (args.strike and (args.strike.strike_map or args.strike.map))
    local target = args.target or (args.strike and args.strike.target)
    if not map or not target then
        return
    end
    M.screen_flash {
        map = map,
        color = COL_PALE_RED,
        alpha = 0.18,
        duration = 16,
    }
    M.world_burst {
        map = map,
        xy = target.xy,
        color = COL_PALE_RED,
        label = "ALLY HIT",
        radius = args.radius or 68,
        duration = 36,
    }
end

return M
