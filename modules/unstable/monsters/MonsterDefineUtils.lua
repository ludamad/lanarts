local EventLog = import "core.ui.EventLog"

local Apts = import "@stats.AptitudeTypes"
local StatContext = import "@StatContext"
local MonsterType = import "@MonsterType"
local Stats = import "@Stats"
local Attacks = import "@Attacks"

local Relations = import "lanarts.objects.Relations"
local AptitudeTypes = import "@stats.AptitudeTypes"
local ContentUtils = import "@stats.ContentUtils"

local Traits = import ".MonsterTraits"

local M = nilprotect {} -- Submodule

local function default_on_appear_message(self)
    EventLog.add(self.appear_message, self.appear_color or {255,255,255})
end

local function default_on_die_message(self)
    EventLog.add(self.type.defeat_message, self.defeat_color or {255,255,255})
end

local function on_draw_sprite(self)
    self.sprite:draw(self.xy)
end

function M.monster_define(t, --[[Optional]] derive_idx)
    t.team = t.team or Relations.TEAM_MONSTER_ROOT
    local stats = ContentUtils.resolve_embedded_stats(t)
    local context = StatContext.stat_context_create(stats)
    local sprite = t.sprite or ContentUtils.derive_sprite(t.name, --[[Callers stack index]] (derive_idx or 1) + 1)

    for trait in values(t.traits or {}) do
        Traits.stat_mod_functions[trait](stats)
    end

    if t.unarmed_attack and #t.unarmed_attack > 0 then
        t.unarmed_attack = Attacks.attack_create(unpack(t.unarmed_attack))
    end
    if not t.unarmed_attack then
        local attack_types = t.multipliers or t.types or Apts.MELEE
        t.unarmed_attack = Attacks.attack_create(t.effectiveness or 0,t.damage or 0, attack_types, --[[Optional]] t.delay, --[[Optional]] t.damage_multiplier)

    end
    return MonsterType.define {
        name = t.name,
        description = t.description:pack(),
        on_first_appear = t.on_first_appear or default_on_appear_message,
        on_die = t.on_die or default_on_die_message,

        sprite = sprite,
        on_draw = t.on_draw or on_draw_sprite,
        traits = t.traits,

        xp_award = t.xp_award,
        radius = t.radius,

        stats = stats,
        unarmed_attack = t.unarmed_attack,

    }
end

function M.animal_define(t, --[[Optional]] derive_idx)
    t.traits = t.traits or {}
    table.insert(t.traits, Traits.ANIMAL)
    return M.monster_define(t, (derive_idx or 1) + 1)
end

return M
