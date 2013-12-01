local PACKAGES = {"modules"} -- Hardcoded for now

local function find_package(str)
    local expected = (str:sub(1,1) == "@") and 2 or 1
    for i,root in ipairs(PACKAGES) do
        if str:find(root) == expected then return root end
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
-- Protect against double-loading the main booting path.
local function create_submodule(vpath, rpath, ...) 
    _IMPORTED[vpath] = {..., module_name = mname(vpath), real_path = rpath}
end
-- Predefined:
for _,vpath in ipairs {"Main", "core.Main", "core.globals.Modules"} do
    create_submodule(vpath, virtual_path_to_real("modules", vpath))
end

local function mname(vpath) return vpath:split(".")[1] end

function module_name(idx)
    idx = (idx or 1) + 1
    local src = debug.getinfo(idx, "S").source
    -- Check cache
    local mdata = _IMPORTED[src]
    if mdata then return mdata.module_name end

    return mname(virtual_path(idx))
end

local function import_file_from_package(package, vpath)
    local rpath = virtual_path_to_real(package, vpath) .. '.lua'
    -- Note, '__loadfile' appears in debug traces and thus the name choice should be clear
    local _load, err = loadfile(rpath)
    if _load then
        local data = { _load(vpath) }
        return data
    else
        if err:find("No such file or directory") then
            err = ("File " .. rpath .. " does not exist")
        end
        return nil, "'import_file': " .. err
    end
end

local function import_file(vpath)
    local errs = nil
    for i,package in ipairs(PACKAGES) do 
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

_ILLEGAL_RECURSION_SENTINEL = {} 

local default_import_args = {
    -- Override implicit module name
    module_name = nil,
    -- Only check if already loaded
    cache_check_only = false
}

function import(vpath, --[[Optional]] args)
    args = args or default_import_args
    local mname, cache_check_only = args.module_name, args.cache_check_only
 
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
        error("'import' detected import recursion from '" .. vpath .. "' being loaded from '" .. (mname or '') .. "'")
    elseif module then 
        return unpack(module)
    elseif cache_check_only then
        return nil
    end

    -- Protect against import circles
    _IMPORTED[vpath] = _ILLEGAL_RECURSION_SENTINEL
    _LOADED[vpath] = _ILLEGAL_RECURSION_SENTINEL

    local errors = {}
    -- Step from most recently added import resolver to least
    for _, resolver in ipairs(_import_resolvers) do
        local resolution, err = resolver(vpath)
        if resolution then 
            _IMPORTED[vpath] = resolution
            _LOADED[vpath] = resolution -- Set the require cache for some try at compatibility
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

-- Utility for finding submodules of a given vpath
function find_submodules(vpath, --[[Optional]] recursive, --[[Optional]] pattern, --[[Optional]] filter)
    if type(filter) == "string" then
        local pattern = filter
        filter = function(c) return c:match(pattern) end
    end
    local ret = {}
    for _, package in ipairs(PACKAGES) do
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

function import_all(subpackage, --[[Optional]] recursive, --[[Optional]] pattern,  --[[Optional]] filter)
    local content = find_submodules(subpackage, recursive or false, pattern or "*", filter)
    for c in values(content) do import(c) end
end

Errors = import ".Errors"