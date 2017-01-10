-- Entry point to lanarts. Ensures correct initialization order of the 
-- various global components, and handles parsing of the command-line configuration.

-- Play nice with dependencies:
require_path_add('dependencies/?.lua')
require_path_add('dependencies/socket/?.lua')

require("GlobalVariableSetup")
require("moonscript.base").insert_loader()

local argv -- Placeholder, set in main()

local LOOKS_LIKE_FLAG = "^[%-]+[^%s]+$"

local function get_varparam(flag, --[[Optional]] max_params)
    local params = {} ; for i,v in pairs(argv) do
        if v == flag then
            for j=i+1,i+1+max_params do
                local param = argv[j]
                if not param or param:match(LOOKS_LIKE_FLAG) then break end
                append(params, param)
            end 
        end
    end ; return unpack(params)
end 

local function get_param(flag) 
    for i,v in pairs(argv) do
        local match = v:match(flag .. ":(.*)")
        match = match or (v == flag and argv[i+1])
        if match then
            assertf(match, "Expected parameter but none given for '%s'!", flag)
            assertf(not match:match("^[%-]+[^%s]+$"), "Aborting because parameter '%s' for '%s' looks like a flag.", match, flag)
            return match
        end 
    end
    return nil
end
local function has_arg(s) return table.contains(argv, s) end
local function num_param(s) local param = get_param(s) ; return param and tonumber(param) end

-- Special global table; holds command-line arguments parsing utilities.
local sysargs = {get_varparam=get_varparam, get_param=get_param, has_arg=has_arg, num_param=num_param}

function sysargs.argv() return table.clone(argv) end

-- Start the beta version of dungenerate, was simply named 'Lanarts'
local function start_lanarts()
    local Display = require "core.Display"
    Display.initialize("Lanarts", {settings.view_width, settings.view_height}, settings.fullscreen)
    -- TODO: Remove any notion of 'internal graphics'. All graphics loading should be prompted by Lua.
    __initialize_internal_graphics()

    -- Hardcoded for now!
    require "LanartsMain"

    return true -- Continue initialization
end

--- Lanarts entry-point. Handles command-line arguments.
-- @param argv the arguments passed on the command-line.
-- @return whether we are performing a full game initialization, or if have finished our task.
local function main(_argv)
    argv = _argv

    -- Misc options
    --if has_arg "--sound" then sound_volume = 1.0 end
    if has_arg "--debug" then debug.attach_debugger() end

    -- Error control options
    local ErrorReporting = require "ErrorReporting"
    if has_arg "--nofilter" then ErrorReporting.filter_patterns = {} end
    -- Number of lines of code context, requires source. 
    ErrorReporting.context = (num_param "-C" or num_param "--context" or 4)

    -- Execution modes
    local function exec(vpath) require(vpath).main(argv) ; return false end
    if has_arg "--tests" then return exec "TestRunner" end
    if has_arg "--testmap" then return exec "unstable.TestMap" end

    -- Using Lanarts as a Lua engine
    if has_arg "--require" then require(get_param "--require") ; return false end
    if has_arg "--lua" then
        -- Optionally, run a Lua file given a normal (ie, not virtual) path
        -- Then drop into a Lua REPL session. The user can exit with 'start_lanarts()'
        local file = (get_param "--lua")
        local ok, err = pcall(dofile, file)
        if err then print(err) end
        local finished = false ; function _G.start_lanarts() finished = true end
        while not finished do __read_eval_print() end
    end

    return start_lanarts()
end


-- The bootstrap module (ie, this file, core/Main.lua) returns a function that 
-- takes the command-line arguments for Lanarts
return main
