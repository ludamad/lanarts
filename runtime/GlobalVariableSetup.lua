--------------------------------------------------------------------------------
-- Finds the standard location of global variables within all the packages.
--------------------------------------------------------------------------------
local error,setmetatable=error,setmetatable

local nilprotect_meta = {__index = function(self, k)
    error( ("Key '%s' does not exist in table!"):format(k) )
end}    
-- Set to a metatable that does not allow nil accesses
function nilprotect(t)
    return setmetatable(t, nilprotect_meta)
end

setmetatable(_G, {__index = function(self, k)
    error( ("Global variable '%s' does not exist!"):format(k) )
end})

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

print "Read core globals:"
require "globals.CoreGlobals"

print "Read modules that must be initialized:"
require "Logging"
require "ErrorReporting"

print "Read other globals"

require "globals.Debug"
require "globals.Deprecated"
require "globals.Draw"
require "globals.FileUtils"
require "globals.GameUtils"
require "globals.LuaJITReplacements"
require "globals.Math"
require "globals.StringUtils"
require "globals.TableUtils"
require "globals.TextComponent"

print "Preload external packages that set globals:"

require "json"
require "socket"
require "ltn12"
require "mime"

print "Preload legacy internal modules that set globals (and need refactoring to not):"

require "effects.Effects"
require "spells.SpellEffects"

print "Assigning, in effect declaring, global variables used by the engine:"
items = false
spells = false
effects = false
enemies = false
sprites = false
projectiles = false
classes = false
player = false

-- TODO get rid of these:
enemy_init = do_nothing
enemy_step = do_nothing
enemy_berserker_step = do_nothing
enemy_berserker_init = do_nothing

-- Now that global variable mutation has been done, protect globals even more strongly, prevent creation of new globals:
setmetatable(_G, {__index = function(self, k)
    error( ("Global variable '%s' does not exist!"):format(k) )
end, __newindex = function(self, k)
    if k ~= 'player' then  -- Hacks for engine
        error( ("Not in file called by GlobalVariableSetup, cannot set global variable '%s'!"):format(k) )
    end
end})

