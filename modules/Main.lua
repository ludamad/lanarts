-- Entry point to lanarts. Ensures correct initialization order of the 
-- various functions needed.

_ROOT_FOLDER = "modules"

dofile(_ROOT_FOLDER .. "/core/Main.lua")

local function attach_debugger()
    local DBG_PATH, DBG_MODULE = "/usr/share/lua/5.1/", "debugger"
    local DBG_FILE = DBG_PATH .. DBG_MODULE .. ".lua"
    if not file_exists(DBG_FILE) then
        error("Debugging requires " .. DBG_FILE .. " to exist!")
    end
    require_path_add(DBG_PATH .. "?.lua")
    require(DBG_MODULE)()
    attach_debugger = do_nothing
end

--- Lanarts Entry point
-- @param args the arguments passed on the command-line.
-- @return whether we are performing a full game initialization, or performing some testing task
local function main(args)
    local Display = import "core.Display"

    import "core.Main"

    if table.contains(args, "--debug") then
        attach_debugger()
    end

    local all_tests, cpp_tests, lua_tests = (args[1] == "--tests"), (args[1] == "--cpp-tests"), (args[1] == "--lua-tests")

    if all_tests or cpp_tests or lua_tests then
        local Display = import "core.Display"
        Display.initialize("Tests", {settings.view_width, settings.view_height}, settings.fullscreen)

        local failures = (cpp_tests or all_tests) and _lanarts_unit_tests() or 0
        local passed = (lua_tests or all_tests) and import "tests.Main" or true
        -- Return exit code so ctest will notice the failure
        if failures > 0 or not passed then os.exit(2) end
        return false
    elseif args[1] == "--simulation" then
        Display.initialize("Lanarts", {settings.view_width, settings.view_height}, settings.fullscreen)
        local S = import "unstable.Simulation"
        S.main(args)
        return false   
    elseif args[1] == "--testmap" then
        Display.initialize("Lanarts", {settings.view_width, settings.view_height}, settings.fullscreen)
        import "unstable.TestMap"
        return false
    elseif args[1] == "--example" then
        import ("examples." .. args[2])
        return false
    elseif args[1] == "--repl" then
        local finished = false
        function _G.start_lanarts()
            -- Continue lanarts initialization 
            finished = true
        end
        while not finished do 
            __read_eval_print()
        end
    end

    Display.initialize("Lanarts", {settings.view_width, settings.view_height}, settings.fullscreen)
    -- TODO: Remove any notion of 'internal graphics'. All graphics loading should be prompted by Lua.
    __initialize_internal_graphics()
    
    -- Hardcoded for now!
    import "lanarts.Main"

    return true -- Continue lanarts initialization
end

return main