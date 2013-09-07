local Inventory = newtype()

local INVENTORY_CAPACITY = 25

function Inventory:init(--[[Optional]] capacity)
    self.items = {}
    self.capacity = capacity or INVENTORY_CAPACITY
end

function Inventory:__copy(other)
    -- Ensure a shallow copy when used with table.deep_copy
    other.items = rawget(other, "items") or {}
    table.copy(self.items, other.items)
    other.capacity = self.capacity
end

function Inventory:find_item(type, --[[Optional]] modifiers)
    for idx,item in ipairs(self.items) do
        if item.type == type and item.modifiers == modifiers then
            return item,idx
        end
    end
    return nil
end

-- Returns whether there was room to add the item
function Inventory:add_item(type, --[[Optional]] amount, --[[Optional]] modifiers)
    local item = self:find_item(type, modifiers)
    if item then
        item.amount = item.amount + (amount or 1)
        return true
    end

    if #self.items < self.capacity then
        table.insert(self.items, Inventory.item_create(type, amount, modifiers))
        return true
    end

    return false
end

local function remove_from_slot(inventory, item, idx, --[[Optional]] amount)
    item.amount = item.amount - (amount or 1)
    if item.amount <= 0 then 
        table.remove(inventory.items, idx) 
    end
end

function Inventory:use_item(user, type, --[[Optional]] modifiers)
    local item, idx = self:find_item(type, modifiers)
    if item then
        item.type.on_use(item, user)
        remove_from_slot(self, item, idx)
    end
    assert(item, "Inventory did not contain 1 of " .. type.name)
end

function Inventory:remove_item(type, --[[Optional]] amount, --[[Optional]] modifiers)
    local item, idx = self:find_item(type, modifiers)
    if item then
        remove_from_slot(self, item, idx, amount)
    end
    assert(item, "Inventory did not contain " .. amount .. " of " .. type.name)
end

function Inventory:values()
    return values(self.items)
end

function Inventory.item_create(type, --[[Optional]] amount, --[[Optional]] modifiers)
    return {
        type = type,
        amount = amount or 1,
        modifiers = modifiers,
        equipped = false
    }
end

return Inventory