GameData = require "core.GameData"
Map = require "core.Map"

count_inv = (player, f) ->
    count = 0
    for i=0,math.huge
        slot = player\inventory_get(i)
        if not slot
            break
        if f(slot)
            count += 1
    return count

count_ally_invs = (player, f) ->
    count = 0
    for ally in *Map.players_on_team(player.team)
        print ally
        count += count_inv(ally, f)
    return count

post_pickup = (player, slot_idx) ->
    slot = player\inventory_get(slot_idx)
    item_data = GameData.item_definer(slot.type)
    if item_data.__method ~= "weapon_create" and item_data.__method ~= "equipment_create"
        print "RET1"
        return
    if item_data.type == "amulet" or item_data.type == "ring"
        print "RET2"
        return
    total_amount = count_ally_invs(player, (s) -> s.type == slot.type)
    print "RET3", total_amount
    if total_amount > #Map.players_on_team(player.team)
        print "RET4"
        player\inventory_sell(slot_idx)

return {
    :post_pickup
}
