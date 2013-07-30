local ItemEntry = newtype()

function ItemEntry:init(type, bonuses, amount)
    self.type = type
    self.amount = amount or 1
    self.bonuses = bonuses
end

function ItemEntry:on_step(holder, derived_stats)
    
end

return ItemEntry
