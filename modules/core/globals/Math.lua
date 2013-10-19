-- Functions that perform mathematical manipulations or checks

--- Given two numeric arrays of the same length, returns a new array with each of the elements multiplied together
function vector_multiply(v1, v2)
    local length = # v1
    assert(length == #v2, "vector_multiply(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] * v2[i] end
    return ret
end

function vector_add(v1, v2)
    local length = # v1
    assert(length == #v2, "vector_add(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] + v2[i] end
    return ret
end

function vector_subtract(v1, v2)
    local length = # v1
    assert(length == #v2, "vector_subtract(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] - v2[i] end
    return ret
end

function vector_scale(v1, scale, --[[Optional]] floor_result)
    local ret = {}
    for i = 1,#v1 do 
        local result = v1[i] * scale
        if floor_result then
            result = math.floor(result)
        end
        ret[i] = result
    end
    return ret
end

function vector_distance(v1,v2)
    local length = # v1
    assert(length == #v2, "vector_distance(): Vector lengths do not match!")

    local sum = 0
    for i = 1,length do 
        sum = sum + (v2[i] - v1[i]) ^ 2 
    end
    return math.sqrt(sum)
end

function vector_sum(v)
    local sum = 0
    for i = 1,#v do
        sum = sum + v[i]
    end
    return sum
end

function vector_magnitude(v)
    local sum = 0
    for i = 1,#v do
        sum = sum + v[i]^2
    end
    return math.sqrt(sum)
end

function vector_to_direction(v)
    return math.atan2(v[2], v[1])
end

function vector_normalize(v, --[[Optional]] magnitude)
    magnitude = magnitude or 1
    return vector_scale(v, magnitude/vector_magnitude(v))
end

function vector_apply(f) 
    return function (v1, v2)
        local length = # v1
        assert(length == #v2, "vector_apply(): Vector lengths do not match!")

        local ret = {}
        for i = 1,length do ret[i] = f(v1[i], v2[i]) end
        return ret
    end
end

--- Calculates the sum t1[k] * t2[k] for every key 'k' that exists in t1 and t2  
function table.dot_product(t1, t2)
    local sum = 0
    for k,v1 in pairs(t1) do
        local v2 = t2[k]
        if v2 ~= nil then
            sum = sum + (v1 * v2)
        end
    end
    return sum
end

vector_max = vector_apply(math.max)
vector_min = vector_apply(math.min)

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

function random_round(num)
    local f = math.floor(num)
    if randomf(0,1) < (num-f) then
        return f + 1
    else
        return f
    end
end

--- Returns the position of an object of size 'size' aligned to a bounding box origin (eg, Display.RIGHT_BOTTOM)
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
-- A valid origin is a pair of numbers between 0 and 1, representing placement on a rectangle, eg Display.LEFT_TOP is {0,0}.
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

function is_position(xy_candidate)
    return type(xy_candidate) == "table" and not getmetatable(xy_candidate) and #xy_candidate == 2
end

function math.sign_of(v)
    if v > 0 then return 1 end
    if v == 0 then return 0 end
    return -1
end