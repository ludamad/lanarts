MapUtils = require "maps.MapUtils"
SourceMap = require "core.SourceMap"

shuffle = (rng, tbl) ->
  for i=#tbl,1,-1 do
      rand = rng\random(1, #tbl + 1)
      tbl[i], tbl[rand] = tbl[rand], tbl[i]
  return tbl

shuffle_cpy = (rng, tbl) -> shuffle(rng, table.clone(tbl))

-- Find a random bbox in a set of regions.
-- The 'seed_selector' determines properties of the top left square.
-- Returns the region it was placed in, and the bbox.
find_bbox = (region_set, size, perimeter={}, n_attempts=100) ->
    {:map, :regions} = region_set
    shuffled_regions = shuffle_cpy(map.rng, regions)
    for i=1,n_attempts
        for region in *shuffled_regions
            {w, h} = size
            {x1, y1, x2, y2} = region\bbox()
            -- Expand just outside the bounds of the region:
            x1, y1, x2, y2 = (x1 - w), (y1 - h), (x2 + w), (y2 + h)
            -- Ensure we are within the bounds of the world map:
            x1, y1, x2, y2 = math.max(x1, 0), math.max(y1, 0), math.min(x2, map.size[1] - w), math.min(y2, map.size[2] - h)
            top_left = MapUtils.random_square(map, {x1, y1, x2, y2}, region.selector, nil, 1)
            if top_left
                {top_x, left_y} = top_left
                bbox = {top_x, left_y, top_x + w, left_y + h}
                if SourceMap.rectangle_query {
                    :map
                    area: bbox
                    fill_selector: region.selector or {}
                    perimeter_width: perimeter.width
                    perimeter_selector: perimeter.selector or {}
                }
                    return {:region, :bbox}
    return nil

-- Find a random sqiare in a set of regions.
-- The 'selector' determines properties of the square.
-- Returns the region it was placed in, and the bbox.
find_square = (region_set) ->
    {:region, :bbox} = random_bbox(region_set, {1,1})
    return {:region, square: {bbox[1], bbox[2]}}

selector_filter = (region_set, selector) ->
    -- TODO allow smarter filtering
    {:map, :regions} = region_set
    return {
        :map
        regions: map_call(regions, (r) -> r\with_selector(selector))
    }

return {:find_bbox, :find_square, :selector_filter}
