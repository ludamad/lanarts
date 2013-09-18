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
    if type(spell_slot) == "string" then 
        spell_slot = SpellType.lookup(spell_slot) 
    end
    if not getmetatable(spell_slot) then
        if not spell_slot.type then spell_slot = {type = spell_slot} end
        spell_slot = spell_slot.type:on_create(spell_slot)
    end
    table.insert(self.spells, spell_slot)
end

function SpellsKnown:values()
    return values(self.spells)
end

-- Returns message on failure
function SpellsKnown:can_use_spell(stats, spell_slot)
    return spell_slot.type.on_prerequisite(spell_slot, stats)
end

function SpellsKnown:use_spell(stats, spell_slot)
    if self:can_use_spell(stats, spell_slot) then
        spell_slot.type.on_use(spell_slot, stats)
        return true
    end
    return false
end

return SpellsKnown