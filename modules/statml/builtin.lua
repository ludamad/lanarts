-- StatML internal module for deriving parsers based on !object, !trait, and !class specifications.

local StatML -- Lazily imported
local function lazy_import() StatML = StatML or import "@StatML" end
local Util = import "@StatMLUtil"

local M = nilprotect {} -- Submodule

function M.trait()
    lazy_import()
end

local function preprocess(str)
    str:gsub("($b{})", function(str)
        local macro = str:sub(3, -2)
        local func = loadstring("return " .. macro)
        setfenv(macro, {
            ids = StatML.id_list
        })
        local result = func()
        if type(result) == "table" and not getmetatable(result) then 
            result = (" "):join(result)
        end
        return tostring(result)
    end)
    return str
end 

local root_parametric = {
    var = function(metanode, type, names, definition)
        local comma_sep = (", "):join(Util.str_part_list(names))
        append(definition, ("%s : %s"):format(comma_sep, type))
    end
}
local on_parse_parametric = {
    BoolSet = function(metanode, input_field, elems, handlers)
        local elem_map = Util.as_map(elems)
        handlers[input_field] = function(node, str_repr)
            local parts = Util.str_part_list(str_repr)
            for _,part in ipairs(parts) do 
                if not elem_map[part] then error("Unknown element '" .. part .."' in '".. input_field .."'.") end
            end
            for key,derived_field in Util.iter_all(elems) do
                Util.add_derived(node, derived_field, table.contains(parts, key))
            end
        end
    end
}

local function extract_parametric(node, funcs, ...)
    assert(type(node) == "table")
    for label,func in pairs(funcs) do
        for param,data in Util.extract_parametric(node, label) do
            func(node, param, data, ...)
        end
    end
end
local function extract_simple(node, funcs)
    for label,func in pairs(funcs) do
        local val = Util.extract(node, label)
        if val then func(node, val) end
    end
end
local function extract_required_fields(node, object_type)
    local struct = assert(object_type.__structinfo, "Logic error")
    local args = {} ; for f in struct:all_required_fields() do
        local val = Util.extract(node, f.name)
        if val == nil then error("Object '" .. node.id .. "' requires field '"..f.name.."'!") end
        append(args, val)
    end ; return args
end

local _classes = {}

function M.object(metanode)
    lazy_import()
    local definition,handlers = {},{}
    extract_parametric(metanode, root_parametric, definition)
    extract_parametric(Util.extract(metanode, "on_parse"), on_parse_parametric, handlers)
    Util.assert_empty(metanode)
    local object_type = typedef(_classes) (metanode.id)(
        ("\n"):join(definition)
    )
    StatML.define_parser {
        [metanode.id] = function(node)
            extract_simple(node, handlers)
            local args = extract_required_fields(node, object_type)
            return object_type.create(unpack(args))
        end
    }
end

function M.class()
    lazy_import()
end

return M