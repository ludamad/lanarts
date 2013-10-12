-- Utility code that can apply to lua programming in general.

-- Global data is a special submodule, its members are always serialized
local GlobalData = import "core.GlobalData"

-- Data is defined on a per-submodule basis
function data_load(key, default, --[[Optional]] vpath)
    -- Make a safe & (almost) guaranteed unique key 
    local global_key = (vpath or virtual_path(2)) .. ':' .. key
    local val = GlobalData[global_key]
    if not val then 
        GlobalData[global_key] = default
        return default
    end
    return val
end

--- Does nothing. 
--@usage dummy_object = { step = do_nothing, draw = do_nothing }
function do_nothing() end

--- Wraps a function around a memoizing weak table.
-- Function results will be stored until they are no longer referenced.
-- 
-- Note: This is intended for functions returning heavy-weight objects,
-- such as images. Functions that return primitives will not interact
-- correctly with the garbage collection.
--
-- @param func the function to memoize, arguments must be strings or numbers
-- @param separator <i>optional, default ';'</i>, the separator used when joining arguments to form a string key
-- @usage new_load = memoized(load)
function memoized(func, --[[Optional]] separator) 
    local cache = {}
    setmetatable( cache, {__mode = "kv"} ) -- Make table weak

    separator = separator or ";"

    return function(...)
        local key = table.concat({...}, separator)

        if not cache[key] then 
            cache[key] = func(...)
        end

        return cache[key]
    end
end

--- Get a  human-readable string from a lua value. The resulting value is generally valid lua.
-- Note that the paramaters should typically not used directly, except for perhaps 'packed'.
-- @param val the value to pretty-print
-- @param tabs <i>optional, default 0</i>, the level of indentation
-- @param packed <i>optional, default false</i>, if true, minimal spacing is used
-- @param quote_strings <i>optional, default true</i>, whether to print strings with spaces
function pretty_tostring(val, --[[Optional]] tabs, --[[Optional]] packed, --[[Optional]] quote_strings)
    tabs = tabs or 0
    quote_strings = (quote_strings == nil) or quote_strings

    local tabstr = ""

    if not packed then
        for i = 1, tabs do
            tabstr = tabstr .. "  "
        end
    end

    if type(val) == "string" and quote_strings then
        return tabstr .. "\"" .. val .. "\""
    end

    if type(val) ~= "table" then
        return tabstr .. tostring(val)
    end

    local parts = {"{", --[[sentinel for remove below]] ""}

    for k,v in pairs(val) do
        table.insert(parts, packed and "" or "\n") 

        if type(k) == "number" then
            table.insert(parts, pretty_tostring(v, tabs+1, packed))
        else 
            table.insert(parts, pretty_tostring(k, tabs+1, packed, false))
            table.insert(parts, " = ")
            table.insert(parts, pretty_tostring(v, type(v) == "table" and tabs+1 or 0, packed))
        end

        table.insert(parts, ", ")
    end

    parts[#parts] = nil -- remove comma or sentinel

    table.insert(parts, (packed and "" or "\n") .. tabstr .. "}");

    return table.concat(parts)
end

function pretty_tostring_compact(v)
    return pretty_tostring(v, nil, true)
end

-- Resolves a number, or a random range
function random_resolve(v)
    return type(v) == "table" and random(unpack(v)) or v
end

--- Get a  human-readable string from a lua value. The resulting value is generally valid lua.
-- Note that the paramaters should typically not used directly, except for perhaps 'packed'.
-- @param val the value to pretty-print
-- @param tabs <i>optional, default 0</i>, the level of indentation
-- @param packed <i>optional, default false</i>, if true, minimal spacing is used
function pretty_print(val, --[[Optional]] tabs, --[[Optional]] packed)
    print(pretty_tostring(val, tabs, packed))
end

-- Convenience print-like function:
function pretty(...)
    local args = {}
    for i=1,select("#", ...) do
    	args[i] = pretty_tostring_compact(select(i, ...)) 
	end
    print(unpack(args))
end

--- Iterate all iterators one after another
function iter_combine(...)
    local args = {...}
    local arg_n = #args
    local arg_i = 1
    local iter = args[arg_i]
    return function()
        while true do
            if not iter then return nil end
            local val = iter()
            if val ~= nil then return val end
            arg_i = arg_i + 1
            iter = args[arg_i]
        end
    end
end

--- Like a functional map of a function onto a list
function map_call(f, list)
    local ret = {}
    for v in values(list) do 
        ret[#ret + 1] = f(v)
    end
    return ret
end

-- Functional composition
function func_apply_and(f1,f2, --[[Optional]] s2)
    return function(s1,...) 
        local v = {f1(s1,...)}
        if not v[1] then return unpack(v)
        else 
            if s2 then return f2(s2, ...)
            else return f2(s1,...) end
        end
    end
end

function func_apply_not(f)
    return function(...) return not f(...) end
end

function func_apply_or(f1,f2, --[[Optional]] s2)
    return function(s1,...) 
        local v = {f1(s1,...)}
        if v[1] then return unpack(v)
        else 
            if s2 then return f2(s2, ...)
            else return f2(s1,...) end
        end
    end
end

function func_apply_sequence(f1,f2,--[[Optional]] s2)
    return function(s1,...)
        local v = f1(s1,...)
        if s2 then v = f2(s2, ...) or v 
        else v = f2(s1,...) or v end
        return v 
    end
end

local _cached_dup_table = {}
function dup(val, times)
    table.clear(_cached_dup_table)
    for i=1,times do
        _cached_dup_table[i] = val
    end
    return unpack(_cached_dup_table)
end

--- Return a random element from a list
function random_choice(choices)
    local idx = random(1, #choices)
    return choices[idx]
end

local function iterator_helper(f,...)
    return f(...)
end
function iterator_step(state)
    local oldf = state[1] -- Used in hack to determine termination
    table.assign(state, iterator_helper(unpack(state)))
    if state[1] ~= oldf then table.clear(state) end
end