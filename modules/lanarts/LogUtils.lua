local EventLog = import "core.ui.EventLog"
local Players = import "@Players"

local M = nilprotect {} -- Submodule

-- Remove start and end character
local function unwrap(str)
    return str:sub(2,#str-1)
end

-- Ignore string
local function toss(str)
    return ""
end

function M.resolve_conditional_message(user, msg)
    local is_player = Players.is_player(user) 
    local is_local_player = Players.is_local_player(user)
    local name = is_local_player and 'You' or user.base_stats.name
    -- Resolve name references
    msg = msg:gsub("$You", name)
    msg = msg:gsub("$you", name:lower())

    -- Non-proper nouns only (eg <the>).
    -- Used for non-unique and
    msg = msg:gsub("(<[^>]+>)", is_player and toss or unwrap)
    -- Third person only (eg work{s})
    msg = msg:gsub("({[^}]+})", is_local_player and toss or unwrap)
    -- First person only (eg [Your])
    msg = msg:gsub("(%[[^%]]+%])", is_local_player and unwrap or toss)
    return msg
end

function M.resolved_log(user, msg, ...)
    EventLog.add(M.resolve_conditional_message(user, msg), ...)
end

-- Logs if it is the current player
function M.log_if_local(user, ...)
    if Players.is_local_player(user) then
        M.resolved_log(user, ...)
        return true
    end
    return false
end

function M.log_if_player(user, ...)
    if Players.is_player(user) then
        M.resolved_log(user, ...)
        return true
    end
    return false
end

return M