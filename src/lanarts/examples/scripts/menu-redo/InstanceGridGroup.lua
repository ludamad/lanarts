-- Organizes instances into rows & columns

require "InstanceGroup"

InstanceGridGroup = newtype( )

local function two_of_three_exist(a,b,c)
    local sum = (a and 1 or 0) + (b and 1 or 0) + (c and 1 or 0)
    return sum >= 2
end

-- Must be provided 2 of the 3 values 'size, dimensions, spacing'
function InstanceGridGroup:init(params)
    self._instances = InstanceGroup.create()

    self.position = {0, 0}

    assert( two_of_three_exist(params.size, params.dimensions, params.spacing), 
        "InstanceGridGroup.create must be provided at least 2 of the 3 values 'size, dimensions, spacing'" )

    self.size = params.size
    self.dimensions = params.dimensions
    self.spacing = params.spacing

    if not params.size then 
        self.size       = vector_multiply(self.dimensions, self.spacing)
    elseif not params.dimensions then 
        self.dimensions = { math.ceil( self.size[1] / self.spacing[1] ), 
                            math.ceil( self.size[2] / self.spacing[2] ) } 
    elseif not params.spacing then
        self.spacing    = vector_divide(self.size, self.dimensions) 
    end

end

function InstanceGridGroup:step(xy) 
    self._instances:step(xy)
end

function InstanceGridGroup:draw(xy)
    self._instances:draw(xy)
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

function InstanceGridGroup:instances(xy)
    return self._instances:instances(xy)
end

function InstanceGridGroup:add_instance(obj, --[[Optional]] offset)

    local x, y = unpack(offset or {0,0} )
    local dx, dy = unpack(self.spacing)
    local row, col = unpack(self.position)

    self._instances:add_instance( obj, { x + row * dx, y + col * dy } )

    self:skip_location()
end

function InstanceGridGroup:__tostring()
    return "[InstanceGridGroup " .. toaddress(self) .. "]"
end