--- Represents a linear group of maps, such as a deepening dungeon

local PortalSet = require "maps.PortalSet"
local Map = require "core.Map"
local Closure = require "Closure"
local MapSequence = newtype()

function MapSequence:init(--[[Optional]] args)
    self.maps = {}
    self.next_slot = 1
    args = args or {}
    for i=1,args.preallocate or 0 do
        self:slot_create() 
    end
end

function MapSequence:_slots_ensure(amount)
    local current_amount = #self.maps
    for idx=current_amount+1, amount do
        local back_portals = nil
        if idx > 1 then 
            back_portals = self.maps[idx-1].forward_portals 
        end
        self.maps[idx] = { back_portals = back_portals, forward_portals = PortalSet.create() }
    end
end

function MapSequence:slot_create()
    local idx = #self.maps + 1
    self:_slots_ensure(self.next_slot)
    self.next_slot = self.next_slot + 1
    return self.next_slot - 1
end

function MapSequence._callback_forward_portal_on_player_interact(args, portal, user) 
    local self, idx, create_map_function, forward_portals, key = unpack(args)
    self:slot_resolve(idx + 1, self:_get_map_id(idx+1) or create_map_function(portal, user))
    print ("Transferring using portal '" .. (idx + 1) .. "', square = " .. pretty_tostring(forward_portals:get_end(key)))
    Map.transfer(user, self:_get_map_id(idx+1), forward_portals:get_end(key))
end

function MapSequence:forward_portal_add(idx, portal, key, create_map_function)
    self:_slots_ensure(idx + 1)
    local forward_portals = self.maps[idx].forward_portals
    forward_portals:set_start(key, portal.xy)
    local on_player_interact_args = {self, idx, create_map_function, forward_portals, key}
    portal.on_player_interact = Closure.create(on_player_interact_args, MapSequence._callback_forward_portal_on_player_interact)
    return function() 
        self:slot_resolve(idx + 1, self:_get_map_id(idx+1) or create_map_function(portal))
    end
end

function MapSequence:_get_map_id(idx)
    return self.maps[idx].map_id
end

function MapSequence._callback_backward_portal_on_player_interact(args, portal, user) 
    local self, idx, back_portals, key = unpack(args)
    for k,v in pairs(self) do
        print(k,v)
    end
    Map.transfer(user, self:_get_map_id(idx - 1), back_portals:get_start(key))
end

function MapSequence:backward_portal_resolve(idx, portal, key)
    local back_portals = self.maps[idx].back_portals
    back_portals:set_end(key, portal.xy)
    local on_player_interact_args = {self, idx, back_portals, key}
    portal.on_player_interact = Closure.create(on_player_interact_args, MapSequence._callback_backward_portal_on_player_interact)
end

function MapSequence:number_of_backward_portals(idx)
    local back_portals = self.maps[idx].back_portals
    if not back_portals then return 0 end
    return back_portals.size
end

function MapSequence:slot_resolve(idx, map_id)
    print("Got slot resolve ", idx, " for map ", map_id)
    self.maps[idx].map_id = map_id
    return map_id
end

function MapSequence.get:size()
    return #self.maps
end

return MapSequence
