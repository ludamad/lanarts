---------------------------------
-- Module loading system --
---------------------------------

-- Name caches
local mname_table = {}

function virtual_path(idx, --[[Optional]] drop_filename)
    idx = (idx or 1) + 1
    local src = debug.getinfo(idx, "S").source
    return virtual_path_create_relative(src, _ROOT_FOLDER, drop_filename)
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
-- Begin with the root main module loaded
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