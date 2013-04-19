-- Functions that perform mathematical manipulations or checks

--- Given two numeric arrays of the same length, returns a new array with each of the elements multiplied together
function vector_multiply(v1, v2)
    local length = # v1

    assert(length == #v2, "vector_multiply(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] * v2[i] end

    return ret
end

--- Given two numeric arrays of the same length, returns a new array with the elements interpolated.
-- The resulting values are in between the original values.
-- @param v1 the first array
-- @param v2 the second array
-- @param percentage the amount to go towards v2. 0 results in v1, 1 results in v2, anything else is in-between.
function vector_interpolate(v1, v2, percentage)
    local length = # v1
    percentage = math.max( math.min(percentage, 1.0), 0.0)

    assert(length == #v2, "vector_interpolate(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do 
        local diff = v2[i] - v1[i]
        ret[i] = v1[i] + diff * percentage
    end

    return ret
end

--- Given two numeric arrays of the same length, returns a new array with the elements of the first array divided by the elements of the second.
function vector_divide(v1, v2)
    local length = # v1

    assert(length == #v2, "vector_divide(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] / v2[i] end

    return ret
end

--- Returns the position of an object of size 'size' aligned to a bounding box origin (eg, RIGHT_BOTTOM)
-- @param bbox the surrounding bounding box
-- @param origin the origin within the bounding box
-- @param obj_size <i>optional, default {0,0}</i> the size of the object
-- @param offset <i>optional, default {0,0}</i> a location offset to apply
function origin_aligned(bbox, origin, --[[Optional]] obj_size, --[[Optional]] offset)
    obj_size = obj_size or {0,0}
    offset = offset or {0,0}
    local bbox_w, bbox_h = bbox[3] - bbox[1], bbox[4] - bbox[2]
    return { ( bbox_w - obj_size[1] ) * origin[1] + bbox[1] + offset[1],
             ( bbox_h - obj_size[2] ) * origin[2] + bbox[2] + offset[2] }
end

--- Returns whether 'origin' represents a valid origin. 
-- A valid origin is a pair of numbers between 0 and 1, representing placement on a rectangle, eg LEFT_TOP is {0,0}.
function origin_valid(origin)
    local rx, ry = unpack(origin)
    return rx >= 0 and rx <= 1 and ry >= 0 and ry <= 1
end

-- Returns a bounding box given by 'xy' and 'size' padded by 'pad' in all directions.
function bbox_padded(xy, size, pad) 
    return { xy[1] - pad, 
             xy[2] - pad, 
             xy[1] + size[1] + pad, 
             xy[2] + size[2] + pad }
end