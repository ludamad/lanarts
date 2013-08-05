--- Represents a linear group of maps, such as a deepening dungeon

local PortalSet = import ".PortalSet"
local Maps = import "core.maps"
local MapSequence = newtype()

function MapSequence:init(args)
    self.maps = {}
    if args.preallocate then
        for i=1,args.preallocate do
            self:slot_create() 
        end
    end
end

function MapSequence:slot_create()
    local idx = #self.maps + 1
    local back_portals = nil
    if idx > 1 then 
        back_portals = self.maps[idx-1].forward_portals 
    end
    self.maps[idx] = { back_portals = back_portals, forward_portals = PortalSet.create() }
    return idx
end

function MapSequence:forward_portal_add(idx, portal, key, create_map_function)
    local forward_portals = self.maps[idx].forward_portals
    forward_portals:set_start(key, portal.xy)
    portal.on_player_interact = function(portal, user)
        self:slot_resolve(idx + 1, self:_get_map_id(idx+1) or create_map_function(portal, user))
        create_map_function = nil -- Serialization optimization
        Maps.transfer(user, self:_get_map_id(idx+1), forward_portals:get_end(key))
    end
end

function MapSequence:_get_map_id(idx)
    return self.maps[idx].map_id
end

function MapSequence:backward_portal_resolve(idx, portal, key)
    local back_portals = self.maps[idx].back_portals
    back_portals:set_end(key, portal.xy)
    portal.on_player_interact = function(portal, user)
        Maps.transfer(user, self:_get_map_id(idx - 1), back_portals:get_start(key))
    end
end

function MapSequence:slot_resolve(idx, map_id)
    print("Got slot resolve ", idx, " for map ", map_id)
    self.maps[idx].map_id = map_id
    return map_id
end

return MapSequence