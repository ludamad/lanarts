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

print "Read core globals:"
require "globals.CoreGlobals"

print "Read modules that must be initialized:"
require "Logging"
require "ErrorReporting"

print "Read other globals"

require "globals.Debug"
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

argv_configuration = nilprotect {
    load_file = false
}

-- Now that global variable mutation has been done, protect globals even more strongly, prevent creation of new globals:
setmetatable(_G, {__index = function(self, k)
    error( ("Global variable '%s' does not exist!"):format(k) )
end, __newindex = function(self, k)
    if k ~= 'player' then  -- Hacks for engine
        error( ("Not in file called by GlobalVariableSetup, cannot set global variable '%s'!"):format(k) )
    end
end})

