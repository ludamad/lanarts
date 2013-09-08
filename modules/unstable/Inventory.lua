local StatContext = import "@StatContext"

local Inventory = newtype()

local INVENTORY_CAPACITY = 25

function Inventory:init(--[[Optional]] capacity)
    self.items = {}
    self.capacity = capacity or INVENTORY_CAPACITY
end

function Inventory:__copy(other)
    -- Ensure a shallow copy when used with table.deep_copy
    other.items = rawget(other, "items") or {}
    for i=1,#self.items do
        other.items[i] = table.clone(self.items[i])
    end
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
        return item
    end

    if #self.items < self.capacity then
        local item = Inventory.item_create(type, amount, modifiers)
        table.insert(self.items, item)
        return item
    end

    return nil
end

local function remove_from_slot(inventory, item_slot, --[[Optional]] amount)
    item_slot.amount = item_slot.amount - (amount or 1)
    if item_slot.amount <= 0 then 
        table.remove_occurences(inventory.items, item_slot)
    end
end

function Inventory:use_item(user, item_slot)
    local to_remove = item_slot.type.on_use(item_slot, user)
    remove_from_slot(self, item_slot, to_remove or 1)
end

function Inventory:get_equipped_items(equipment_type)
    local items = {}
    for item in self:values() do
        if item.equipped and table.contains(item.type.traits, equipment_type) then
            table.insert(items, item)
        end
    end
    return items
end

function Inventory:get_equipped_item(...)
    local items = self:get_equipped_items(...)
    assert(#items <= 1)
    return items[1]
end

function Inventory:remove_item(item_slot, --[[Optional]] amount)
    remove_from_slot(self, item_slot, amount)
end

function Inventory:equip_item(item_slot)
    item_slot.equipped = true
end

function Inventory:values()
    return values(self.items)
end

function Inventory:on_step(stats)
    for item in self:values() do
        if item.equipped then
            local on_step = item.type.on_step
            if on_step then
                on_step(item, stats)
            end
        end
    end
end

function Inventory:on_calculate(stats)
    for item in self:values() do
        if item.equipped then
            local bonuses = item.type.equipment_bonuses 
            if bonuses then
                StatContext.temporary_add(stats, bonuses)
            end
            if item.modifiers and item.modifiers.equipment_bonuses then
                StatContext.temporary_add(stats, item.modifiers.equipment_bonuses)
            end
            local on_calc = item.type.on_calculate
            if on_calc then
                on_calc(item, stats)
            end
        end
    end
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