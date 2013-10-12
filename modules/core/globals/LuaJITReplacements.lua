-- Define replacement for common operations that should be in C for normal Lua, but in C++ for LuaJIT:
if not __LUAJIT then
    return
end

function profile(f, ...)
  local profile = require("jit.profile")
  local pr = {}
  profile.start("f", function(th, samples, vmmode)
    local d = profile.dumpstack(th, "l\t", 3)
    pr[d] = (pr[d] or 0) + samples
  end)
  local ret = {f(...)}
  
end

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

function table.deep_copy(t1, t2, invoke_meta)
    if invoke_meta == nil then invoke_meta = true end
    setmetatable(t2, getmetatable(t1))
    if invoke_meta and metacopy(t1, t2) then
        return
    end

    for k,_ in pairs(t2) do
        if rawget(t1, k) == nil then
            t2[k] = nil
        end
    end
    for k,v in pairs(t1) do
        if type(v) == "table" then
            local old_val = rawget(t2, k)
            if old_val then
                table.deep_copy(v, old_val)
            else
                t2[k] = table.deep_clone(v)
            end
        else
            t2[k] = v
        end
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
        error(("Class object '%s': Cannot read '%s', member does not exist!\n"):format(tostring(self), tostring(k)))
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