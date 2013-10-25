---------------------------------
-- Module loading system --
---------------------------------

-- Name caches
local mname_table = {}

function real_path_to_virtual(src,  --[[Optional]] drop_filename)
    return virtual_path_create_relative(src, _ROOT_FOLDER, drop_filename)
end

function virtual_path(idx, --[[Optional]] drop_filename)
    return real_path_to_virtual(debug.getinfo((idx or 1) + 1, "S").source, drop_filename)
end

function virtual_path_to_real(vpath)
    local rpath = vpath:gsub("%.", "/")
    return _ROOT_FOLDER .. '/' .. rpath
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
    local rpath = virtual_path_to_real(vpath) .. '.lua'
    -- Note, '__loadfile' appears in debug traces and thus the name choice should be clear
    local _load, err = loadfile(rpath)
    if _load then
        local entries = { _load(vpath) }
        return entries
    else
        if err:find("No such file or directory") then
            err = ("File " .. rpath .. " does not exist")
        end
        return nil, "'import_file': " .. err
    end
end

function import_internal(vpath)
    return assert(LEngine.import_internal_raw(vpath)[1], "Internal import failed for '" .. vpath .. "'.")
end

_import_resolvers = { import_file, --[[Engine defined]] LEngine.import_internal_raw }

_ILLEGAL_RECURSION_SENTINEL = {} 
-- Protect against double-loading the main booting path.
_IMPORTED = {
    ["Main"]={}, ["core.Main"]={}, ["core.globals.Modules"]={} 
}

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
function find_submodules(vpath, --[[Optional]] recursive, --[[Optional]] pattern)
    local ret, root = {}, virtual_path_to_real(vpath) 
    for file in values(io.directory_search(root, (pattern or "*") .. ".lua", recursive or false)) do
        table.insert(ret, virtual_path_create_relative(file, _ROOT_FOLDER))
    end
    return ret
end

function import_all(subpackage, --[[Optional]] pattern, --[[Optional]] recursive, --[[Optional]] filter)
    local content = find_submodules(subpackage, recursive or false, pattern or "*")
    for c in values(content) do
        if not filter or filter(c) then
            import(c)
        end
    end
end

Errors = import ".Errors"