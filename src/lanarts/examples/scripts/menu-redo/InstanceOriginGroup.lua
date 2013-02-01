-- Places instances on a certain origin, an optional offset can also be specified

require "utils"
require "InstanceGroup"

InstanceOriginGroup = newtype()

function InstanceOriginGroup:init(params)
    self._instances = InstanceGroup.create()
    self.size = params.size
end

function InstanceOriginGroup:step(xy) 
    self._instances:step(xy)
end

function InstanceOriginGroup:draw(xy)
    self._instances:draw(xy)
    DEBUG_BOX_DRAW(self, xy)
end

function InstanceOriginGroup:instances(xy)
    return self._instances:instances(xy)
end

function InstanceOriginGroup:add_instance(obj, origin, --[[Optional]] offset)
    assert( origin_valid(origin), "InstanceOriginGroup: Expected origin coordinates between [0,0] and [1,1]") 

    offset = offset or {0, 0}

    local self_w, self_h = unpack(self.size)
    local obj_w, obj_h = unpack(obj.size)

    local xy = { ( self_w - obj_w ) * origin[1] + offset[1],
                 ( self_h - obj_h ) * origin[2] + offset[2] }

    self._instances:add_instance( obj, xy )
end

function InstanceOriginGroup:__tostring()
    return "[InstanceOriginGroup " .. toaddress(self) .. "]"
end