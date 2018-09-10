local MiscSpellAndItemEffects = require "core.MiscSpellAndItemEffects"
local player = require("core.World").local_player
local Map = require "core.Map"

local function mock_transfer(f)
    local prev = Map.transfer
    Map.transfer = do_nothing
    f()
    Map.transfer = prev
end

return {
    complete_maps = function(maps)
        noscreen(function()
            mock_transfer(function()
                MiscSpellAndItemEffects.visit_all_maps(player)
            end)
            for _, map_label in ipairs(maps) do
                MiscSpellAndItemEffects.map_completion(player, map_label)
            end
        end)
    end
}

