local Type, Traits = import "@SpellType", import ".SpellTraits"
local Apts = import "@stats.AptitudeTypes"
local ContentUtils = import "@stats.ContentUtils"
local CooldownUtils = import "@stats.CooldownUtils"
local StatusType = import "@StatusType"
local StatContext = import "@StatContext"
local SpellType = import "@SpellType"
local AttackProjectileObject = import "@objects.AttackProjectileObject"

local M = nilprotect {} -- Submodule

M.NOT_ENOUGH_MANA = "You do not have enough mana!"

local function mana_prereq(self, caster)
    if self.mp_cost > caster.base.mp then
        return false, M.NOT_ENOUGH_MANA
    end
    return true
end

local function mana_use(self, caster)
    StatContext.add_mp(caster, -self.mp_cost)
end

function M.base_spell_on_create(spell, args)
    if spell.name then
        args.name = (spell.name):interpolate(args)
    end
    if spell.description then
        args.description = (spell.description):interpolate(args)
    end
    args.type = args.type or spell
    CooldownUtils.resolve_action_cooldown(args)
    return setmetatable(args, {__index = spell})
end

function M.spell_define(args)
    args.on_create = args.on_create or M.base_spell_on_create
    args.on_prerequisite = ContentUtils.combine_on_prereq(mana_prereq, args.on_prerequisite)
    args.on_use = ContentUtils.combine_on_use(mana_use, args.on_use)
    args.cooldown_offensive = args.cooldown_offensive or args.cooldown 

    return SpellType.define(args)
end

local function missile_spell_on_use(self, caster, xy)
    printf "USED"
--    local dir = vector_subtract(xy, caster.xy)
--    dir = vector_normalize(dir, self.speed)
--    AttackProjectileObject.create {
--    }
end

function M.missile_spell_define(args)
    assert(args.speed)

    args.on_use = ContentUtils.combine_on_use(missile_spell_on_use, args.on_use)

    args.target_type = args.target_type or SpellType.TARGET_HOSTILE
    args.traits = args.traits or {Traits.FORCE_SPELL}

    args.types = args.types or {Apts.FORCE} -- For 'derive_attack'
    args.attack = ContentUtils.derive_attack(args)

    return M.spell_define(args)
end

return M