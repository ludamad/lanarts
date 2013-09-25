local Type, Traits = import "@SpellType", import ".SpellTraits"
local Apts = import "@stats.AptitudeTypes"
local ActionUtils = import "@stats.ActionUtils"
local Actions = import "@Actions"
local ContentUtils = import "@stats.ContentUtils"
local CooldownUtils = import "@stats.CooldownUtils"
local ActionUtils = import "@stats.ActionUtils"
local StatusType = import "@StatusType"
local StatContext = import "@StatContext"
local SpellType = import "@SpellType"
local AttackProjectileObject = import "@objects.AttackProjectileObject"

local M = nilprotect {} -- Submodule

function M.base_spell_on_create(spell, args)
    if spell.name then
        args.name = (spell.name):interpolate(args)
    end
    if spell.description then
        args.description = (spell.description):interpolate(args)
    end
    args.type = args.type or spell
    return setmetatable(args, ContentUtils.RESOURCE_METATABLE)
end

function M.spell_define(args)
    args.on_create = args.on_create or M.base_spell_on_create
    args.sprite = ContentUtils.resolve_sprite(args)
    local action_args = args.action_use or args
    args.action_use = ActionUtils.derive_action(action_args, --[[Cleanup members]] true)
    args.on_use = function(self, caster, target)
        return Actions.use_action(caster, self.action_use, target)
    end
    args.on_prerequisite = function(self, caster, target)
        return Actions.can_use_action(caster, self.action_use, target)
    end
    args.aptitude_types = ActionUtils.desirable_user_aptitudes(args.action_use)
    args.base_on_create = M.base_spell_on_create

    return SpellType.define(args)
end

local function missile_apply(self, user, target_xy) 
    return M.missile_create(self, user, target_xy) 
end

return M