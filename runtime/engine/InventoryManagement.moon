GameData = require "core.GameData"

post_pickup = (player, slot_idx) ->
    slot = player\inventory_get(slot_idx)
    entry = GameData.item_definer(slot.type)
    pretty slot.type
    pretty entry

return {
    :post_pickup
}
