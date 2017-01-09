-- To run, use !!getkeys in in-game chat.

local ObjectUtils = require "objects.ObjectUtils"
local ItemUtils = require "maps.ItemUtils"
local player = require("core.World").local_player

ObjectUtils.spawn_item_near(player, "Azurite Key", 1)
ObjectUtils.spawn_item_near(player, "Dandelite Key", 1)
