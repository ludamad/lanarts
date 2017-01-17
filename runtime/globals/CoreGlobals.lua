--- General convenience global functions.
-- IE, utility code that was decided makes Lua programming in general easier.
-- 
-- Additional (potentially more domain-specific) global functins are define in the 
-- 'globals' module in this package, as well as the 'globals' folder within a module, 
-- or its Globals.lua submodule.

--- Does nothing. 
--@usage dummy_object = { step = do_nothing, draw = do_nothing }
function do_nothing() end
_EMPTY_TABLE = {}

-- Enable references to coloured prints through-out bootstrap code:
require "terminal.Globals"

-- Global data is a special submodule, its members are always serialized
local GlobalData = require "core.GlobalData"
print "WHATT"

local print,error,assert=print,error,assert -- Performance

append = table.insert -- Convenient alias
local tinsert = table.insert
function appendf(t, s, ...)
    return tinsert(t, s:format(...))
end

--- Return whether a file with the specified name exists.
-- More precisely, returns whether the given file can be opened for reading.
function file_exists(name)
    local f = io.open(name,"r")
    if f ~= nil then io.close(f) end
    return f ~= nil
end

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

    local meta = getmetatable(val)
    if type(val) ~= "table" or (meta and meta.__tostring) then
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

-- Like C printf, but always prints new line
function printf(fmt, ...) print(fmt:format(...)) end
function errorf(fmt, ...) error(fmt:format(...)) end
function assertf(cond, fmt, ...) return assert(cond, fmt:format(...)) end
-- Convenient handle for very useful function:
fmt = string.format

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
    for i=1,#list do 
        ret[i] = f(list[i])
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

local NEXT_TO_PLAY_PAINED = 1
function play_pained_sound()
    play_sound("sound/pained" .. NEXT_TO_PLAY_PAINED .. ".ogg")
    NEXT_TO_PLAY_PAINED = NEXT_TO_PLAY_PAINED + 1
    if NEXT_TO_PLAY_PAINED > 3 then
        NEXT_TO_PLAY_PAINED = 1
    end
end

function newtype(args)
    local parent = args and args.parent
    local type = args or {}
    -- 'Inherit' via simple copying.
    -- Note fall back in __newindex anyway.
    if parent ~= nil then
        for k,v in pairs(parent) do type[k] = type[k] or v end
    end
    local get, set = type.get or {}, type.set or {}
    type.get,type.set = get,set
    if _G.type(get) == 'function' then get = {} end
    if _G.type(set) == 'function' then set = {} end

    type.parent = parent
    if type.init == nil then
        type.init = do_nothing
    end

    function type.isinstance(obj)
        local otype = getmetatable(obj)
        while otype ~= nil do 
            if otype == type then
                return true
            end
            otype = otype.parent
        end
        return false
    end
    function type.create(...)
        local val = setmetatable({}, type)
        type.init(val, ...)
        return val
    end

    function type:__index(k)
        local getter = get[k]
        if getter then return getter(self, k) end
        local type_val = type[k]
        if type_val ~= nil then return type_val end
        if parent then
            local idx_fun = parent.__index
            if idx_fun then return idx_fun(self, k) end
        end
        error(("Cannot read '%s', member does not exist!\n"):format(tostring(k)))
    end

    function type:__newindex(k, v)
        if v == nil then
            assert(v ~= nil, "Writing 'nil' to class objects is dubious (because it can't normally be read back). Erroring! Key was: " .. tostring(k))
        end
        local setter = set[k]
        if setter then
            setter(self, v)
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


