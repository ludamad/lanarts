-- Places instances on a certain origin, an optional offset can also be specified

require "utils"
require "InstanceGroup"

InstanceBox = newtype()

function InstanceBox:init(params)
    self._instances = InstanceGroup.create()
    self.size = params.size
end

function InstanceBox:step(xy) 
    self._instances:step(xy)
end

function InstanceBox:draw(xy)
    self._instances:draw(xy)
    DEBUG_BOX_DRAW(self, xy)
end

function InstanceBox:remove(obj)
    self._instances:remove(obj)
end

function InstanceBox:clear()
    self._instances:clear()
end

function InstanceBox:instances(xy)
    return self._instances:instances(xy)
end

function InstanceBox:add_instance(obj, origin, --[[Optional]] offset)
    assert( origin_valid(origin), "InstanceBox: Expected origin coordinates between [0,0] and [1,1]") 

    offset = offset or {0, 0}

    local self_w, self_h = unpack(self.size)
    local obj_w, obj_h = unpack(obj.size)

    local xy = { ( self_w - obj_w ) * origin[1] + offset[1],
                 ( self_h - obj_h ) * origin[2] + offset[2] }

    self._instances:add_instance( obj, xy )
end

function InstanceBox:mouse_over(xy)
    return mouse_over(xy, self.size)
end

function InstanceBox:__tostring()
    return "[InstanceBox " .. toaddress(self) .. "]"
end