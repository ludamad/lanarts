-- Define replacement for common operations that should be in C++ for normal Lua, but in Lua for LuaJIT:

if not __LUAJIT then
    function table.clear(t) -- Luajit defines an innate operation
        for i=#t,1 do
            t[i] = nil
        end
        for k,v in pairs(t) do
            t[k] = nil
        end
    end

    function profile(f, ...) return f(...) end
    return
end

--- XXX: Seemingly hit a LuaJIT bug that causes segfaults with the 'fwd' optimization.
(require 'jit.opt').start('-fwd')

require("table.clear")

local setmetatable, tostring, rawset, error = setmetatable, tostring, rawset, error
local do_nothing = do_nothing

function profile(f)
    require("jit.p").start("vFL")
    local ret = {f()}
    require("jit.p").stop()
    return unpack(ret)
end

