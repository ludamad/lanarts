local GameData = require("core.GameData")
local Map = require("core.Map")
local count_inv
count_inv = function(player, f)
  local count = 0
  for i = 0, math.huge do
    local slot = player:inventory_get(i)
    if not slot then
      break
    end
    if f(slot) then
      count = count + 1
    end
  end
  return count
end
local count_ally_invs
count_ally_invs = function(player, f)
  local count = 0
  local _list_0 = Map.players_on_team(player.team)
  for _index_0 = 1, #_list_0 do
    local ally = _list_0[_index_0]
    count = count + count_inv(ally, f)
  end
  return count
end
local post_pickup
post_pickup = function(player, slot_idx)
  local slot = player:inventory_get(slot_idx)
  local item_data = GameData.item_definer(slot.type)
  if item_data.__method ~= "weapon_create" and item_data.__method ~= "equipment_create" then
    return
  end
  if item_data.type == "amulet" or item_data.type == "ring" then
    return
  end
  local total_amount = count_ally_invs(player, function(s)
    return s.type == slot.type
  end)
  if total_amount > #Map.players_on_team(player.team) then
    return player:inventory_sell(slot_idx)
  end
end
return {
  post_pickup = post_pickup
}
