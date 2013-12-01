-- Entry point to lanarts. Ensures correct initialization order of the 
-- various global components, and handles parsing of the command-line configuration.

_ROOT_FOLDER = "core"
-- For bootstrapping purposes. We normally only use 'import'.
require_path_add(_ROOT_FOLDER .. '/?.lua')
-- Play nice with luasocket:
require_path_add(_ROOT_FOLDER .. '/socket/?.lua')

require("GlobalVariableLoader")

local argv -- Placeholder, set in main()

local function get_param(flag) 
    for i,v in pairs(argv) do 
        local match = v:match(flag .. ":(.*)")
        match = match or (v == flag and argv[i+1])
        if match then
            assertf(match, "Expected parameter but none given for '%s'!", flag)
            assertf(not match:match("^%-%-[^%s]+$"), "Aborting because parameter '%s' for '%s' looks like a flag.", match, flag)
            return match
        end 
    end
    return nil
end
local function has_arg(s) return table.contains(argv, s) end
local function num_param(s) local param = get_param(s) ; return param and tonumber(param) end

-- Special global table; holds command-line arguments parsing utilities.
sysargs = {get_param=get_param, has_arg=has_arg, num_param=num_param}

function sysargs.argv() return table.clone(argv) end

-- Start the beta version of dungenerate, simply 'Lanarts'
local function start_lanarts()
    local Display = import "core.Display"
    Display.initialize("Lanarts", {settings.view_width, settings.view_height}, settings.fullscreen)
    -- TODO: Remove any notion of 'internal graphics'. All graphics loading should be prompted by Lua.
    __initialize_internal_graphics()
    
    -- Hardcoded for now!
    import "lanarts.Main"

    return true -- Continue initialization
end

--- Lanarts entry-point. Handles command-line arguments.
-- @param argv the arguments passed on the command-line.
-- @return whether we are performing a full game initialization, or if have finished our task.
local function main(_argv)
    argv = _argv

    -- Misc options
    if has_arg "--sound" then sound_volume = 1.0 end
    if has_arg "--debug" then debug.attach_debugger() end

    -- Error control options
    local ErrorReporting = import "ErrorReporting"
    if has_arg "--nofilter" then ErrorReporting.filter_patterns = {} end
    -- Number of lines of code context, requires source. 
    ErrorReporting.context = (num_param "-C" or num_param "--context" or 2)

    -- Execution modes
    local function exec(vpath) import(vpath).main(argv) ; return false end
    if has_arg "--tests" then return exec "testrunner" end
    if has_arg "--testmap" then return exec "unstable.TestMap" end
    if has_arg "--simulation" then return exec "unstable.Simulation" end
    if has_arg "--dungenerate" then return exec "dungenerate.Main" end

    -- Using Lanarts as a Lua engine
    if has_arg "--import" then import(get_param "--import") ; return false end
    if has_arg "--lua" then
        -- Optionally, run a Lua file given a normal (ie, not virtual) path
        -- Then drop into a Lua REPL session. The user can exit with 'start_lanarts()'
        local file = (get_param "--lua")
        if file then dofile(file) end
        local finished = false ; function _G.start_lanarts() finished = true end
        while not finished do __read_eval_print() end
    end

    return start_lanarts()
end

-- The bootstrap module (ie, this file, core/Main.lua) returns a function that 
-- takes the command-line arguments for Lanarts
return main