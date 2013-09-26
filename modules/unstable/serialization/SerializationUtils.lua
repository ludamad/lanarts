-- Provides routines for essentially writine a struct to a SerializationBuffer
local M = nilprotect {} -- Submodule

-- Serialization types
M.XY,M.NUM,M.BINARY={},{},{}

function M.read_values(sb, types)
    local ret = {}
    for t in values(types) do
        if t == M.XY then
            table.insert(ret, {sb:read_int(), sb:read_int()})
        elseif t == M.NUM then
            table.insert(ret, sb:read_int())
        else
            assert(t == M.BINARY)
            local len = sb:read_int()
            table.insert(ret, sb:read_raw(len))
        end
    end
    return ret
end

function M.write_values(sb, types, values)
    for i,t in ipairs(types) do
        local value = values[i]
        if t == M.XY then
            sb:write_int(math.round(value[1]))
            sb:write_int(math.round(value[2]))
        elseif t == M.NUM then
            sb:write_int(math.round(value))
        else
            assert(t == M.BINARY)
            sb:write_int(#value)
            sb:write_raw(value)
        end
    end
end

return M