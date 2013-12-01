local M = nilprotect {} -- Submodule

function M.function_compile(signature, body, --[[Optional]] prelude, --[[Optional]] perf_name)
    if type(body) == "table" then body = table.concat(body) end
    local func_str = table.concat {
        prelude or "", "\nreturn ", signature, "\n", 
        perf_name and ("perf.timing_begin('"..perf_name.."')\n") or "", 
        body, 
        perf_name and ("perf.timing_end('"..perf_name.."')\n") or "", 
        "end"
    }
    local func_loader, err = loadstring(func_str)
    if err then error(err) end
    return func_loader()
end

-- Resolve attributes
function M.resolve_key(attrib)
    local formatter = '[%s]' 
    if type(attrib) == "string" then
        formatter = '["%s"]'
        if attrib:match("^[_%w]+$") then formatter = '.%s' end
    end
    return formatter:format(attrib)
end

local R = M.resolve_key

function M.simple_copy(attrib)
    return ("b%s = a%s\n"):format(R(attrib), R(attrib))
end

function M.meta_copy(attrib)
    return ("a%s:__copy(b%s)\n"):format(R(attrib), R(attrib))
end

function M.shallow_copy(attrib)
    return ("table.copy(a%s, b%s)\n"):format(R(attrib), R(attrib))
end

function M.deep_copy(attrib)
    return ("table.deep_copy(a%s, b%s)\n"):format(R(attrib), R(attrib))
end

function M.method_copy(attrib, method)
    return ("a%s:%s(b%s)\n"):format(R(attrib), method, R(attrib))
end

local function tabs(indent) return ("\t"):rep(indent) end

function M.complex_copy(attrib, body, --[[Optional]] indent)
    indent = indent or 0
    if type(body) == "table" then
        body = M.copy_block_create(body, indent + 1)
    else 
        body = tabs(indent+1) .. body .. "\n"
    end
    local prelude = ("do local a,b = a%s, b%s\n"):format(R(attrib), R(attrib))
    return table.concat {prelude, body, tabs(indent), "end\n"}
end

local attrib_map = { simple = M.simple_copy, meta = M.meta_copy, shallow = M.shallow_copy, deep = M.deep_copy }
function M.copy_block_create(attribs, --[[Optional]] indent)
    indent = indent or 1
    local statements = {}
    for k, v in pairs(attribs) do
        if attrib_map[k] then
            table.insert_all(statements, map_call(attrib_map[k], v))
        end
    end
    for attrib, body in pairs(attribs.complex or {}) do 
        table.insert(statements, M.complex_copy(attrib, body, indent))
    end
    for attrib, method in pairs(attribs.method or {}) do 
        table.insert(statements, M.method_copy(attrib, method))
    end
    return tabs(indent) .. table.concat(statements, tabs(indent))
end

function M.copy_function_compile(attribs, --[[Optional]] indent)
    return M.function_compile("function(a, b)", M.copy_block_create(attribs, indent), attribs.prelude or "")
end

return M
