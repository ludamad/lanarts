--- Define stats and cooldown prerequisites. 
-- These are used to help define actions, which consist of a list of prerequisites and effects. 
-- Prerequisites consist of a check() method, which returns true/false as well as the reason for failure (string).

local StatContext = import "@StatContext"
local StatusType = import "@StatusType"
local ItemType = import "@ItemType"

local M = nilprotect {} -- Submodule

------ GENERIC STAT PREREQ (>= stat prereq) ------------------------
M.StatPrereq = newtype()
local function resolve_stat_prereq(prereqs, derived_stats)
    for k,v in pairs(prereqs) do
        if type(v) == "table" then
            local is_ok, problem = resolve_stat_prereq(v, derived_stats[k])
            if not is_ok then
                return false, problem
            end
        end
        -- Special case for HP! Note this assumes 'hp' isn't used anywhere other than main stat table.
        if k == "hp" and derived_stats[k] < v + 1 then
            return false, "That would kill you!"
        end
        if derived_stats[k] < v then
            return false, "You do not have enough " .. k .. "." -- TODO Fix for awkward cases
        end
    end
    return true
end
function M.StatPrereq:init(stat_prereqs) 
    -- Takes mapping stat -> prereq. Can take nested tables.
    self.stat_prereqs = stat_prereqs
end
function M.StatPrereq:check(user) 
    return resolve_stat_prereq(self.stat_prereqs, user.derived)
end

------ GENERIC COOLDOWN REQUIREMENT (<= 0) ------------------------
M.CooldownPrereq = newtype()
function M.CooldownPrereq:init(cooldown_prereq_list) 
    -- Takes list of required cooldowns
    self.cooldown_prereq_list = cooldown_prereq_list
end
function M.CooldownPrereq:check(user)
    for c in values(self.cooldown_prereq_list) do
        if StatContext.has_cooldown(user, c) then
            return false, "You must wait before doing that! ["..c.."]"
        end
    end
    return true
end

------ DISTANCE REQUIREMENT ---------------------------------------
-- Note: Able to take both xy and obj targets!
M.DistancePrereq = newtype()
function M.DistancePrereq:init(max_dist, --[[Optional]] min_dist)
    assert(type(max_dist) == 'number') 
    -- Takes list of required cooldowns
    self.max_dist = max_dist or math.huge
    self.min_dist = min_dist or -math.huge
end

function M.DistancePrereq:check(user, target)
    local target_xy, target_radius 
    -- Assumption: Target is either a position or a StatContext
    if is_position(target) then 
        target_xy, target_radius = target, 0
    else 
        target_xy, target_radius = target.obj.xy, target.obj.radius 
    end

    -- Compare only at the tip of the object(s)
    local reach = user.obj.radius + target_radius
    local distance = vector_distance(user.obj.xy, target_xy) - reach
    if distance < self.min_dist then
        return false, "You are too close!"
    end
    if distance > self.max_dist then
        return false, "You cannot reach!"
    end 

    return true
end

------ STATUS EFFECT REQUIREMENT ----------------------------------
M.UserStatusPrereq = newtype()
function M.UserStatusPrereq:init(forbidden, required)
    -- Resolve statuses referred to by name
    forbidden = forbidden and table.clone(forbidden) or {}
    required = required and table.clone(required) or {}
    for i=1,#forbidden do forbidden[i] = StatusType.resolve(forbidden[i]) end
    for i=1,#required do required[i] = StatusType.resolve(required[i]) end
    self.forbidden, self.required = forbidden, required
end

function M.UserStatusPrereq:check(user)
    for f in values(self.forbidden) do
        if StatContext.get_status(user, f) then 
            return false, "You cannot do this in a " .. f.name:lower() .. " state!" 
        end
    end
    for r in values(self.required) do
        if not StatContext.get_status(user, r) then 
            return false, "You must do this in a " .. r.name:lower() .. " state!"
        end
    end
    return true
end

------ EQUIPMENT WORN REQUIREMENT ----------------------------------
M.EquipmentPrereq = newtype()
function M.EquipmentPrereq:init(slot, categorical_name, --[[Optional]] type, --[[Optional]] trait, --[[Optional]] amount)
    self.slot = slot
    self.categorical_name = categorical_name
    self.type = type and ItemType.resolve(type) or false
    self.trait = trait or false
    self.amount = amount or 1
end

function M.EquipmentPrereq:check(user)
    local item = StatContext.get_equipped_item(user, self.slot)
    local notrait = self.trait and item and not table.contains(item.traits, self.trait)  
    local notype = self.type and item and not item.type == self.type  
    if not item or notrait or notype then
        return false, "You need to have " .. self.categorical_name .. " equipped!"
    end
    if item.amount < self.amount then
        return false, "You do not have enough " .. self.categorical_name .. " equipped!"
    end
    return true
end

return M