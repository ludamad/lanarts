local yaml = import "core.yaml"

local M = nilprotect {} -- Submodule

local StatMLNode = newtype()
function StatMLNode:init(id, tag) self.id = id ; self.__tag = tag end
function StatMLNode:remove(k)
    local val = rawget(self,k) ; if val == nil then
        error("Error: Expected manditory key '" .. k .. "' when parsing '" .. self.__tag .. "'.")
    end ; self[k] = nil ; return val
end
function StatMLNode:check_empty()
    self:remove("__tag") ; self:remove("id") -- Make sure they do not conflict with the check
    local parts = nil ; for k,v in pairs(self) do parts = parts or {}
        append(parts, "Error: Unknown/unused key '" .. k .. "' when parsing '" .. self.__tag .. "'.")
    end ; if parts then error(("\n"):join(parts)) end
end

local ObjectSet = typedef [[
    list :list
    map :map
]]

function ObjectSet:add(id, r)
    assert(not self.map[id])
    append(self.list, r)
    self.map[id] = r 
end

local Parser = typedef [[
    preprocess_func, parse_func :function
]]
function Parser:parse(raw)
    local id = raw[1][1] -- the key in the first key-value pair
    local node = StatMLNode.create(id, raw.__tag)
    self.preprocess_func(raw, node)
    local parsed = self.parse_func(node)
    node:check_empty()
    return id, parsed
end

local StatMLContext = typedef [[
    parsers, resolved, unresolved :map
]]

local _context = StatMLContext.create({}, {}, {})
local _parsers,_resolved,_unresolved=_context.parsers,_context.resolved,_context.unresolved

local function parse_raw(raw)
    local tag = assert(raw.__tag, "Root-level node without type/tag!")
    local parser = _parsers[tag]
    if parser then
        local id, r = parser:parse(raw)
        _resolved[tag]:add(id, r)
    else
        local unres = _unresolved[tag] or {}
        _unresolved[tag] = unres
        append(unres, raw)
    end
end

function M.parse_file(file)
    print("PARSING " .. file)
    print("--------------------------------------------------------------------------------")
    local nodes = yaml.load(file_as_string(file))
    for i,raw in ipairs(nodes) do
        parse_raw(raw)
    end
end

function M.parse_all(directory, --[[Optional]] extension)
    extension = extension or "yaml"
    local files = io.directory_search(directory, "*." .. extension, --[[recursive]] true)
    for _,file in ipairs(files) do M.parse_file(file) end
end

function M.define_parser(tag, preproc, func)
    assert(not _parsers[tag], "Parser already defined for '" .. tag .. "'!")
    assert(not _resolved[tag], "Object set already exists for '" .. tag .. "'!")
    _parsers[tag] = Parser.create(preproc, func)
    _resolved[tag] = ObjectSet.create({},{})
    for i,raw in ipairs(_unresolved[tag] or {}) do parse_raw(raw) end
    _unresolved[tag] = nil
end

local function as_map(raw, node) 
    for i=2,#raw do
        local k,v=raw[i][1],raw[i][2]
        assert(rawget(node,k) == nil, "Key '"..k.."' already specified!") -- TODO better errors
        node[k] = v
    end
end

function M.define_map_parse(parserdefs)
    for tag,func in pairs(parserdefs) do
        M.define_parser(tag, as_map, func)
    end
end

function M.assert_finished()
    for k, v in pairs(_unresolved) do
        assert(#v == 0, "Fatal: Finished parsing, but did not parse '!"..k.."' nodes!") 
    end
end

--local function parse_class_candidate(context, line, fields)
--    -- Returns false if was not a class description.
--    local class_name = line:match("^class%s+([%w_]+)")
--    if not class_name then return false end
--    local class = StatMLClass.create(class_name)
--end


return M 