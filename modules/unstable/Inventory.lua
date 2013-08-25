local Inventory = newtype()

function Inventory:init()
    self.items = {}
end

function Inventory:__copy(other)
    -- Ensure a shallow copy when used with table.deep_copy
    table.copy(self, other)
end
