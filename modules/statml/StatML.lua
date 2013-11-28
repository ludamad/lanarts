local yaml = import "core.yaml"
-- Defines builtin handlers !object, !trait, and !class:
local builtintags = import "@builtintags"
local nodeops = import "@nodeops"
local Util = import "@StatMLUtil"

-- This module uses global variables heavily, 'M.reset' resets them
local _context,_parsers,_parsed,_unparsed,_all_parsed, _all_unparsed -- Uninitialized until M.reset below!

local M = nilprotect {} -- Submodule

local ObjectSet = typedef [[
    list :list({})
    map, reverse_map :map ({})
]]

function ObjectSet:add(id, r)
    assert(not self.map[id])
    append(self.list, r)
    self.map[id] = r ; self.reverse_map[r] = id 
end

local StatMLContext = typedef [[
    parsers, parsed, unparsed, all_parsed, all_unparsed :map({})
]]

function M.reset()
    _context = StatMLContext.create()
    _parsers,_parsed,_unparsed=_context.parsers,_context.parsed,_context.unparsed
    _all_parsed,_all_unparsed=_context.all_parsed,_context.all_unparsed
    -- Copy builtin into parsers
    for k,v in pairs(builtintags) do _parsers[k] = v ; _parsed[k] = ObjectSet.create({},{},{}) end
    M.instances = _parsed["instance"].map
end

M.reset() -- Initialize above variables.

-- Implementation routine that does not remove from unparsed list:
local function _parse(node)
    local tag,id = node.__tag,node.id
    if _all_unparsed[tag] then M.resolve_node(tag) end
    local parser = _parsers[tag] or rawget(builtintags, tag)
    if not parser then
        error("No parser defined for '" .. tag .. "'!")
    end

   local result = assert(parser(node), "Parser did not return result object!")
    _parsed[tag]:add(id, result) ; _all_parsed[id] = result ; _all_unparsed[id] = nil
end

function M.resolve_node(id)
    local node = _all_unparsed[id]
    if node then
        assert(not _all_parsed[id])
        _all_unparsed[id] = nil
        table.remove_occurrences(_unparsed[node.__tag], node) 
        _parse(node)
    end
    return _all_parsed[id]
end

local function parse_all_for_tag(tag)
    local list = _unparsed[tag]
    for _, node in ipairs(list) do _parse(node) end
    table.clear(list) 
end

function M.parse_all()
    for tag,_ in pairs(_unparsed) do parse_all_for_tag(tag) end
    table.clear(_all_unparsed)
end

local function load(raw, file)
    local tag = nodeops.convert_raw_to_statml_node(raw, file)
    -- Resolve tags on demand, add to list of unparsed
    local list = _unparsed[tag] or {}
    _unparsed[tag] = list ; append(list, raw)
    if _all_unparsed[raw.id] then error("Node '" .. raw.id .. "' already exists!") end
    _all_unparsed[raw.id] = raw
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

return M 