--------------------------------------------------------------------------------
-- Installs a better debug.traceback, with color highlighting and filtered noise.
--------------------------------------------------------------------------------

local AnsiColors -- Lazy imported

local LUAFILE_PATTERN = "core/[^%.]*%.lua"
local LUAMODULE_PATTERN = "[%.%w_]+%.%u%w*"

local function modulestart(s)
    -- Search for space, followed by any root package name.
    return ("%s*%w+/" .. s)
end

-- Configuration
local M -- Forward declare for inner functions
M = {
    filter_patterns = {
        -- Lines to delete starting with this line and going up
        [modulestart "ErrorReporting%.lua"] = 1,
        [modulestart "GlobalVariableLoader%.lua:.*'__index'"] = 1,
        [modulestart "ModuleSystem%.lua:%s+:.*'import.*'"] = 2,
        [modulestart "globals/General%.lua:.*'errorf'"] = 2,
        [modulestart "Main.lua"] = 1,
        [modulestart "TestRunner.lua:[^']+'main'"] = 1,
        ["%s*%[C%]: in function 'error'"] = 4
    },
    
    stacktrace_replacements = {
        {"stack traceback:", function(s) return M.resolve_color("WHITE", s) end}
    },
    
    error_replacements = {
        {"core/core/globals/Modules%.lua:%d+: ", ""},
        {LUAMODULE_PATTERN, function(s) return M.resolve_color("WHITE", s) end},
--        {"'[_%w]+'", function(s) return M.resolve_color("WHITE", s) end},
        {'%d+', function(s) return M.resolve_color("WHITE", s) end},
        {LUAFILE_PATTERN, function(s) return M.resolve_color("WHITE", s) end}
    },
    
    virtual_paths = true, 
    use_color = true,
    context = 0
}

local DOT_LINE = "--------------------------------------------------------------------------------"
function M.resolve_context(fpath, line_num, context)
    local file = io.open(fpath, "r");
    local arr = {}
    if not file then 
        return arr
    end
    local i,min_i,max_i = 1,line_num-context,line_num+context
    -- Find lines within [min_i, max_i]
    for line in file:lines() do
        if i == line_num then
            table.insert(arr, M.resolve_color("WHITE", ">> " .. line, ";1"))
        elseif i >= min_i and i <= max_i then
            table.insert(arr, M.resolve_color("YELLOW", ".. " .. line, ";2"))
        end
        i = i + 1
    end
    table.insert(arr, M.resolve_color("YELLOW", DOT_LINE, ";2"))

    return arr
end

function M.resolve_color(col, str, params)
    if M.use_color and AnsiColors then return AnsiColors[col](str, params) end
    return str 
    
end
local function resolve_path(path)
    if not M.virtual_paths then return path end
    return real_path_to_virtual(path)
end 

local function resolve_replacements(str, replacements)
    for _,r in ipairs(replacements) do
        str = str:gsub(r[1], r[2])
    end
    return str
end

local function resolve_deletions(stacktrace, i)
    -- We exit immediately if we meet a filter pattern
    for pattern,amount in pairs(M.filter_patterns) do
        if stacktrace[i]:find(pattern) == 1 then
            local delete = math.min(i, amount)
            local index = i - delete + 1
            for j=1,delete do table.remove(stacktrace, index) end
            return delete
        end
    end
    return 0
end

local function resolve_changes(stacktrace, i)
    stacktrace[i] = resolve_replacements(stacktrace[i], M.stacktrace_replacements)

    local inserts = 0
    local converted = {}
    local function path_conv(path_and_line)
        -- A hack to remove cases with ) at the end
        local parts = path_and_line:split(":")
        local line_num = tonumber(parts[#parts])
        parts[#parts] = nil
        local path = (":"):join(parts)
        if M.virtual_paths then table.insert(converted, M.resolve_color("CYAN", path, ';2')) end
        local ret = resolve_path(path) .. ':' .. M.resolve_color("RESET",line_num, ';1')
        if M.context > 0 and #converted == 1 then
            local lines = M.resolve_context(path, line_num, M.context)
            inserts = #lines
            -- Add context lines
            for j=1,#lines do table.insert(stacktrace, j + i, lines[j]) end
        end
        return ret
    end
    stacktrace[i] = stacktrace[i]:gsub(LUAFILE_PATTERN .. ":%d+", path_conv)
    if #converted > 0 then
        stacktrace[i] = stacktrace[i] .. ' (' .. converted[1] .. ')'
    end
    return inserts
end

local debug_traceback = debug.traceback -- Stash & wrap the current debug.traceback
-- Improve the traceback in various ways, including adding color and reducing noise:
function M.traceback(--[[Optional]] str)
    local traceback = debug_traceback()
    local stacktrace = traceback:split('\n')
    local i = 1
    while i <= #stacktrace do
        i = i + 1 - resolve_deletions(stacktrace, i)
    end
    i = 1
    while i <= #stacktrace do
        i = i + 1 + resolve_changes(stacktrace, i)
    end
    return resolve_replacements(str or "", M.error_replacements) .. '\n' .. table.concat(stacktrace, '\n')
end

AnsiColors = import "terminal.AnsiColors" -- Lazy import
debug.traceback = M.traceback

return M