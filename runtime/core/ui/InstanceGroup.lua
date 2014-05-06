
-- Keeps track of instances + relative positions
-- Tries to store & resolve relative positions in a somewhat efficient manner

--- A layout utility that stores relative positions of objects.
-- Each object can be placed with an origin, as well as an optional offset.
-- Objects that are stored should have 'step' and 'draw' methods that take a position.
-- @usage InstanceGroup.create()
local InstanceGroup = newtype()

--- Initializes a new instance group. 
-- @usage InstanceGroup.create()
function InstanceGroup:init()
    self._instances = {}
end

--- Add an object to this container.
-- @param xy the origin to align against, eg Display.LEFT_TOP, Display.RIGHT_BOTTOM.
function InstanceGroup:add_instance(obj, xy )
    self._instances[#self._instances + 1] = { obj, xy }
end

--- Calls step on all contained objects.
function InstanceGroup:step(xy)
    local step_xy = {} -- shared array for performance

    for instance in values(self._instances) do
        local obj, obj_xy = unpack(instance)

        step_xy[1] = obj_xy[1] + xy[1]
        step_xy[2] = obj_xy[2] + xy[2]

        obj:step(step_xy)
    end
end

--- Return an iterable that iterates over all objects and their positions.
-- @param xy <i>optional, default {0,0}</i>
-- @usage for obj, xy in instance_group:instances({100,100}) do ... end
function InstanceGroup:instances(xy)
    xy = xy or {0,0}

    local adjusted_xy = {} -- shared array for performance

    local arr,idx = self._instances,1

    -- Iterate the values in a fairly future-proof way, via a helper closure
    return function() 
        local val = arr[idx]

        if val == nil then
            return nil 
        end

        idx = idx + 1
        local obj, obj_xy = unpack(val)
        
        adjusted_xy[1] = obj_xy[1] + xy[1]
        adjusted_xy[2] = obj_xy[2] + xy[2]

        return obj, adjusted_xy
    end
end

--- Removes the contained object 'obj'.
function InstanceGroup:remove(obj)
    local insts = self._instances

    for i = 1, #insts do
        if insts[i][1] == obj then
            table.remove(insts, i)
            return true
        end
    end

    return false
end

--- Removes all contained objects.
function InstanceGroup:clear()
    self._instances = {}
end

--- Calls draw on all contained objects.
function InstanceGroup:draw(xy)
    local draw_xy = {} -- shared array for performance

    for _, instance in ipairs(self._instances) do
        local obj, obj_xy = unpack(instance)

        draw_xy[1] = obj_xy[1] + xy[1]
        draw_xy[2] = obj_xy[2] + xy[2]

        obj:draw(draw_xy)
    end
end

--- A simple string representation for debugging purposes.
function InstanceGroup:__tostring()
    return "[InstanceGroup " .. toaddress(self) .. "]"
end
return InstanceGroup
