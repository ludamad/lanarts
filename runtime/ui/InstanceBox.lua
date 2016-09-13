-- Places instances on a certain origin, an optional offset can also be specified

local InstanceGroup = require "ui.InstanceGroup"

--- A layout consisting of objects placed on a grid. 
-- Each object can be placed with an origin, as well as an optional offset.
-- Objects that are stored should have 'step' and 'draw' methods that take a position, as well as a 'size' member.
-- @usage InstanceBox.create({ size = {640,480} })
local InstanceBox = newtype()

--- Initializes a new InstanceBox.
-- @usage InstanceBox.create({ size = {640,480} })
function InstanceBox:init(params)
    self._instances = InstanceGroup.create()
    self.size = params.size
end

--- Calls step on all contained objects.
function InstanceBox:step(xy) 
    self._instances:step(xy)
end

--- Calls draw on all contained objects.
function InstanceBox:draw(xy)
    self._instances:draw(xy)
    DEBUG_BOX_DRAW(self, xy)
end

--- Removes the contained object 'obj'.
function InstanceBox:remove(obj)
    self._instances:remove(obj)
end

--- Removes all contained objects.
function InstanceBox:clear()
    self._instances:clear()
end

--- Return an iterable that iterates over all objects and their positions.
-- @param xy <i>optional, default {0,0}</i>
-- @usage for obj, xy in instance_box:instances({100,100}) do ... end
function InstanceBox:instances( --[[Optional]] xy)
    return self._instances:instances(xy)
end

--- Add an object to this container.
-- @param origin the origin to align against, eg Display.LEFT_TOP, Display.RIGHT_BOTTOM.
-- @param offset <i>optional, default {0,0}</i> a position offset for the object.
function InstanceBox:add_instance(obj, origin, --[[Optional]] offset)
    assert( origin_valid(origin), "InstanceBox: Expected origin coordinates between [0,0] and [1,1]") 

    offset = offset or {0, 0}

    local self_w, self_h = unpack(self.size)
    local obj_w, obj_h = unpack(obj.size)

    local xy = { ( self_w - obj_w ) * origin[1] + offset[1],
                 ( self_h - obj_h ) * origin[2] + offset[2] }

    self._instances:add_instance( obj, xy )
end

function InstanceBox:readd_instance(obj, origin, --[[Optional]] offset)
    self:remove(obj)
    self:add_instance(obj, origin, offset)
end

--- Whether the mouse is within the InstanceBox.
function InstanceBox:mouse_over(xy)
    return mouse_over(xy, self.size)
end

--- A simple string representation for debugging purposes.
function InstanceBox:__tostring()
    return "[InstanceBox " .. toaddress(self) .. "]"
end

return InstanceBox