local Cooldowns = import "@Cooldowns"

local SpellsKnown = newtype()

function SpellsKnown:init()
    self.spells = {}
end

function SpellsKnown:add_spell(spell_type, --[[Optional]] modifiers)
    local name = (modifiers and modifiers.name) or spell_type.name
    local slot = {type = spell_type, modifiers = modifiers, name = name }
    table.insert(self.spells, slot)
end

function SpellsKnown:values()
    return values(self.spells)
end

function SpellsKnown:can_use_spell(stats, spell_slot)
    local C,c = Cooldowns,stats.base.cooldowns
    if c:has_cooldown(C.ALL_ACTIONS) or c:has_cooldown(C.OFFENSIVE_ACTIONS) or
       c:has_cooldown(C.SPELL_ACTIONS) or c:has_cooldown(spell_slot.type) then
        return false
    end
    return spell_slot.type.on_prerequisite(spell_slot, stats)
end

function SpellsKnown:use_spell(stats, spell_slot)
    if self:can_use_spell(stats, spell_slot) then
        local C,c = Cooldowns,stats.base.cooldowns
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

return SpellsKnown