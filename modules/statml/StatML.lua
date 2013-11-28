local yaml = import "core.yaml"
-- Defines builtin handlers !object, !trait, and !class:
local builtin = import "@builtin"
local Util = import "@StatMLUtil"

-- This module uses global variables heavily, 'M.reset' resets them
local _context,_parsers,_parsed,_unparsed,_all_parsed, _all_unparsed -- Uninitialized until M.reset below!

local M = nilprotect {} -- Submodule

local ObjectSet = typedef [[
    list :list
    map, reverse_map :map
]]

function ObjectSet:add(id, r)
    assert(not self.map[id])
    append(self.list, r)
    _all_parsed[id] = r
    self.map[id] = r ; self.reverse_map[r] = id 
    print("Adding " .. id .. ' ' .. tostring(r))
end

local StatMLContext = typedef [[
    parsers, parsed, unparsed, all_parsed, all_unparsed :map
]]

function M.reset()
    _context = StatMLContext.create({}, {}, {})
    _parsers,_parsed,_unparsed=_context.parsers,_context.parsed,_context.unparsed
    _all_parsed,_all_unparsed=_context._all_parsed,_context,_all_unparsed
    -- Copy builtin into parsers
    for k,v in pairs(builtin) do _parsers[k] = v ; _parsed[k] = ObjectSet.create({},{},{}) end
    M.instances = _parsed["instance"].map
end

M.reset() -- Initialize above variables.

local function parse_all(tag, nodes)
    local parser = assert(_parsers[tag], "No parser defined for '" .. tag .. "'!")
    for _,n in ipairs(nodes) do
        if rawget(builtin,tag) ~= nil then
            -- Resolve builtin tags somewhat specially
           local id = n.id
           local r = builtin[tag](n)
           if r then _parsed[tag]:add(id, r) end
        else
            -- User-defined tag
            local result = assert(parser(n), "Parser did not return result object!")
            _parsed[tag]:add(n.id, result)
        end
    end 
end

local function prepare_node(raw, file)
    raw.__file = file
    if raw.__type == "map" then
        if not raw.__tag and raw[1] then
            local first = raw[1]
            local id,tag=first[1],first[2].__tag
            raw.id,raw.__tag=id,tag
            table.remove(raw, 1)
        end
        Util.node_assert(raw.__tag, raw, "Root-level node without type/tag!")
        return raw.__tag
    elseif raw.__type == "list" then
        assert("Not expecting root list yet.")
    end ; assert(false)
end

local function load(raw, file)
    local tag = prepare_node(raw, file)
    -- Resolve tags on demand, add to list of unparsed
    local list = _unparsed[tag] or {}
    _unparsed[tag] = list ; append(list, raw)
    if _all_unparsed[raw.id] then error("Node '" .. raw.id .. "' already exists!") end
    _all_unparsed[raw.id] = tag
end

function M.load_file(file)
    local nodes = yaml.load(file_as_string(file))
    if not nodes then -- File had no content
        print("StatML: Warning, '" .. file .. "' had no content.")
        return
    end
    for _,raw in ipairs(nodes) do load(raw, file) end
end

function M.load_directory(directory, --[[Optional]] extension)
    extension = extension or "yaml"
    local files = io.directory_search(directory, "*." .. extension, --[[recursive]] true)
    for _,file in ipairs(files) do M.load_file(file) end
end

function M.id_list(tag)
    assert(_parsers[tag], "No parser associated with '"..tag.."'!")
    assert(_unparsed[tag], "Attempt to take id list of '"..tag.."', but no elements were found.")
    local ids, oset = {}, _parsed[tag]
    for _, r in ipairs(oset.list) do append(ids, oset.reverse_map[r]) end
    for _, node in ipairs(_unparsed[tag]) do append(ids, node.id) end
    return ids
end

function M.define_parser(parserdefs)
    for tag,func in pairs(parserdefs) do
        assert(not _parsers[tag], "Parser already defined for '" .. tag .. "'!")
        assert(not _parsed[tag], "Object set already exists for '" .. tag .. "'!")
        _parsers[tag] = func
        _parsed[tag] = ObjectSet.create({},{},{})
    end
end

function M.lookup(tag, id) 
    return _parsed[tag].map[id]
end

function M.parse_all()
    for tag,list in pairs(_unparsed) do parse_all(tag, list) end
    table.clear(_unparsed)
    table.clear(_all_unparsed)
end

return M 