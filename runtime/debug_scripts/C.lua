local MiscSpellAndItemEffects = require "core.MiscSpellAndItemEffects"
local Map = require "core.Map"
local player = require("core.World").local_player

noscreen(function()
    MiscSpellAndItemEffects.map_completion(player, Map.map_label(player.map))
end)
