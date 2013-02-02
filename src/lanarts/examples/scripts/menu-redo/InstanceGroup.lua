
-- Keeps track of instances + relative positions
-- Tries to store & resolve relative positions in a somewhat efficient manner

InstanceGroup = newtype()

function InstanceGroup:init()
    self._instances = {}
end

-- Requires 'obj' to have 'size' member if 'origin' is specified
function InstanceGroup:add_instance(obj, xy )
    self._instances[#self._instances + 1] = { obj, xy }
end

function InstanceGroup:step(xy)
    local step_xy = {} -- shared array for performance

    for instance in values(self._instances) do
        local obj, obj_xy = unpack(instance)

        step_xy[1] = obj_xy[1] + xy[1]
        step_xy[2] = obj_xy[2] + xy[2]

        obj:step(step_xy)
    end
end

-- Iterate the values in a fairly implementation-change-proof way
function InstanceGroup:instances(xy)
    xy = xy or {0,0}

    local adjusted_xy = {} -- shared array for performance

    local iter = values(self._instances)

    return function() 
        local val = iter()

        if val == nil then
            return nil 
        end
        
        local obj, obj_xy = unpack(val)
        
        adjusted_xy[1] = obj_xy[1] + xy[1]
        adjusted_xy[2] = obj_xy[2] + xy[2]

        return obj, adjusted_xy
    end
end

function InstanceGroup:remove(obj)
    local insts = self._instances

    for i = 1, #insts do
        if insts[i] == obj then
            table.remove(insts, i)
            return true
        end
    end

    return false
end

function InstanceGroup:clear()
    self._instances = {}
end

function InstanceGroup:draw(xy)
    local draw_xy = {} -- shared array for performance

    for instance in values(self._instances) do
        local obj, obj_xy = unpack(instance)

        draw_xy[1] = obj_xy[1] + xy[1]
        draw_xy[2] = obj_xy[2] + xy[2]

        obj:draw(draw_xy)
    end
end


function InstanceGroup:__tostring()
    return "[InstanceGroup " .. toaddress(self) .. "]"
end