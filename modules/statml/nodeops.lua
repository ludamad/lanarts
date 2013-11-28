local Util = import "@StatMLUtil"
local preprocess = (import "@preprocessor").preprocess
local M = nilprotect {} -- Submodule

-- StatML objects 'steal' the tag of their first element.
local function find_tag(raw)
    if not raw.__tag and raw[1] then
        local first = raw[1]
        local id,initializer=first[1],first[2]
        raw.__initializer = initializer
        raw.id,raw.__tag=id,initializer.__tag
        table.remove(raw, 1)
    end
    Util.node_assert(raw.__tag, raw, "Root-level node without type/tag!")
end 

function M.convert_raw_to_statml_node(raw, file)   
    raw.__file = file
    if raw.__type == "map" then
        find_tag(raw)
        return raw.__tag
    elseif raw.__type == "list" then
        assert("Not expecting root list yet.")
    end ; assert(false)
end

local function preprocess_field(node, name, val)
    Util.node_assert(val, node, "Object '%s' requires field '%s'!", node.id, name)
    return preprocess(val)
end

-- Handle the initializer, if present. Currently an alternative to fields.
local function parse_initializer(node, struct)
    -- No initializer:
    local init = node.__initializer
    if init == "" then return nil end
    assert(type(init) == "table", "Initializer must be a list or map!")
    local args = {}
    if init.__type == "map" then
        local map = Util.as_map(init)
        for f in struct:all_required_fields() do
            append(args, preprocess_field(node, f.name, map[f.name])) ; map[f.name] = nil
        end
        if not table.is_empty(map) then
            Util.node_error(node, "Extra field(s) '%s' in initializer!", (", "):join(table.key_list(map))) 
        end
    else -- list
        local i = 1 ; for f in struct:all_required_fields() do
            append(args, preprocess_field(node, f.name, init[i]))
            i = i + 1
        end ; Util.node_assert(#init == i-1, node, "Extra fields in initializer!")
    end
    return args
end

function M.find_field_values(node, object_type)
    local struct = assert(object_type.__structinfo, "Logic error")
    local args = parse_initializer(node, struct)
    if args then return args end -- Fields specified compactly
    for f in struct:all_required_fields() do
        local val = Util.extract(node, f.name)
        append(args, preprocess_field(node, f.name, val))
    end
    return args
end

return M