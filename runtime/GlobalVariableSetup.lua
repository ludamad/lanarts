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

-- Ensure undefined global variable access results in an error
nilprotect(_G)

print "NOOOO3"
setmetatable(_G, {__index = function(self, k)
    error( ("Global variable '%s' does not exist!"):format(k) )
end})

print "Read core globals"
require "globals.CoreGlobals"

print "Read modules that must be initialized"
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
