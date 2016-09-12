_PACKAGES = {"core", "game", "example"} -- Hardcoded for now

--------------------------------------------------------------------------------
-- Path utility functions
--------------------------------------------------------------------------------

--- Return whether a file with the specified name exists.
-- More precisely, returns whether the given file can be opened for reading.
function file_exists(name)
    local f = io.open(name,"r")
    if f ~= nil then io.close(f) end
    return f ~= nil
end

local function find_package(str)
    local expected = (str:sub(1,1) == "@") and 2 or 1
    for i,package in ipairs(_PACKAGES) do
        if str:find(package) == expected then return package end
    end
    error("Could not find package for '"..str.."'!")
end

function real_path_to_virtual(src,  --[[Optional]] drop_filename)
    return virtual_path_create_relative(src, find_package(src), drop_filename)
end

function virtual_path(idx, --[[Optional]] drop_filename)
    return real_path_to_virtual(debug.getinfo((idx or 1) + 1, "S").source, drop_filename)
end

function virtual_path_to_real(root, vpath)
    assert(vpath, "No virtual path given!")
    local rpath = vpath:gsub("%.", "/")
    return root .. '/' .. rpath
end

local function mname(vpath) return vpath:split(".")[1] end

_IMPORTED = {} -- Hold submodule data
local function create_submodule(vpath, rpath, ...) 
    _IMPORTED[vpath] = {..., module_name = mname(vpath), real_path = rpath}
end
-- Protect against double-loading the main booting path:
for _,vpath in ipairs {"GlobalVariableLoader", "Main", "ModuleSystem"} do
    create_submodule(vpath, virtual_path_to_real("core", vpath))
end

local function mname(vpath) return vpath:split(".")[1] end

function package_name(idx)
    idx = (idx or 1) + 1
    local src = debug.getinfo(idx, "S").source
    return find_package(src)
end

function module_name(idx)
    idx = (idx or 1) + 1
    local src = debug.getinfo(idx, "S").source
    -- Check cache
    local mdata = _IMPORTED[src]
    if mdata then return mdata.module_name end

    return mname(virtual_path(idx))
end

-- Utility for finding submodules of a given virtual-path
function find_submodules(vpath, --[[Optional]] recursive, --[[Optional]] pattern, --[[Optional]] filter)
    if type(filter) == "string" then
        local pattern = filter
        filter = function(c) return c:match(pattern) end
    end
    local ret = {}
    for _, package in ipairs(_PACKAGES) do
        local root =  virtual_path_to_real(package, vpath)
        for file in values(io.directory_search(root, (pattern or "*") .. ".lua", recursive or false)) do
            local c = virtual_path_create_relative(file, package)
            if not filter or filter(c) then
                table.insert(ret, c)
            end        
        end
    end
    return ret
end

--------------------------------------------------------------------------------
-- Import resolvers
--------------------------------------------------------------------------------
local function import_file_rpath(rpath, vpath)
    if not file_exists(rpath) then
        return nil, ("import: File " .. rpath .. " does not exist")
    end
    local _load, err = loadfile(rpath)
    if _load then
        local data = { _load(vpath) }
        return data
    else -- The file exists, but there was an error. Do not continue.
        error(err)
    end
end

local function lastentry(vpath)
    local parts = vpath:split(".")
    return parts[#parts]
end

local function import_file_from_package(package, vpath)
    local data, err = import_file_rpath(virtual_path_to_real(package, vpath) .. '.lua', vpath, package)
    if not data and package then
        data = import_file_rpath(virtual_path_to_real(package, vpath .. "." .. lastentry(vpath)) .. '.lua')
    end
    return data, err    
end

local function import_file(vpath)
    local errs = nil
    for i,package in ipairs(_PACKAGES) do 
        local data, err = import_file_from_package(package, vpath)
        if data then return data end
        if err then 
            errs = errs or {}
            table.insert(errs, err)
        end
    end
    return nil, table.concat(errs, "\n")
end

function import_internal(vpath)
    return assert(LEngine.import_internal_raw(vpath)[1], "Internal import failed for '" .. vpath .. "'.")
end

_import_resolvers = { import_file, --[[Engine defined]] LEngine.import_internal_raw }

--------------------------------------------------------------------------------
-- Handle root imports. These imports always resolve to the current package
-- root folder (ie, do not exist in any module).
--------------------------------------------------------------------------------

local root_imports = {}
for _,package in ipairs(_PACKAGES) do
    root_imports[package] = {}
end

-- Import a file from a package's root (ie, does not exist in any module)
local function import_root_aux(package, vpath)
    local cache = root_imports[package]
    local data,err = cache[vpath],nil
    if not data then
        data, err = import_file_rpath(string.format("%s/%s.lua", package, vpath))
        cache[vpath] = data
    end
    return data, err
end

function import_root(package, vpath)
    local data,err = import_root_aux(package, vpath)
    if not data then error(err) end
    return unpack(data)
end

--------------------------------------------------------------------------------
-- Main interface, import functions.
--------------------------------------------------------------------------------

_ILLEGAL_RECURSION_SENTINEL = {} 

local default_import_args = {
    -- Override implicit module name
    module_name = nil,
    -- Only check if already loaded
    cache_check_only = false,
    root_load = false,
    ignore_cache = false
}

function import(vpath, --[[Optional]] args)
    args = args or default_import_args
    local mname, cache_check_only, ignore_cache = args.module_name, args.cache_check_only, args.ignore_cache

    local first_chr = vpath:sub(1,1)
    local vpath_rest = vpath:sub(2, #vpath)
    if first_chr == '@' then
        -- This is a module-local import. The name of the current module replaces '@'.
        mname = mname or module_name(2)
        vpath = mname .. '.' .. vpath_rest
    elseif first_chr == '.' then
        -- This is a relative import. The current virtual path is used for look-up.
        local path_root = virtual_path(2, --[[Drop ending filename]] true)
        if path_root == "" then vpath = vpath_rest
        else vpath = path_root .. '.' .. vpath_rest end
    elseif not vpath:find("%.") then 
        -- Try to see if we are a root import within the current package. A root import does not live within a
        -- module and cannot normally be imported from another package, except explicitly by 'import_root'. 
        local data,__ignored_err = import_root_aux(package_name(2), vpath)
        if data then return unpack(data) end
        -- Otherwise, not a root import. We carry on as normal; it can also be a shortform: eg, 'foo' expands to 'foo.foo'
    end

    if not ignore_cache then
        -- Test cache
        local module = _IMPORTED[vpath] or _LOADED[vpath]
        if module == _ILLEGAL_RECURSION_SENTINEL then
            error("'import' detected import recursion from '" .. vpath .. "' being loaded from '" .. (mname or '') .. "'")
        elseif module then 
            return unpack(module)
        elseif cache_check_only then
            return nil
        end

        -- Protect against import circles
        _IMPORTED[vpath] = _ILLEGAL_RECURSION_SENTINEL
        _LOADED[vpath] = _ILLEGAL_RECURSION_SENTINEL
    end

    local errors = {}
    -- Step from most recently added import resolver to least
    for _, resolver in ipairs(_import_resolvers) do
        local resolution, err = resolver(vpath)
        if resolution then
            if not ignore_cache then 
                _IMPORTED[vpath] = resolution
                _LOADED[vpath] = resolution -- Set the require cache for some try at compatibility
            end
            return unpack(resolution)
        else
            table.insert(errors, err)
        end
    end
    local main_err = "\n'import': Failed to load module '" .. vpath .. "'"
    error(table.concat(errors, '\n') .. main_err)
end

local weak_args = {cache_check_only = true}
function import_weak(vpath) return import(vpath, weak_args) end


function import_all(subpackage, --[[Optional]] recursive, --[[Optional]] pattern,  --[[Optional]] filter)
    local content = find_submodules(subpackage, recursive or false, pattern or "*", filter)
    for c in values(content) do import(c) end
end

function import_if_file(vpath, --[[Optional]] import_args)
    for _, package in ipairs(_PACKAGES) do
        local fname = virtual_path_to_real(package, vpath) .. '.lua'
        if file_exists(fname) then
            return import(vpath, import_args)
        end
    end
    return nil
end

local dofile_args = {ignore_cache = true}
function import_dofile(vpath) return import(vpath, dofile_args) end
function import_dofile_if_file(vpath, --[[Optional]] import_args)
    return import_if_file(vpath, dofile_args)
end

-- Import a mutable copy of (potentially) multiple submodules merged together.
function import_copy(...)
    local import_args = {module_name = module_name(2)}
    local copy = nilprotect {} -- Make sure nil access errors instead.
    for i=1,select("#", ...) do
        local module = import(select(i, ...), import_args)
    	for k,v in pairs(module) do
            copy[k] = v
    	end
    end
    return copy
end

-- Iterates, returning [module, package]
function module_package_iter()
    local i,j = 1,1
    local package,list = nil,nil
    return function()
        while not list or j > #list do
            i = i+1 ; package = _PACKAGES[i-1]
            if not package then return nil end
            local modules = io.directory_subdirectories(package)
            list = modules ; j = 1
        end
        j = j+1 ; return list[j-1], package
    end
end

-- Returns list of all unique modules.
function module_list()
    local ret, mmap = {}, {}
    for m, _ in module_package_iter() do
        if not mmap[m] then
            append(ret, m) ; mmap[m] = true
        end 
    end
    return ret
end

-- Iterate all unique modules
function module_iter() return values(module_list()) end
