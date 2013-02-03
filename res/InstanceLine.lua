-- Organizes instances into rows & columns

require "InstanceGroup"

InstanceLine = newtype( )

function InstanceLine:init(params)
    assert( params.dx,
        "InstanceLine.create expects a 'dx' member to indicate spacing of elements.")

    assert( (params.per_row and params.dy) or (not params.per_row and not params.dy),
        "InstanceLine.create expects 'per_row' and 'dy' to be passed together, or not at all.")

    self._instances = InstanceGroup.create()

    self.position = {0, 0}
    self.size =  params.force_size or {0, 0}

    self.dx = params.dx
    self.dy = --[[Optional]] params.dy or 0

    self.force_size = --[[Optional]] params.force_size or false
    self.per_row = --[[Optional]] params.per_row or false
end

function InstanceLine:clear()
    self._instances:clear()
    self.position = {0, 0}

    if not self.force_size then
        self.size = {0,0}
    end
end

function InstanceLine:remove(obj)
    self._instances:remove(obj)
end

function InstanceLine:step(xy) 
    self._instances:step(xy)
end

function InstanceLine:draw(xy)
    self._instances:draw(xy)
    DEBUG_BOX_DRAW(self, xy)
end

-- Moves the next location
function InstanceLine:skip_location()
    local nx, ny = unpack(self.position)

    nx = nx + 1

    if self.per_row and nx >= self.per_row then
        self.position[1] = 0
        self.position[2] = ny + 1
    else
        self.position[1] = nx
    end
end

function InstanceLine:instances(xy)
    return self._instances:instances(xy)
end

function InstanceLine:add_instance(obj, --[[Optional]] offset)

    local x, y = unpack(offset or {0,0})
    local row, col = unpack(self.position)
    local subx, suby = row * self.dx, col * self.dy

    self._instances:add_instance( obj, {x + subx, y + suby} )

    if not self.force_size then
        self.size[1] = math.max(self.size[1], subx + obj.size[1] )
        self.size[2] = math.max(self.size[2], suby + obj.size[2] )
    end

    self:skip_location()
end

function InstanceLine:mouse_over(xy)
    return mouse_over(xy, self.size)
end

function InstanceLine:__tostring()
    return "[InstanceLine " .. toaddress(self) .. "]"
end