local EventLog = require "ui.EventLog"
local GameObject = require "core.GameObject"
local Map = require "core.Map"

local M = nilprotect {} -- Submodule

-- MINOR MISSILE

Data.spell_create {
    name = "Minor Missile",
    spr_spell = "minor missile",
    description = "A low cost, fast bolt of energy. Hits a single target. The bolt can bounce off walls safely.",
    projectile = "Minor Missile",
    mp_cost = 4,
    cooldown = 35
}

-- FIRE BOLT

Data.spell_create {
    name = "Fire Bolt",
    spr_spell = "fire bolt",
    description = "A fast bolt of fire. Hits a single target.",
    projectile = "Fire Bolt",
    mp_cost = 10,
    cooldown = 35
}

-- POISON CLOUD

Data.spell_create {
    name = "Poison Cloud",
    spr_spell = "poison",
    description = "A poisonous cloud that causes damage over time.",
    projectile = "Poison Cloud",
    mp_cost = 25,
    cooldown = 60
}

-- BERSERK

local Berserk = {
    name = "Berserk",
    spr_spell = "berserk",
    description = "Allows you to strike powerful blows for a limited duration, afterwards you are slower and vulnerable.",
    mp_cost = 40,
    cooldown = 30,
    can_cast_with_held_key = false,
    fallback_to_melee = false
}

function Berserk.prereq_func(caster)
    return not caster:has_effect("Berserk") and not caster:has_effect("Exhausted")
end

function Berserk.autotarget_func(caster)
    return caster.x, caster.y
end

function Berserk.action_func(caster, x, y)
    caster:add_effect("Berserk", 150 + math.min(4, caster.stats.level)  * 20)
    if caster:is_local_player() then
        EventLog.add("You enter a powerful rage!", {200,200,255})
    elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " enters a powerful rage!", {200,200,255})
    end
end

Data.spell_create(Berserk)

-- BLINK

local Blink = {
    name = "Blink",
    spr_spell = "blink",
    description = "Takes you instantly across a short distance.",
    mp_cost = 50,
    cooldown = 50,
    autotarget_func = spell_choose_safest_square,
    can_cast_with_held_key = false,
    can_cast_with_cooldown = true,
    fallback_to_melee = false
}

function Blink.prereq_func(caster, x, y)
    local xy = {x,y}
    return Map.radius_visible(xy, caster.radius) and not Map.object_collision_check(caster, xy)
end

function Blink.action_func(caster, x, y)
    caster.xy = {x,y}
end

Data.spell_create(Blink)

-- MAGIC ARROW

local MagicArrow = {
    name = "Magic Arrow",
    description = "Allows you to create an arrow of pure energy, requires a bow. Does well against strudy opponents.",
    spr_spell = "magic arrow",
    projectile = "Magic Arrow",
    mp_cost = 15,
    cooldown = 25,
    resist_modifier = 0.5
}

function MagicArrow.prereq_func(caster)
    return caster.stats.weapon_type == "bows"
end

Data.spell_create(MagicArrow)

-- MAGIC BLAST

Data.spell_create {
    name = "Magic Blast",
    description = "A slow, powerful magic blast of energy. The blast can bounce off an enemy twice before dissipating.",
    spr_spell = "magic blast",
    projectile = "Magic Blast",
    mp_cost = 20,
    cooldown = 65
}

-- POWER STRIKE

local PowerStrike = {
    name = "Power Strike",
    description = "A powerful charged strike. Deals stunning blows to all surrounding enemies, knocking them back.",
    can_cast_with_held_key = true,
    spr_spell = "chargestrike",
    can_cast_with_cooldown = false,
    mp_cost = 40,
    cooldown = 0, -- Uses cooldown of weapon
    fallback_to_melee = true,
}

local function ChargeCallback(_, caster)
    local num = 0
    for mon in values(Map.monsters_list() or {}) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + caster.target_radius + 30 + caster.stats.strength then
            num = num + 1
            caster:melee(mon)
            --            local chance = math.max(25, 100 - num * 20)
            --if rand_range(0, 100) < chance then -- decreasing chance of knockback
            local str_diff = math.max(0, caster.stats.strength - mon.stats.strength)
                thrown = mon:add_effect("Thrown", 10 + 10 * str_diff)
                thrown.angle = vector_direction({caster.x, caster.y}, {mon.x, mon.y})
                if caster:is_local_player() then
                    EventLog.add("The " .. mon.name .." is thrown back!", {200,200,255})
                end
            --end
        end
    end
end

function PowerStrike.action_func(caster)
    caster:apply_melee_cooldown()
    if caster:is_local_player() then
        EventLog.add("You strike wildly in all directions!", {200,200,255})
    elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " strikes wildly in all directions!", {200,200,255})
    end

    caster:add_effect("Charge", 8).callback = ChargeCallback
end

function PowerStrike.prereq_func(caster)
    if caster:has_ranged_weapon() then
        return false
    end
    for mon in values(Map.monsters_list() or {}) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + caster.target_radius + 30 then
            return true
        end
    end
    return false
end

function PowerStrike.autotarget_func(caster)
    return caster.x, caster.y
end

Data.spell_create(PowerStrike)

-- EXPEDITE --

local Expedite = {
    name = "Expedite",
    description = "Run faster for a short duration.",
    can_cast_with_held_key = false,
    spr_spell = "expedite",
    can_cast_with_cooldown = true,
    mp_cost = 25,
    cooldown = 30,
    fallback_to_melee = false,
}

function Expedite.prereq_func(caster)
    return not caster:has_effect("Expedited")
end

function Expedite.autotarget_func(caster)
    return caster.x, caster.y
end

function Expedite.action_func(caster, x, y)
    caster:add_effect("Expedited", 150)
    if caster:is_local_player() then
        EventLog.add("You feel expedient!", {200,200,255})
    elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " seems expedient.", {200,200,255})
    end
end

Data.spell_create(Expedite)

return M
