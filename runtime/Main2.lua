-- Setup paths
package.path = package.path .. ';dependencies/?.lua' 
package.path = package.path .. ';dependencies/socket/?.lua' 

-- Surpress noisy input
require("Logging").set_log_level(os.getenv("LANARTS_LOG") or "WARN")

-- Include necessary global modifications
require("GlobalVariableSetup")(--[[Surpress loading draw-related globals?]] os.getenv("LANARTS_HEADLESS") ~= nil)
require("moonscript.base").insert_loader()

local argparse = require "argparse"

local function main(raw_args)
    local parser = argparse("lanarts", "Lanarts shell. By default, starts lanarts.")
    parser:option("-L --lua", "Run lua files."):count("*")
    parser:option("-R --require", "Require lua modules."):count("*")
    -- Parse arguments
    local args = parser:parse(raw_args)
    for _, filename in ipairs(args.lua) do
        local modulename = filename:gsub(".moon", ""):gsub(".lua", ""):gsub("/", ".")
        require(modulename)
    end
    for _, modulename in ipairs(args.require) do
        require(modulename)
    end
end

return main
