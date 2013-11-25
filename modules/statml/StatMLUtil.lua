local M = nilprotect {} -- Submodule

-- Parse the yes/no values of a discrete amount of elements
function M.bool_vector_parse(str, elements)
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

return M