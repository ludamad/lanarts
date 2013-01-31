-- Organizes instances into rows & columns

require "InstanceGroup"

InstanceGridGroup = newtype( )

function InstanceGridGroup:init(parameters)
    self.instances = InstanceGroup.create()

    self.size = parameters.size
    self.dimensions = parameters.dimensions

    self.position = {0, 0}
end

function InstanceGridGroup:step(xy) 
    self.instances:step(xy)
end

function InstanceGridGroup:draw(xy)
    self.instances:draw(xy)
    DEBUG_BOX_DRAW(self, xy)
end

-- Moves the location to the next along the grid
function InstanceGridGroup:skip_location()
    local cols, rows = unpack(self.dimensions)
    local nx, ny = unpack(self.position)

    nx = nx + 1
    if nx < cols then
        self.position[1] = nx
    else
        self.position[1] = 0
        self.position[2] = ny + 1
    end
end

function InstanceGridGroup:add_instance(obj, _)
    assert(_ == nil, "InstanceGridGroup:add_instance takes only one argument!")

    local w, h = unpack(self.size)
    local cols, rows = unpack(self.dimensions)
    local nx, ny = unpack(self.position)

    self.instances:add_instance( obj, { (w/cols) * nx, (h/rows) * ny } )

    self:skip_location()
end

function InstanceGridGroup:__tostring()
    return "[InstanceGridGroup]"
end
