--- Represents a linear group of maps, such as a deepening dungeon

local PortalSet = import ".PortalSet"
local GameMap = import "core.Map"
local MapSequence = newtype()

function MapSequence:init(--[[Optional]] args)
    self.maps = {}
    self.next_slot = 1
    args = args or {}
    if args.preallocate then
        for i=1,args.preallocate do
            self:slot_create() 
        end
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

function MapSequence:forward_portal_add(idx, portal, key, create_map_function)
    self:_slots_ensure(idx + 1)

    local forward_portals = self.maps[idx].forward_portals
    forward_portals:set_start(key, portal.xy)
    portal.on_player_interact = function(portal, user)
    
        self:slot_resolve(idx + 1, self:_get_map_id(idx+1) or create_map_function(portal, user))
        create_map_function = nil -- Serialization optimization
        print ("Transferring using portal '" .. (idx + 1) .. "', square = " .. pretty_tostring(forward_portals:get_end(key)))
        GameMap.transfer(user, self:_get_map_id(idx+1), forward_portals:get_end(key))
    end
end

function MapSequence:_get_map_id(idx)
    return self.maps[idx].map_id
end

function MapSequence:backward_portal_resolve(idx, portal, key)
    local back_portals = self.maps[idx].back_portals
    back_portals:set_end(key, portal.xy)
    portal.on_player_interact = function(portal, user)
        GameMap.transfer(user, self:_get_map_id(idx - 1), back_portals:get_start(key))
    end
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