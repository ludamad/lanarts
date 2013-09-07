local MapGen = import "core.MapGeneration"
local MapUtils = import ".MapUtils"

local M = nilprotect {} -- Submodule

-- Each layout takes up a rectangular area
-- Can be connected ... how ?
-- Need to draw a line from R1 to R2
-- Draw in line segments

-- Find a line (not line segment) that intersects two regions
local function find_intersecting_line(map, region1, region2)
    return {
        MapUtils.random_square(map, region1),
        MapUtils.random_square(map, region2)
    }
end

function M.brute_tunnel(map, region1, region2, args) 
    args = args or {}
    local interval_length = args.interval_length or 4 
    local line_width = args.line_width or 4
    local line = find_intersecting_line(map, region1, region2)
    MapGen.line_apply {
        map = map,
        line_width = line_width,
        from_xy = line[1], to_xy = line[2],
        operator = args.operator
    }
end

--region1, region2, operator, continue_selector, finished_selector, --[[Optional]] interval_length)

return M