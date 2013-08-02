-- Define essential global variables, and module engine interaction here.
-- Global variables & functions will be locked after the 'main.lua' file of each module has ran

local ROOT_FOLDER = "modules"

-- Name caches
local mname_table = {}
local vpath_table = {}

function virtual_path(idx)
    idx = (idx or 1) + 1
    local src = debug.getinfo(idx, "S").source
    local vpath = vpath_table[src]
    if vpath then return vpath end
    vpath = virtual_path_create_relative(src, ROOT_FOLDER)
    vpath_table[src] = vpath
    return vpath
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
        return { loaded(vpath) }
    else
        print("'import_file' error loading path \"" .. rpath .. "\": " .. err)
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
        local path_root = virtual_path(2)
        if path_root == "" then vpath = vpath_rest
        else vpath = path_root .. '.' .. vpath_rest end
    end

    local module = _IMPORTED[vpath]
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

-- Require is used when interacting with 'vanilla' lua modules
require_path_add("modules/?.lua")

local main = import "core.main"

-- Hardcoded for now!
local modules = {"lanarts", "test"}

for m in values(modules) do
    import(m .. ".main")
end

function main()
    
end

print "End of 'modules/main.lua'."
