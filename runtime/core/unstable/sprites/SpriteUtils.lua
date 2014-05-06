local M = nilprotect {} -- Submodule

local WeightedDirectional = newtype()

function WeightedDirectional:init(image_ranges)
	self.image_ranges = image_ranges
end

-- Force angles within [0, 2*pi)
local function angle_wrap(angle)
    return math.fmod(angle + math.pi * 2, math.pi * 2)
end

function WeightedDirectional:_resolve_image(angle)
    angle = angle_wrap(angle)
    for _, range in ipairs(self.image_ranges) do
        local img, min_angle, max_angle = range[1], range[2], range[3]
        local meets_neg = false
        if min_angle < 0 and max_angle > 0 then -- Special case
            meets_neg = (angle > min_angle + 2 * math.pi)
        end
        if meets_neg or (angle >= min_angle and angle <= max_angle) then
            return img
        end
    end
    error("Could not find image for angle " .. angle)
end

function WeightedDirectional:draw(options, xy)
    if not xy then -- xy is the first argument if called with only one argument
        xy = options
        options = _EMPTY_TABLE
    end
    local subimage = self:_resolve_image(options.angle or 0)
    subimage:draw(options, xy)
end

function M.weighted_directional_create(images, weights, --[[Optional]] angle_offset)
    angle_offset = angle_offset or 0

    local total_weight, interval = vector_sum(weights), (2 * math.pi)/ #images

    local ranges = {}
    for i, img in ipairs(images) do
        local weight = assert(weights[i], "Must have a weight for each image!")
        local ang = angle_wrap(interval * (i-1) - angle_offset)
        local width = 2 * math.pi * weight / total_weight
        table.insert(ranges, {img, ang - width/2, ang + width/2}) 
    end

    return WeightedDirectional.create(ranges)    
end

return M
