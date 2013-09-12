local CooldownSet = import "@CooldownSet"
local SlotUtils = import "@SlotUtils"
local SpellType = import "@SpellType"

local SpellsKnown = newtype()

function SpellsKnown:init()
    self.spells = {}
end

function SpellsKnown:__copy(other)
    -- Ensure a shallow copy when used with table.deep_copy
    table.copy(self, other, --[[No metacall]] false)
end

function SpellsKnown:add_spell(spell_slot)
    -- Resolve item slot
    if type(spell_slot) == "string" or not getmetatable(spell_slot) then
        if not spell_slot.type then spell_slot = {type = spell_slot} end
        spell_slot = SpellsKnown.spell_slot_create(spell_slot)
    end
    table.insert(self.spells, spell_slot)
end

function SpellsKnown:values()
    return values(self.spells)
end

function SpellsKnown:can_use_spell(stats, spell_slot)
    local C,c = CooldownSet,stats.base.cooldowns
    if c:has_cooldown(C.ALL_ACTIONS) or c:has_cooldown(C.OFFENSIVE_ACTIONS) or
       c:has_cooldown(C.SPELL_ACTIONS) or c:has_cooldown(spell_slot.type) then
        return false
    end
    return spell_slot.type.on_prerequisite(spell_slot, stats)
end

function SpellsKnown:use_spell(stats, spell_slot)
    if self:can_use_spell(stats, spell_slot) then
        local C,c = CooldownSet,stats.base.cooldowns
        local spell = spell_slot.type
        c:add_cooldown(spell, spell.cooldown_self)
        c:add_cooldown(C.OFFENSIVE_ACTIONS, spell.cooldown_offensive)
        c:add_cooldown(C.SPELL_ACTIONS, spell.cooldown_offensive)
        c:add_cooldown(C.ALL_ACTIONS, spell.cooldown_global)
        spell_slot.type.on_use(spell_slot, stats)
        return true
    end
    return false
end

function SpellsKnown.spell_slot_create(args)
    assert(args.type)
    if _G.type(args.type) == "string" then 
        args.type = SpellType.lookup(args.type)
    end
    return setmetatable(args, SlotUtils.METATABLE)
end

return SpellsKnown