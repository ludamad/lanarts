local EventLog = import "ui.EventLog"
local Players = import "@Players"
local AnsiColors = import "terminal.AnsiColors"

local M = nilprotect {} -- Submodule

-- TODO: Turn on/off logging per module in some efficient manner
-- Initializing a logger is possible... However, a special version of LogUtils for each module may be better.
__LANARTS_DEBUG_MODE = true
local DEBUG_MESSAGE_PREFIX = "DEBUG: "
-- TODO Colorize logging from different sections
local DEBUG_COLOR = AnsiColors.YELLOW

-- Remove start and end character
local function unwrap(str)
    return str:sub(2,#str-1)
end

-- Ignore string
local function toss(str)
    return ""
end

function M.set_debug_mode(debug_mode)
    rawset(_G, "__LANARTS_DEBUG_MODE", debug_mode)
end

function M.get_debug_mode(debug_mode)
    return __LANARTS_DEBUG_MODE
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

function M.event_log_resolved(user, msg, ...)
    EventLog.add(M.resolve_conditional_message(user, msg), ...)
end

function M.debug_log(msg, ...)
    if not __LANARTS_DEBUG_MODE then return end
    AnsiColors.print(DEBUG_MESSAGE_PREFIX, AnsiColors.WHITE, AnsiColors.BOLD .. AnsiColors.UNDERLINE)
    AnsiColors.println(msg .. table.concat{...}, DEBUG_COLOR, AnsiColors.UNDERLINE)
end

function M.debug_log_resolved(user, msg, ...)
    if not __LANARTS_DEBUG_MODE then return end
    M.debug_log(M.resolve_conditional_message(user, msg .. table.concat{...}))
end

-- Logs if it is the current player
function M.event_log_local(user, ...)
    if Players.is_local_player(user) then
        M.event_log_resolved(user, ...)
        return true
    end
    return false
end

function M.event_log_player(user, ...)
    if Players.is_player(user) then
        M.event_log_resolved(user, ...)
        return true
    end
    return false
end

return M