#include "SmellMap.h"

static bool _can_head(Grid<bool>& solidity, const BBox& bbox, float dx, float dy) {
    bool is_diag = (fabs(dx) > 0 && fabs(dy) > 0);

    int xx, yy;
    for (int y = bbox.y1; y <= bbox.y2 + TILE_SIZE; y += TILE_SIZE) {
        for (int x = bbox.x1; x <= bbox.x2 + TILE_SIZE; x += TILE_SIZE) {
            xx = squish(x, bbox.x1, bbox.x2 + 1);
            yy = squish(y, bbox.y1, bbox.y2 + 1);

            int gx = (xx + dx) / TILE_SIZE;
            int gy = (yy + dy) / TILE_SIZE;
            if (solidity[{gx, gy}]) {
                return false;
            }
            if (is_diag) {
                if (solidity[{xx / TILE_SIZE, gy}]) {
                    return false;
                }
                if (solidity[{gx, yy / TILE_SIZE}]) {
                    return false;
                }
            }
        }
    }
    return true;
}

static void _least_smell_near(Grid<bool>& solidity, Grid<float>& smells, const Pos xy, float& iter_least, Pos& iter_loc) {
    BBox area {xy - Pos{1,1}, Size{3, 3}};
    FOR_EACH_BBOX(area, x, y) {
            if ((xy.x == x && xy.y == y) || solidity[{x,y}]) {
                continue;
            }
            Pos d = Pos {x, y} - xy;
            if (abs(d.x) == 1 && abs(d.y) == 1) {
                // Check if diagonally blocked:
                if (solidity[{x, xy.y}] && solidity[{xy.x, y}]) {
                    continue;
                }
            }
            if (smells[{x, y}] < iter_least) {
                iter_least = smells[{x,y}];
                iter_loc = {x, y};
            }
        }
}

static PosF _towards_least_smell(Grid<bool>& solidity, Grid<float>& smells, const BBox& bbox, float speed) {
    using namespace std;

    //Set up coordinate min and max
    int mingrid_x = bbox.x1 / TILE_SIZE, mingrid_y = bbox.y1 / TILE_SIZE;
    int maxgrid_x = bbox.x2 / TILE_SIZE, maxgrid_y = bbox.y2 / TILE_SIZE;
    //Make sure coordinates do not go out of bounds
    BBox area = {
            squish(mingrid_x, 2, smells.width() - 2),
            squish(mingrid_y, 2, smells.height() - 2),
            squish(maxgrid_x, 2, smells.width() - 2) + 1,
            squish(maxgrid_y, 2, smells.height() - 2) + 1};

    float acc_x = 0, acc_y = 0;
    FOR_EACH_BBOX(area, xx, yy) {
            int sx = max(xx * TILE_SIZE, bbox.x1), sy = max(yy * TILE_SIZE,
                                                            bbox.y1);
            int ex = min((xx + 1) * TILE_SIZE, bbox.x2), ey = min(
                    (yy + 1) * TILE_SIZE, bbox.y2);

            float least = INFINITY;
            Pos least_xy = {xx, yy};
            if (!solidity[{xx,yy}]) {
                _least_smell_near(solidity, smells, {xx,yy}, least, least_xy);
                PosF dir = (least_xy) - Pos(xx,yy);
                int sub_area = (ex - sx) * (ey - sy) + 1;
                /*Make sure all interpolated directions are possible*/
                acc_x += dir.x * sub_area;
                acc_y += dir.y * sub_area;
            }
        }
    float mag = sqrt(float(acc_x * acc_x + acc_y * acc_y));
    if (mag == 0) {
        return PosF();
    } else {
        float vx = speed * float(acc_x) / mag;
        float vy = speed * float(acc_y) / mag;
        return PosF(vx, vy);
    }
}

static PosF _towards_least_smell(Grid<bool>& solidity, Grid<float>& smells, const BBox& bbox, float speed);

SmellMap::SmellMap(GameTiles& tiles) : tiles(tiles) {
    map.resize(tiles.size());
}

void SmellMap::step() {

}

void SmellMap::add_smell(GameInst* inst, float smell) {
}

PosF SmellMap::towards_least_smell(CombatGameInst* inst) {
}

PosF SmellMap::towards_most_smell(CombatGameInst* inst) {
}
