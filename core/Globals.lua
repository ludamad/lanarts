local nilprotect_meta = {__index = function(self, k)
    error( ("Key '%s' does not exist in table!"):format(k) )
end}    
-- Set to a metatable that does not allow nil accesses
function nilprotect(t)
    return setmetatable(t, nilprotect_meta)
end

-- Ensure undefined global variable access results in an error
nilprotect(_G)

local modules_submodule = "ModuleSystem"
-- Note: 'import' is not defined until ModuleSystem.lua is ran.
require(modules_submodule)

import "core.globals.FileUtils"
import "core.globals.Errors" -- Get better error reporting as soon as possible

-- Get all directories
local modules = io.directory_subdirectories("core")
for _,module in ipairs(modules) do
    -- Import of they exist
    if file_exists("core/"..module.."/Globals.lua") then
        import(module..".Globals")
    end
    if file_exists("core/"..module.."/globals") then
        import_all(module..".globals")
    end
end