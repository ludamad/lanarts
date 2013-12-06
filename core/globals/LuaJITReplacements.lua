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

function values(table)
    local idx = 1
    return function()
        local val = table[idx]
        idx = idx + 1
        return val
    end
end

function newtype(args)
    local get, set = {}, {}
    local parent = args and args.parent
    local type = {get = get, set = set}

    function type.create(...)
        local val = setmetatable({}, type)
        type.init(val, ...)
        return val
    end

    function type:__index(k)
        local getter = get[k]
        if getter then return getter(self, k) end
        local type_val = type[k]
        if type_val then return type_val end
        if parent then
            local idx_fun = parent.__index
            if idx_fun then return idx_fun(self, k) end
        end
        error(("Cannot read '%s', member does not exist!\n"):format(tostring(k)))
    end

    function type:__newindex(k, v)
        local setter = set[k]
        if setter then
            setter(self, k, v)
            return
        end
        if parent then
            local newidx_fun = parent.__newindex
            if newidx_fun then
                newidx_fun(self, k, v)
                return
            end
        end
        rawset(self, k, v)
    end

    return type
end