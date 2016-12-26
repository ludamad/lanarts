local EventLog = require "ui.EventLog"
local GameObject = require "core.GameObject"
local Map = require "core.Map"
local Bresenham = require "core.Bresenham"
local SpellObjects = require "objects.SpellObjects"

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
    name = "Fireball",
    spr_spell = "fire ball",
    description = "A great, strong bolt of fire. Hits a single target.",
    projectile = "Fireball",
    mp_cost = 30,
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
    name = "Mephitize",
    spr_spell = "spr_spells.cloud",
    description = "A noxious debilitating ring of clouds that cause damage as well as reduced defenses and speed over time.",
    projectile = "Mephitize",
    mp_cost = 30,
    cooldown = 35
}

-- FEAR CLOUD

Data.spell_create {
    name = "Trepidize",
    spr_spell = "spr_spells.cause_fear",
    description = "An insidious apparition that instills the fear of death in enemies it hits.",
    projectile = "Trepidize",
    mp_cost = 40,
    cooldown = 35
}

-- REGENERATION

local Regeneration = {
    name = "Regeneration",
    spr_spell = "spr_spells.regeneration",
    description = "Regenerate health 20x for two seconds.",
    mp_cost = 35,
    cooldown = 40,
    can_cast_with_held_key = false,
    fallback_to_melee = false
}

function Regeneration.prereq_func(caster)
    return not caster:has_effect("Regeneration") and not caster:has_effect("Exhausted")
end

function Regeneration.autotarget_func(caster)
    return caster.x, caster.y
end

function Regeneration.action_func(caster, x, y)
    caster:add_effect("Regeneration", 60 * 2)
    if caster:is_local_player() then
        EventLog.add("You start to regenerate quickly!", {200,200,255})
    else
        EventLog.add(caster.name .. " starts to regenerate quickly!", {200,200,255})
    end
end

Data.spell_create(Regeneration)


-- BERSERK

local Berserk = {
    name = "Berserk",
    spr_spell = "berserk",
    description = "Initiate frenzy, gaining +2 defence, +5 willpower, +{Level} strength, +60% melee speed, +25% move speed. Killing enemies grants you longer frenzy, and heals 20HP per kill. Afterwards, become exhausted with -3 defence, -3 willpower, -25% action speed, -50% move speed.",
    mp_cost = 50,
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

-- FEAR STRIKE --

local FearStrike = {
    name = "Fear Strike",
    description = "Strike an enemy, magically instilling the fear of death within them.",
    can_cast_with_held_key = true,
    spr_spell = "spr_spells.fear_strike",
    can_cast_with_cooldown = false,
    mp_cost = 40,
    cooldown = 0, -- Uses cooldown of weapon
    fallback_to_melee = true,
}

function FearStrike.action_func(caster, x, y)
    caster:apply_melee_cooldown()
    local num = 0
    local closest_mon, least_dist = nil, math.huge
    for mon in values(Map.monsters_list() or {}) do
        local dist = vector_distance({mon.x, mon.y}, {caster.x, caster.y})
        if dist < mon.target_radius + caster.target_radius + caster.weapon_range + 8 and least_dist > dist then
            least_dist = dist
            closest_mon = mon
        end
    end
    if not closest_mon then
        return
    end
    caster:melee(closest_mon)
    closest_mon:add_effect("Fear", 100)
    if caster:is_local_player() then
        EventLog.add("You strike into the soul of your enemy!", {200,200,255})
    elseif caster.name == "Your ally" then
        EventLog.add(caster.name .. " strikes into the soul of their enemy!", {200,200,255})
    end
end

function FearStrike.prereq_func(caster)
    if caster:has_ranged_weapon() then
        return false
    end
    for mon in values(Map.monsters_list() or {}) do
        if vector_distance({mon.x, mon.y}, {caster.x, caster.y}) < mon.target_radius + caster.target_radius + caster.weapon_range + 8 then
            return true
        end
    end
    return false
end

function FearStrike.autotarget_func(caster)
    return caster.x, caster.y
end

Data.spell_create(FearStrike)

-- EXPEDITE --

local Expedite = {
    name = "Expedite",
    description = "Run 25% faster for a short duration, with 33% faster rate of fire.",
    can_cast_with_held_key = false,
    spr_spell = "expedite",
    can_cast_with_cooldown = false,
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

-- Wallanthor --

local Wallanthor = {
    name = "Wallanthor",
    description = "Creates a temporary wall of pure energy in a line.",
    can_cast_with_held_key = false,
    spr_spell = "spr_spells.spell-wall",
    can_cast_with_cooldown = false,
    mp_cost = 10,
    cooldown = 35,
    fallback_to_melee = false,
}

function Wallanthor.prereq_func(caster)
    return true
end

function Wallanthor.autotarget_func(caster)
    local dx, dy = unpack(caster.last_moved_direction)
    return caster.x + dx * 32, caster.y + dy * 32
end

function Wallanthor.action_func(caster, x, y)
    if caster:is_local_player() then
        EventLog.add("You create a wall of pure energy!", {200,200,255})
    else 
        EventLog.add(caster.name .. " creates a wall of pure energy!", {200,200,255})
    end

    local dx, dy = x - caster.x, y - caster.y
    local mag = math.sqrt(dx*dx+dy*dy)
    dx, dy = dx / mag, dy / mag
    local start = {math.floor(caster.x / 32), math.floor(caster.y / 32)}
    local finish = {math.floor(caster.x / 32 + dx * 7), math.floor(caster.y / 32 + dy * 7)}
    local points = Bresenham.line_evaluate(start, finish)
    for i=1,#points do
        for j=1,2 do
            points[i][j] = points[i][j] * 32 + 16
        end
    end
    local tx, ty = math.round((caster.x+dx*24) / 32) * 32 + 16, math.round((caster.y+dy*24) / 32) * 32 + 16
    for i=2,#points do
        local obj = SpellObjects.SpellWall.create { points = points, point_index = i }
        GameObject.add_to_level(obj)
    end
end

Data.spell_create(Wallanthor)

return M
