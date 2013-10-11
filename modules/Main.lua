dofile "modules/core/VMConfiguration.lua" -- Configure based on the VM we are using.

-- Ensure undefined global variable access results in an error
nilprotect(_G)

-- Define essential global variables, and module engine interaction here.
-- Global variables & functions will be locked after the 'main.lua' file of each module has ran
_ROOT_FOLDER = "modules"

-- Name caches
local mname_table = {}

function virtual_path(idx, --[[Optional]] drop_filename)
    idx = (idx or 1) + 1
    local src = debug.getinfo(idx, "S").source
    return virtual_path_create_relative(src, _ROOT_FOLDER, drop_filename)
end

function module_name(idx)
    idx = (idx or 1) + 1
    local src = debug.getinfo(idx, "S").source
    -- Check cache
    local mname = mname_table[src]
    if mname then return mname end

    local vpath = virtual_path(idx)
    mname = vpath:split(".")[1]
    mname_table[src] = mname
    return mname
end

function virtual_path_to_real(vpath)
    local rpath = vpath:gsub("%.", "/")
    return _ROOT_FOLDER .. '/' .. rpath
end

local function import_file(vpath)
    local rpath = virtual_path_to_real(vpath) .. '.lua'
    local loaded, err = loadfile(rpath)
    if loaded then
        local entries = { loaded(vpath) }
        return entries
    else
        error("Error while importing " .. rpath .. ":\n\t" .. err)
        return nil
    end
end

_import_resolvers = { import_file, --[[Engine defined]] import_internal }

_ILLEGAL_RECURSION_SENTINEL = {} 
-- Begin with the root main module loaded (it is this one)
_IMPORTED = { main={} }
function import(vpath, --[[Optional]] mname)
    local first_chr = vpath:sub(1,1)
    local vpath_rest = vpath:sub(2, #vpath)
    if first_chr == '@' then
        mname = mname or module_name(2)
        vpath = mname .. '.' .. vpath_rest
    elseif first_chr == '.' then
        local path_root = virtual_path(2, --[[Drop ending filename]] true)
        if path_root == "" then vpath = vpath_rest
        else vpath = path_root .. '.' .. vpath_rest end
    end

    local module = _IMPORTED[vpath] or _LOADED[vpath]
    if module == _ILLEGAL_RECURSION_SENTINEL then
        error("import detected import recursion from '" .. vpath .. "' being loaded from '" .. (mname or '') .. "'")
    elseif module then 
        return unpack(module)
    end

    -- Protect against import circles
    _IMPORTED[vpath] = _ILLEGAL_RECURSION_SENTINEL
    _LOADED[vpath] = _ILLEGAL_RECURSION_SENTINEL

    -- Step from most recently added import resolver to least
    for i=#_import_resolvers,1,-1 do
        local resolution = _import_resolvers[i](vpath)
        if resolution then 
            _IMPORTED[vpath] = resolution
            _LOADED[vpath] = resolution -- Set the require cache for some try at compatibility
            return unpack(resolution)
        end
    end
    error("import was not able to find a loader for '" .. vpath .. "', loaded from '" .. mname .. "'")
end

-- Global data is a special submodule, its members are always serialized
local GlobalData = import "core.GlobalData"

-- Data is defined on a per-submodule basis
function data_load(key, default, --[[Optional]] vpath)
    -- Make a safe & (almost) guaranteed unique key 
    local global_key = (vpath or virtual_path(2)) .. ':' .. key
    local val = GlobalData[global_key]
    if not val then 
        GlobalData[global_key] = default
        return default
    end
    return val
end

-- Utility for finding submodules of a given vpath
function find_submodules(vpath, --[[Optional]] recursive, --[[Optional]] pattern)
    local ret, root = {}, virtual_path_to_real(vpath) 
    for file in values(io.directory_search(root, (pattern or "*") .. ".lua", recursive or false)) do
        table.insert(ret, virtual_path_create_relative(file, _ROOT_FOLDER))
    end
    return ret
end

-- Require is used when interacting with 'vanilla' lua modules
require_path_add("modules/?.lua")

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
function Engine.main(args)
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