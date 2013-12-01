local M = nilprotect {} -- Submodule

function M.as_map(node)
    local map = {} ; for k,v in M.iter_all(node) do
        if map[k] ~= nil then M.node_error(node, "Key '" .. k .. "' is used twice!") end
        map[k] = v
    end ; return map
end

-- Parse the yes/no values of a discrete amount of elements
function M.bool_vector_parse(str, elements)
    print(str)
    local parts = str:split(" ")
    for _,p in ipairs(parts) do
        if p:trim() ~= "" then 
            assert(table.contains(elements, p), "Unrecognized symbol '" .. p .. "'.")
        end
    end
    local ret = {} ; for _,e in ipairs(elements) do
        append(ret, table.contains(parts, e))
    end ; return ret
end

function M.iter_all(node)
    local idx = 0
    return function()
        idx = idx + 1
        local pair = rawget(node,idx)
        if not pair then return nil end
        return pair[1],pair[2],idx
    end
end

function M.node_assert(condition, node, error_str, ...)
    if not condition then
        if node.__file then
            error(("%s (%s:%d:%d)"):format(fmt(error_str,...), node.__file, node.__line, node.__column))
        else
            error(("%s (line %d column %d)"):format(fmt(error_str,...), node.__line, node.__column))
        end
    end
end
function M.node_error(node, error_str, ...) M.node_assert(false, node, error_str, ...) end

function M.str_part_list(str) 
    local list = {} ; for p in M.str_parts(str) do 
        append(list, p)
    end ; return list
end
function M.str_parts(str)
    return str:gmatch("%s*([^%s]+)%s*")
end

function M.key_list(node)
    local list = {} ; for k,v in M.iter_all(node) do append(list, k) end ; return list
end
function M.index_map(node)
    local map = {} ; for k,_,idx in M.iter_all(node) do map[k] = idx end ; return map
end

function M.extract_all(node) for i=#node,1,-1 do node[i] = nil end end
function M.extract(node, label)
    local value, value_idx = nil,nil
    for k,v,idx in M.iter_all(node) do
        if k == label then
            M.node_assert(value == nil, node, "Cannot have more than one definition for '" .. label .."'!")
            M.node_assert(v ~= nil, node, "Logic error")
            value = v
            value_idx = idx
        end
    end
    if value_idx then table.remove(node, value_idx) end
    return value
end

function M.assert_empty(node)
    local parts = nil ; for k,_ in M.iter_all(node) do parts = parts or {}
        append(parts, "Error: Unknown/unused key '" .. k .. "' when parsing '" .. node.__tag .. "'.")
    end ; if parts then error(("\n"):join(parts)) end
end

function M.add_derived(node,k,v) append(node,{k,v}) end

function M.extract_matches(node, pattern)
    local i = 1
    return function()
        while i <= #node do
            local label,data = unpack(node[i])
            local matched = label:match(pattern)
            if matched then
                table.remove(node,i) -- Don't need to increment 'i'
                return matched, data
            end
            i = i + 1
        end
        return nil
    end
end
function M.extract_parametric(node, label) return M.extract_matches(node, "^%s*" .. label .. "%s*%((.*)%)%s*$") end


return M