local Item = newtype()

function Item:init(type, amount, bonuses)
	self.type = type
	self.amount = amount
	self.bonuses = bonuses
end

function Item:on_step()

end

return Item
