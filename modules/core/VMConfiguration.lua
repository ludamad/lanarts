-- Define replacement for common operations that should be in C for normal Lua, but in C++ for LuaJIT:
if __LUAJIT then
    function values(table)
        local idx = 1
        return function()
            local val = table[idx]
            idx = idx + 1
            return val
        end
    end

    local function metacopy(t1, t2)
        local meta = getmetatable(t1)
        if meta then
            local copy = meta.__copy
            if copy then
                copy(t1, t2)
                return true
            end
        end
        return false
    end

    function table.copy(t1, t2, invoke_meta)
        if invoke_meta == nil then invoke_meta = true end
        setmetatable(t2, getmetatable(t1))
        if invoke_meta and metacopy(t1, t2) then
            return
        end

        for k,_ in pairs(t2) do
            t2[k] = nil
        end
        for k,v in pairs(t1) do
            t2[k] = v
        end
    end

end