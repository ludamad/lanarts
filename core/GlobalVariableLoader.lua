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

-- Note: 'import' is not defined until ModuleSystem.lua is ran.
require "ModuleSystem"

import "ErrorReporting"
import "globals.FileUtils" -- For 'file_exists'

-- Find all global definitions
for _,package in ipairs(_PACKAGES) do
    local modules = io.directory_subdirectories(package)
    for _,module in ipairs(modules) do
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
end