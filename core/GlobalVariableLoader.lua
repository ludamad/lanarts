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

setmetatable(_G, {__index = function(self, k)
    error( ("Global variable '%s' does not exist!"):format(k) )
end})

-- Note: 'import' is not defined until ModuleSystem.lua is ran.
require "ModuleSystem"

import "ErrorReporting"

-- Find all global definitions
for module,package in module_package_iter() do
    if module == "globals" then
        import_all(module)
    else
        if file_exists(package.."/"..module.."/Globals.lua") then
            import(module..".Globals")
        end
        if file_exists(package.."/"..module.."/globals") then
            import_all(module..".globals")
        end
    end
end