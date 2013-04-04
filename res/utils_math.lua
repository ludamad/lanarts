
function vector_multiply(v1, v2)
    local length = # v1

    assert(length == #v2, "vector_multiply(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] * v2[i] end

    return ret
end

function vector_interpolate(v1, v2, percentage --[[0 results in v1, 1 results in v2, anything else is inbetween]])
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

function vector_divide(v1, v2)
    local length = # v1

    assert(length == #v2, "vector_divide(): Vector lengths do not match!")

    local ret = {}
    for i = 1,length do ret[i] = v1[i] / v2[i] end

    return ret
end

function origin_valid(origin)
    local rx, ry = unpack(origin)
    return rx >= 0 and rx <= 1 and ry >= 0 and ry <= 1
end

function bbox_padded(xy, size, pad) 
    return { xy[1] - pad, 
             xy[2] - pad, 
             xy[1] + size[1] + pad, 
             xy[2] + size[2] + pad }
end
