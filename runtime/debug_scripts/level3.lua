-- To use in game, run !!level3.
-- Gets the user most of the way to a level3-ish build.

local ObjectUtils = require "objects.ObjectUtils"
local ItemUtils = require "maps.ItemUtils"
local ItemGroups = require "maps.ItemGroups"
local player = require("core.World").local_player
for i=1,12 do
    local item = ItemUtils.randart_generate(1)
    ObjectUtils.spawn_item_near(player, item.type, item.amount)
end

--for i=1,20 do
--    local item = ItemUtils.item_generate(ItemGroups.basic_items)
--    ObjectUtils.spawn_item_near(player, item.type, item.amount)
--end

player:gain_xp(1000)

dofile("debug_scripts/getkeys.lua")
