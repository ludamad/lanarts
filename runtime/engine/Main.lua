-- engine.Main
--
-- This module exposes a 'main' function that acts as a flexible runner for
-- the full Lanarts game as well as test components / alternate entry points.

-- (1) Setup paths
package.path = package.path .. ';dependencies/?.lua'
package.path = package.path .. ';dependencies/socket/?.lua'

-- (2) Surpress noisy input
require("Logging").set_log_level(os.getenv("LANARTS_LOG") or "WARN")

-- (3) Include necessary global modifications
require("GlobalVariableSetup")(--[[Surpress loading draw-related globals?]] os.getenv("LANARTS_HEADLESS") ~= nil)

-- (4) Ensure moonscript loading is inserted
require("moonscript.base").insert_loader()

-- Default usage:
--   On normal desktop play: ./lanarts engine.StartLanarts *args*
local function main(raw_args)
    -- Get the module / lua file to run
    local run_target = raw_args[1] or 'engine.StartLanarts'
    table.remove(raw_args, 1)
    local module_name = run_target:gsub(".moon", ""):gsub(".lua", ""):gsub("/", ".")
    -- Delegate to the passed module
    -- COMPATIBLITY with lua utilities:
    rawset(_G, 'arg', raw_args)
    if not run_target:find("engine%.") then
        local StartEngine = require "engine.StartEngine"
        return StartEngine.start_engine {
            settings = { -- Settings object
                -- Window settings
                fullscreen = false,
                view_width = 640,
                view_height = 480,
                -- Font settings
                font = 'fonts/Gudea-Regular.ttf',
                menu_font = 'fonts/alagard_by_pix3m-d6awiwp.ttf'
            },
            entry_point = function() 
                require(module_name).main(raw_args)
            end
        }
    else 
        local main = require(module_name)
        return main(raw_args)
    end
end

return main
