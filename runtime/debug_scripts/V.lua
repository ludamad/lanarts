local MiscSpellAndItemEffects = require "core.MiscSpellAndItemEffects"
local Map = require "core.Map"
local player = require("core.World").local_player

noscreen(function()
    MiscSpellAndItemEffects.visit_all_maps(player)
end)
