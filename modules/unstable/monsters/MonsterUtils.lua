local EventLog = import "core.ui.EventLog"

local StatContext = import "@StatContext"
local MonsterType = import "@MonsterType"
local Stats = import "@Stats"

local Relations = import "lanarts.objects.Relations"
local AptitudeTypes = import "@content.aptitude_types"

local Traits = import ".monster_traits"
local Utils = import ".monster_utils"


local M = nilprotect {} -- Submodule

function M.appear_message(msg)
    return function(t)
        t.on_first_appear = function()
            EventLog.add(msg, {255,255,255})
        end
    end
end

function M.defeat_message(msg)
    return function(t)
        t.on_die = function()
            EventLog.add(msg, {255,255,255})
        end
    end
end

local function on_draw_sprite(self)
    self.type.sprite:draw(self.xy)
end

function M.monster_define(t)
    t.stats.team = t.stats.team or Relations.TEAM_MONSTER_ROOT
    local stats = Stats.stats_create(t.stats)
    local context = StatContext.stat_context_create(stats)
    local sprite_name = t.name:gsub(' ', '_'):lower() .. ".png"
    local sprite = ContentUtils.derive_sprite(name, --[[Callers stack index]] 2)

    for type,v in pairs(t.aptitudes or {}) do
        local eff,dam,res,def = unpack(v)
        StatContext.add_effectiveness(context, type, eff or 0, --[[Permanent]] true)
        StatContext.add_damage(context, type, dam or 0, --[[Permanent]] true)
        StatContext.add_resistance(context, type, res or 0, --[[Permanent]] true)
        StatContext.add_defence(context, type, def or 0, --[[Permanent]] true)
    end

    for trait in values(t.traits or {}) do
        Traits.stat_mod_functions[trait](stats)
    end

    return MonsterType.define {
        M.appear_message(t.appear_message),
        M.defeat_message(t.defeat_message),
        name = t.name,
        description = t.description,
        traits = t.traits,
        stats = stats,
        xp_award = t.xp_award,
        radius = t.radius,
        sprite = sprite,
        on_draw = on_draw_sprite,
        attacks = t.attacks
    }
end

return M