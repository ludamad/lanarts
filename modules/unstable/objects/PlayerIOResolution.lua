local Keys = import "core.Keyboard"
local Mouse = import "core.Mouse"
local PlayerIOActions = import ".PlayerIOActions"

local M = nilprotect {} -- Submodule

local function add_if_valid(player, actions, action, candidate)
    if PlayerIOActions.can_use_action(player, action, candidate) then
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

function M.resolve_movement(player, actions)
    local stats = player:stat_context()
    local move_speed = stats.derived.movement_speed

    local dx,dy=0,0

    if Keys.key_pressed('w') then dy=-1 end
    if Keys.key_pressed('s') then dy=1 end
    if Keys.key_pressed('a') then dx=-1 end
    if Keys.key_pressed('d') then dx=1 end

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

return M