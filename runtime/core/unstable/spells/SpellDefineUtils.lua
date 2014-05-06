local Actions = import "@Actions"
local ContentUtils = import "@stats.ContentUtils"
local ActionUtils = import "@stats.ActionUtils"
local SpellType = import "@SpellType"

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
    args.base_on_create = args.base_on_create or M.base_spell_on_create
    args.on_create = args.on_create or args.base_on_create
    args.sprite = ContentUtils.resolve_sprite(args)
    args.action_use = ActionUtils.derive_action(args.action_use or args, ActionUtils.ALL_ACTION_COMPONENTS, --[[Cleanup members]] true)
    args.aptitude_types = ActionUtils.desirable_user_aptitudes(args.action_use)

    return SpellType.define(args)
end

return M