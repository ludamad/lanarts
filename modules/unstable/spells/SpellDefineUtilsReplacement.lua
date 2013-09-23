local Type, Traits = import "@SpellType", import ".SpellTraits"
local Apts = import "@stats.AptitudeTypes"
local Actions = import "@Actions"
local ContentUtils = import "@stats.ContentUtils"
local CooldownUtils = import "@stats.CooldownUtils"
local StatusType = import "@StatusType"
local StatContext = import "@StatContext"
local SpellType = import "@SpellType"
local AttackProjectileObject = import "@objects.AttackProjectileObject"

local M = nilprotect {} -- Submodule

local SPELL_META = {
    __index = function (t,k) return assert(rawget(t, "type"))[k] end,
    __copy = function(t1,t2) table.copy(t1,t2, --[[Do not invoke meta]] false) end
}

function M.base_spell_on_create(spell, args)
    if spell.name then
        args.name = (spell.name):interpolate(args)
    end
    if spell.description then
        args.description = (spell.description):interpolate(args)
    end
    args.type = args.type or spell
    return setmetatable(args, SPELL_META)
end

function M.spell_define(args)
    args.on_create = args.on_create or M.base_spell_on_create
    args.cooldown_offensive = args.cooldown_offensive or args.cooldown
    args.use_action = 

    return SpellType.define(args)
end

function M.missile_spell_define(args)
    
end

return M