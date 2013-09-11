local Apts = import "@stats.AptitudeTypes"
local SpellsKnown = import "@SpellsKnown"
local StatContext = import "@StatContext"
local Spells = import "@Spells"
local Stats = import "@Stats"

local M = nilprotect {} -- Submodule

-- Derive sprite from name
function M.derive_sprite(name, --[[Optional]] stack_idx)
    stack_idx = (stack_idx or 1) + 1
    local path = path_resolve("sprites/" .. name:gsub(' ', '_'):lower() .. ".png", stack_idx)
    return image_cached_load(path)
end

-- Resolves bonuses of the form [Apts.FOOBAR] = {0,1,1,0}
-- or [Apts.FOOBAR] = 2, equivalent to {2,2,2,2}
-- Expects a mutable 'table' that is being used to define a resource type
-- It will remove the 'convenience variables' used to define the aptitudes.
function M.resolve_aptitude_bonuses(table, --[[Optional]] result)
    result = result or {effectiveness={},damage={},resistance={},defence={}}
    for key,val in pairs(table) do
        if rawget(Apts, key) then
            local eff,dam,res,def
            if type(val) == 'number' then
                eff,dam,res,def = val,val,val,val
            else
                assert(#val == 4)
                eff,dam,res,def = unpack(val)
            end

            -- Add the bonus
            local curr_eff, curr_dam = result.effectiveness[key] or 0, result.damage[key] or 0 
            local curr_res, curr_def = result.resistance[key] or 0, result.defence[key] or 0
            result.effectiveness[key], result.damage[key] = curr_eff + eff, curr_dam + dam 
            result.resistance[key], result.defence[key] = curr_res + res, curr_def + def
 
            table[key] = val -- Cleanup the aptitude convenience variable
        end
    end
    return result
end

-- Expects a mutable 'table' that is being used to define a resource type
-- It will remove the 'convenience variables' used to define the aptitudes.
function M.resolve_embedded_stats(table)
    local aptitudes = M.resolve_aptitude_bonuses
    local spells = table.spells
    if not getmetatable(table.spells) == SpellsKnown then
        spells = SpellsKnown.create()
        for spell in values(table.spells) do
            if type(spell) == "table" and not spell.id then
                spell = Spells.define(spell)
            end 
            spells:add_spell(spell)
        end
    end
    table.spells = spells
    return Stats.stats_create(table)
end

return M