import "@Utils"

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
end