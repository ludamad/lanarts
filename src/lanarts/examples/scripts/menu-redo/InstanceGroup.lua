
-- Keeps track of instances + relative positions
-- Tries to store & resolve relative positions in a somewhat efficient manner

InstanceGroup = newtype()

function InstanceGroup:init()
    self.instances = {}
end

function InstanceGroup.get:size() 
    error("InstanceGroup does not have a size!") 
end

-- Requires 'obj' to have 'size' member if 'origin' is specified
function InstanceGroup:add_instance(obj, xy )
    self.instances[#self.instances + 1] = { obj, unpack(xy) }
end

function InstanceGroup:step(xy)
    xy = xy or {0,0}

    local step_xy = {} -- shared array for performance

    for instance in values(self.instances) do
        local obj, x, y = unpack(instance)

        step_xy[1] = x + xy[1]
        step_xy[2] = y + xy[2]

        obj:step(step_xy)
    end
end

function InstanceGroup:draw(xy)
    xy = xy or {0,0}

    local draw_xy = {} -- shared array for performance

    for instance in values(self.instances) do
        local obj, x, y = unpack(instance)

        draw_xy[1] = x + xy[1]
        draw_xy[2] = y + xy[2]

        obj:draw(draw_xy)
    end
end