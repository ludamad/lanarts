local M = nilprotect {} -- Submodule

function M.clip(area, xy) 
    return {math.max(area[1], math.min(xy[1], area[3])), math.max(area[2], math.min(xy[2], area[4]))}
end

function M.random_polygon(area, xy, magnitude_range, num_points)
    local r = {}
    local iter = math.pi * 2 / num_points
    local angle = randomf(0, math.pi*2)
    for i=1,num_points do
        local mag = randomf(magnitude_range[1], magnitude_range[2])
        r[i] = M.clip(area, vector_add(xy, {mag * math.sin(angle), mag * math.cos(angle)}))
        angle = angle + randomf(iter*.5, iter*1.5)
    end
    return r
end

return M