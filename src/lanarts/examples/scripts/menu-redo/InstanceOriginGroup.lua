-- Places instances on a certain origin 

require "utils"
require "InstanceGroup"

InstanceOriginGroup = newtype()

function InstanceOriginGroup:init(size)
    self.instances = InstanceGroup.create()
    self.size = size
end

function InstanceOriginGroup:step(xy) 
    self.instances:step()
end

function InstanceOriginGroup:draw(xy) 
    self.instances:draw()
    DEBUG_BOX_DRAW(self, xy)
end

function InstanceOriginGroup:add_instance(obj, origin)
    assert( origin_valid(origin), "InstanceOriginGroup: Expected origin coordinates between [0,0] and [1,1]")  

    local xy = { ( self.size[1] - obj.size[1] ) * origin[1], 
                 ( self.size[2] - obj.size[2] ) * origin[2] }

    self.instances:add_instance( obj, xy )
end

function InstanceOriginGroup:__tostring()
    return "[InstanceOriginGroup]"
end