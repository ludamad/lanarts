----
-- Overworld variant "labyrinth": a hedge maze.
-- A coarse grid of maze cells, cut to a wobbly oval in the middle of the valley, is carved into a
-- perfect maze by a recursive backtracker; corridors are grass worn to dirt, the walls between
-- them stay as hedges of trees. A few extra openings add loops, a few 2x2 blocks of cells become
-- open lawns, and a large lawn sits at the heart of the maze. Outside the oval is forest.
----

SourceMap = require "core.SourceMap"
{:TILES, :paint, :layout_area, :connect_walkable, :grid_regions, :create, :protect_matching} = require "map_descs.OverworldCommon"

CORRIDOR = 3           -- corridor width in squares
PITCH = 5              -- corridor plus hedge
LOOP_CHANCE = 0.08     -- chance a remaining hedge between two maze cells gets an opening
N_LAWNS = {3, 6}       -- range of 2x2 lawns (upper bound exclusive)

rect = (map, x1, y1, x2, y2, tile) ->
    SourceMap.rectangle_apply {:map, area: {x1, y1, x2, y2}, fill_operator: paint(tile), create_subgroup: false}

layout = (node) ->
    {:map, :rng} = node
    area = layout_area(node)
    {x1, y1, x2, y2} = area

    -- The maze grid, centred in the valley
    n = rng\random(15, 18)
    ox = math.floor((x1 + x2 - n * PITCH) / 2)
    oy = math.floor((y1 + y2 - n * PITCH) / 2)
    -- Top-left square of cell (i, j), 0-based
    cell_xy = (i, j) -> ox + i * PITCH, oy + j * PITCH

    -- Cells inside an oval whose radius wobbles with a few random harmonics, so the maze edge is
    -- ragged rather than square
    harmonics = for k=2,4
        {k, rng\randomf(0, 2 * math.pi), rng\randomf(0.04, 0.09)}
    in_maze = {}
    for j=0,n-1
        in_maze[j] = {}
        for i=0,n-1
            dx, dy = (i + 0.5 - n / 2) / (n / 2), (j + 0.5 - n / 2) / (n / 2)
            a = math.atan2(dy, dx)
            r = 1.0
            for {k, phase, amp} in *harmonics
                r += amp * math.sin(k * a + phase)
            in_maze[j][i] = dx * dx + dy * dy <= r * r

    -- Recursive backtracker (with an explicit stack) from the centre cell
    visited = {}
    for j=0,n-1
        visited[j] = {}
    open_cell = (i, j) ->
        cx, cy = cell_xy(i, j)
        rect(map, cx, cy, cx + CORRIDOR, cy + CORRIDOR, TILES.grass)
    -- Opens the hedge between two orthogonally adjacent cells
    open_between = (i, j, ni, nj) ->
        ax, ay = cell_xy(math.min(i, ni), math.min(j, nj))
        if ni ~= i
            rect(map, ax + CORRIDOR, ay, ax + PITCH, ay + CORRIDOR, TILES.grass)
        else
            rect(map, ax, ay + CORRIDOR, ax + CORRIDOR, ay + PITCH, TILES.grass)
    usable = (i, j) -> i >= 0 and j >= 0 and i < n and j < n and in_maze[j][i]
    DIRS = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}

    c = math.floor(n / 2)
    visited[c][c] = true
    open_cell(c, c)
    stack = {{c, c}}
    while #stack > 0
        {i, j} = stack[#stack]
        choices = {}
        for {dx, dy} in *DIRS
            ni, nj = i + dx, j + dy
            if usable(ni, nj) and not visited[nj][ni]
                append choices, {ni, nj}
        if #choices == 0
            table.remove(stack)
        else
            {ni, nj} = choices[rng\random(1, #choices + 1)]
            visited[nj][ni] = true
            open_cell(ni, nj)
            open_between(i, j, ni, nj)
            append stack, {ni, nj}

    -- Loops: a few extra openings between visited neighbours (already-open ones just repaint)
    for j=0,n-1
        for i=0,n-1
            if visited[j][i]
                for {dx, dy} in *{{1, 0}, {0, 1}}
                    ni, nj = i + dx, j + dy
                    if usable(ni, nj) and visited[nj][ni] and rng\randomf() < LOOP_CHANCE
                        open_between(i, j, ni, nj)

    -- Well-trodden corridors wear down to dirt
    SourceMap.noise_apply {:map, area: {ox, oy, ox + n * PITCH, oy + n * PITCH}, seed: rng\random(0, 2^30), scale: 8, octaves: 2, bands: {
        {max: 0.25}
        paint(TILES.dirt, {max: 2, matches_none: {SourceMap.FLAG_SOLID}})
    }}

    -- Lawns: all-visited blocks of cells cleared of hedges
    lawn = (i, j, size, tile) ->
        for b=j,j+size-1
            for a=i,i+size-1
                if not usable(a, b) or not visited[b][a]
                    return false
        lx, ly = cell_xy(i, j)
        rect(map, lx, ly, lx + (size - 1) * PITCH + CORRIDOR, ly + (size - 1) * PITCH + CORRIDOR, tile)
        return true
    for _=1,rng\random(N_LAWNS[1], N_LAWNS[2])
        for attempt=1,20
            if lawn(rng\random(1, n - 2), rng\random(1, n - 2), 2, TILES.grass_lush)
                break
    -- The central lawn, with a ring of hedge trees left standing at its corners
    lawn(c - 1, c - 1, 3, TILES.grass_dry)
    lx, ly = cell_xy(c - 1, c - 1)
    span = 2 * PITCH + CORRIDOR
    for {tx, ty} in *{{lx + 3, ly + 3}, {lx + span - 4, ly + 3}, {lx + 3, ly + span - 4}, {lx + span - 4, ly + span - 4}}
        map\square_apply({tx, ty}, paint(TILES.trees))

    if not connect_walkable(map, area, 0.1, rng)
        return false
    map.regions = grid_regions({ox, oy, ox + n * PITCH, oy + n * PITCH}, 2, 2, 2)
    -- Keep vaults out of the heart of the maze; they and the dungeon entrances sit round its rim
    protect_matching(map, {ox + 2 * PITCH, oy + 2 * PITCH, ox + (n - 2) * PITCH, oy + (n - 2) * PITCH}, {})
    return true

return create(layout)
