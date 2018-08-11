-- To run, use !!items in in-game chat.

local ObjectUtils = require "objects.ObjectUtils"
local ItemUtils = require "maps.ItemUtils"
local player = require("core.World").local_player
local Bonuses = require "items.Bonuses"

for _, k in ipairs(Bonuses.BONUSES.__keys) do
    ObjectUtils.spawn_item_near(player, k .. " Mace", 1)
end
