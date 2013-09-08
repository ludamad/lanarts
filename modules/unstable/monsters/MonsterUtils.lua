local EventLog = import "core.ui.EventLog"

local StatContext = import "@StatContext"
local MonsterType = import "@MonsterType"
local Stats = import "@Stats"
local Attacks = import "@Attacks"

local Relations = import "lanarts.objects.Relations"
local AptitudeTypes = import "@stats.AptitudeTypes"
local ContentUtils = import "@stats.ContentUtils"

local Traits = import ".MonsterTraits"

local M = nilprotect {} -- Submodule

function M.appear_message(msg, --[[Optional]] color)
    return function(t)
        t.on_first_appear = function()
            EventLog.add(msg, color or {255,255,255})
        end
    end
end

function M.defeat_message(msg, --[[Optional]] color)
    return function(t)
        t.on_die = function()
            EventLog.add(msg, color or {255,255,255})
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
    local sprite = ContentUtils.derive_sprite(t.name, --[[Callers stack index]] 2)

    for type,v in pairs(t.aptitudes or {}) do
        StatContext.add_all_aptitudes(context, type, v, true)
    end

    for trait in values(t.traits or {}) do
        Traits.stat_mod_functions[trait](stats)
    end

    if t.unarmed_attack and #t.unarmed_attack > 0 then
        t.unarmed_attack = Attacks.attack_create(unpack(t.unarmed_attack))
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
        unarmed_attack = t.unarmed_attack
    }
end

return M
