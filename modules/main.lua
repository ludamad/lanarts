-- Define essential global variables, and module engine interaction here.
-- Global variables & functions will be locked after the 'main.lua' file of each module has ran

local ROOT_FOLDER = "modules"

-- Name caches
local mname_table = {}

function virtual_path(idx, --[[Optional]] drop_filename)
    idx = (idx or 1) + 1
    local src = debug.getinfo(idx, "S").source
    return virtual_path_create_relative(src, ROOT_FOLDER, drop_filename)
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

local function import_file(vpath)
    local rpath = vpath:gsub("%.", "/") .. ".lua"
    local loaded, err = loadfile(ROOT_FOLDER .. '/' .. rpath)
    if loaded then
        local entries = { loaded(vpath) }
        return entries
    else
        error("'import file': error loading path \"" .. rpath .. "\": " .. err)
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
        error("import detected import recursion from '" .. vpath .. "' being loaded from '" .. mname .. "'")
    elseif module then 
        return unpack(module)
    end

    -- Protect against import circles
    _IMPORTED[vpath] = ILLEGAL_RECURSION_SENTINEL
    _LOADED[vpath] = ILLEGAL_RECURSION_SENTINEL

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

--- Lanarts Entry point
-- @param args the arguments passed on the command-line.
function Engine.main(args)
    if table.contains(args, "--tests-only") then
        import "tests.main"
        return false
    elseif table.contains(args, "--simulation") then
        import "unstable.simulation"
        return false
    end
    return true -- Continue graphical startup
end

-- Require is used when interacting with 'vanilla' lua modules
require_path_add("modules/?.lua")

-- Hardcoded for now!
local modules = {"core", "lanarts"}

-- Begin loading all the modules
for m in values(modules) do
    import(m .. ".main")
end

print "End of 'modules/main.lua'."
