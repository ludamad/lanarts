/*
 * map_misc_ops.h:
 *  Connectivity analysis via floodfill and other operations.
 */

#include "map_misc_ops.h"

#include <vector>

//
//local function floodStackScanline(x,y,grid,stack)
//stack:clear()
//local y1
//local spanLeft, spanRight
//if grid:has(x,y) and grid._map[y][x].v==0 then
//stack:push(grid._map[y][x])
//end
//while not stack:isEmpty() do
//local p = stack:pop()
//x, y = p.x, p.y
//y1 = y
//while ((y1>=1) and grid:has(x,y1) and grid._map[y1][x].v==0) do
//y1 = y1 - 1
//end
//y1 = y1 + 1
//spanLeft, spanRight = false, false
//while (y1<=grid._height and grid:has(x,y1) and grid._map[y1][x].v==0) do
//grid:set(x,y1,1)
//if (not spanLeft and x>0 and grid:has(x-1,y1) and grid._map[y1][x-1].v==0) then
//stack:push(grid._map[y1][x-1])
//spanLeft = true
//elseif (spanLeft and x>0 and grid:has(x-1,y1) and grid._map[y1][x-1].v~=0)then
//spanLeft = false
//end
//if (not spanRight and x<=grid._width and grid:has(x+1,y1) and grid._map[y1][x+1].v==0) then
//stack:push(grid._map[y1][x+1])
//spanRight = true
//elseif (spanRight and x<=grid._width and grid:has(x+1,y1) and grid._map[y1][x+1].v~=0) then
//spanRight = false
//end
//y1 = y1+1
//end
//end
//end

using namespace ldungeon_gen;

// Base function of area_fill_unconnected, and area_fully_connected
// Mark the map with our marker Operator
void area_connected_search_mark(Map& map, BBox area, Pos seed, Selector unfilled, Operator mark, Selector marked) {
    std::vector<Pos> queue {seed};
    map[seed].apply(mark);
    while (!queue.empty()) {
        Pos curr = queue.back();
        queue.pop_back();
        auto queue_pos = [&](Pos next) {
            if (next.x < area.x1 || next.y < area.y1 || next.x >= area.x2 || next.y >= area.y2) {
                return;
            }
            if (map[next].matches(marked) || !map[next].matches(unfilled)) {
                return;
            }
            map[next].apply(mark);
            queue.push_back(next);
        };
        int x = curr.x, y = curr.y;
        queue_pos(Pos(x-1, y));
        queue_pos(Pos(x, y-1));
        queue_pos(Pos(x, y+1));
        queue_pos(Pos(x+1, y));
    }
}

namespace ldungeon_gen {
    void area_fill_unconnected(Map& map, BBox area, Pos seed, Selector unfilled, Operator mark, Selector marked, Operator fill) {
        if (!map[seed].matches(unfilled)) {
            FOR_EACH_BBOX(area, x, y) {
                if (map[Pos(x,y)].matches(unfilled)) {
                    seed = Pos(x, y);
                    break;
                }
            }
            if (seed.x == -1 && seed.y == -1) {
                return; // Fully filled area is fully connected, technically.
            }
        }

        area_connected_search_mark(map, area, seed, unfilled, mark, marked);

        FOR_EACH_BBOX(area, x, y) {
            if (map[Pos(x,y)].matches(unfilled) && !map[Pos(x,y)].matches(marked)) {
            //    map[Pos(x,y)].apply(fill);
            }
        }
    }

    // Used for outright rejecting maps, as a pragmatic step:
    bool area_fully_connected(Map& map, BBox area, Selector unfilled, Operator mark, Selector marked) {
        // Find first unfilled element:
        Pos seed {-1, -1};
        FOR_EACH_BBOX(area, x, y) {
            if (map[Pos(x,y)].matches(unfilled)) {
                seed = Pos(x, y);
                break;
            }
        }
        if (seed.x == -1 && seed.y == -1) {
            return true; // Fully filled area is fully connected, technically.
        }

        area_connected_search_mark(map, area, seed, unfilled, mark, marked);

        FOR_EACH_BBOX(area, x, y) {
            if (map[Pos(x,y)].matches(unfilled) && !map[Pos(x,y)].matches(marked)) {
                return false;
            }
        }
        return true;
    }
    void erode_diagonal_pairs(Map& map, MTwist& rng, BBox area, Selector candidate) {
        area.y2--; // Don't consider last row, guarantees we can query row below us.
        FOR_EACH_BBOX(area, x, y) {
            Square& s1 = map[Pos(x,y)];
            if (s1.matches(candidate)) {
                if (x - 1 >= area.x1) {
                    Square& n1 = map[Pos(x-1,y)];
                    Square& n2 = map[Pos(x,y+1)];
                    Square& s2 = map[Pos(x-1,y+1)];
                    if (!n1.matches(candidate) && !n2.matches(candidate) && s2.matches(candidate)) {
                        Square& from = (rng.rand(2) ? n1 : n2);
                        Square& to = (rng.rand(2) ? s1 : s2);
                        to = from;
                    }
                }
                if (x + 1 < area.x2) {
                    Square& n1 = map[Pos(x+1,y)];
                    Square& n2 = map[Pos(x,y+1)];
                    Square& s2 = map[Pos(x+1,y+1)];
                    if (!n1.matches(candidate) && !n2.matches(candidate) && s2.matches(candidate)) {
                        Square& from = (rng.rand(2) ? n1 : n2);
                        Square& to = (rng.rand(2) ? s1 : s2);
                        to = from;
                    }
                }
            }
        }
    }
}
