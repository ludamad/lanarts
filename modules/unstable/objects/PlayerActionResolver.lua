local Keys = import "core.Keyboard"
local Mouse = import "core.Mouse"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local PlayerIOActions = import ".PlayerIOActions"
local ActionResolvers = import ".ActionResolvers"

local M = nilprotect {} -- Submodule

local function add_if_valid(player, actions, action, candidate)
    if PlayerIOActions.can_use_io_action(player, action, candidate) then
        table.insert(actions, {action, candidate})
        return true
    end
    return false
end
local function add_first_valid_candidate(player, actions, action, candidates)
    for c in values(candidates) do
        if add_if_valid(player, actions, action, c) then
            return true
        end
    end
    return false
end

function M.resolve_autoattacks(player, actions)
    if Keys.key_held('h') then 
    end
end

function M.resolve_movement(player, actions)
    local stats = player:stat_context()
    local move_speed = stats.derived.movement_speed

    local dx,dy=0,0

    if Keys.key_held('w') then dy=-1 end
    if Keys.key_held('s') then dy=1 end
    if Keys.key_held('a') then dx=-1 end
    if Keys.key_held('d') then dx=1 end

    -- Return if no movement
    if dx == 0 and dy == 0 then return end

    local candidates = {
        vector_add(player.xy, {dx*move_speed,dy*move_speed}),
        vector_add(player.xy, {0,dy*move_speed}),
        vector_add(player.xy, {dx*move_speed,0})
    }

    add_first_valid_candidate(player, actions, PlayerIOActions.MoveIOAction, candidates)
end


function M.default_io_action_resolver(player, actions)
    M.resolve_movement(player, actions)
end

local Base = ActionResolvers.ActionResolverBase
local Player = newtype {parent = Base}
M.create = Player.create

--------------------------------------------------------------------------------
--                           IO Events                                        --
--------------------------------------------------------------------------------
local function resolve_movement(R, obj, event)
    if event.dx then R.dx = event.dx end
    if event.dy then R.dy = event.dy end
end

local function resolve_action(R, obj, event)
    local action = event.action
    if type(action) == "function" then action = action(obj, event) end
    R.action = action
    R.target = ObjectUtils.find_closest_hostile(obj):stat_context()
end

local io_events = {
    {
        triggers = {'H'}, callback = resolve_action,
        action = function(obj) return obj:weapon_action_context() end
    },
    {triggers = {'w', Keys.UP}, callback = resolve_movement, dy = -1},
    {triggers = {'s', Keys.DOWN}, callback = resolve_movement, dy = 1},
    {triggers = {'a', Keys.LEFT}, callback = resolve_movement, dx = -1},
    {triggers = {'d', Keys.RIGHT}, callback = resolve_movement, dx = 1}
}

for i,key in ipairs {'y', 'u', 'i', 'o', 'p'} do
    table.insert(io_events, {
        triggers = {key}, callback = resolve_action,
        action = function(obj) return obj:spell_action_context(obj, obj.base_stats.spells.spells[i]) end
    })
end

local function trigger_matches(io_event)
    for _, trigger in ipairs(io_event.triggers) do
        if Keys.key_held(trigger) then return true end
    end
    return false
end

function Player:init(cgroup)
    Base.init(self, cgroup)
    self.io_events = io_events 
end

function Player:resolve_action(obj)
    self.dx,self.dy = 0,0

    for _, io_event in ipairs(self.io_events) do
        if trigger_matches(io_event) then
            io_event.callback(self, obj, io_event)
        end
    end

    local speed = obj:speed()
    self.preferred_velocity[1] = self.dx * speed
    self.preferred_velocity[2] = self.dy * speed
end

return M