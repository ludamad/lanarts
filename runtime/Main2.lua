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
    -- Parse arguments
    local args = parser:parse(raw_args)
    for _, filename in ipairs(args.lua) do
        dofile(filename)
    end
end

return main
