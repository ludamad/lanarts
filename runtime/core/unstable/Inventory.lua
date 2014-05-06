local StatContext = import "@StatContext"
local Proficiency = import "@Proficiency"
local SlotUtils = import "@SlotUtils"
local Actions = import "@Actions"
local ItemType = import "@ItemType"

local Inventory = newtype()

local INVENTORY_CAPACITY = 2500

function Inventory:init(--[[Optional]] capacity)
    self.items = {}
    self.capacity = capacity or INVENTORY_CAPACITY
end

function Inventory:find_item(item_type)
    for idx,item in ipairs(self.items) do
        if item_type == item.type then
            return item,idx
        end
    end
    return nil
end

-- Returns whether there was room to add the item
function Inventory:add_item(item_slot)
    -- Resolve item slot
    if type(item_slot) == "string" or not getmetatable(item_slot) then 
        if not item_slot.type then item_slot = {type = item_slot} end
        item_slot = Inventory.item_slot_create(item_slot) 
    end
    if item_slot.stackable then
        local item = self:find_item(item_slot.type)
        if item then
            item.amount = item.amount + (item_slot.amount or 1)
            return item
        end
    end

    if #self.items < self.capacity then
        table.insert(self.items, item_slot)
        return item_slot
    end
    return nil
end

local function remove_from_slot(inventory, item_slot, --[[Optional]] amount)
    item_slot.amount = item_slot.amount - (amount or 1)
    if item_slot.amount <= 0 then 
        table.remove_occurrences(inventory.items, item_slot)
    end
end

function Inventory:use_item(user, item_slot, --[[Optional]] target)
    Actions.use_action(user, item_slot.action_use, target, item_slot)
    remove_from_slot(self, item_slot, 1)
end

function Inventory:can_use_item(user, item_slot, --[[Optional]] target)
    return Actions.can_use_action(user, item_slot.action_use, target, item_slot)
end

function Inventory:get_equipped_items(equipment_type)
    local items = {}
    for _, item in ipairs(self.items)  do
        if item.equipped and table.contains(item.traits, equipment_type) then
            table.insert(items, item)
        end
    end
    return items
end

function Inventory:get_equipped_item(equipment_type)
    local items = self:get_equipped_items(equipment_type)
    assert(#items <= 1)
    return items[1]
end

function Inventory:remove_item(item_slot, --[[Optional]] amount)
    remove_from_slot(self, item_slot, amount)
end

function Inventory:equip_item(item_slot)
    item_slot.equipped = true
end

function Inventory:deequip_item(item_slot)
    item_slot.equipped = false
end

function Inventory:values()
    return values(self.items)
end

function Inventory:on_step(stats)
    for _, item in ipairs(self.items)  do
        if item.equipped then
            local on_step = item.type.on_step
            if on_step then
                on_step(item, stats)
            end
        end
    end
end

function Inventory:on_event(method_name, ...)
    for _, item in ipairs(self.items)  do
        if item.equipped then
            local method = item[method_name]
            if method then method(item, ...) end
        end
    end
end

function Inventory:on_calculate(user)
    for _, item in ipairs(self.items)  do
        if item.equipped and item.on_calculate then
            item:on_calculate(user)
        end
    end
end

function Inventory:get_on_draw_methods(func_list)
    for _, item in ipairs(self.items)  do
        if item.equipped and item.on_draw then
            table.insert(func_list, item)
            table.insert(func_list, item.on_draw)
        end
    end
end

function Inventory:calculate_proficiency_modifier(stats, item_slot)
    local fail, total = Proficiency.resolve_proficiency_requirements(item_slot.type.proficiency_requirements, stats)
    if total == 0 then return 0.0 end
    return fail/total
end

function Inventory.item_slot_create(args)
    if _G.type(args.type) == "string" then 
        args.type = ItemType.lookup(args.type)
    end
    local type = args.type
    assert(type)
    args.amount = args.amount or 1
    args.equipped = args.equipped or false
    setmetatable(args, SlotUtils.METATABLE)

    if type.on_map_init then
       type.on_map_init(args)
    end
    return args
end

return Inventory
