#ifndef MAP_ALGO_UTIL_H
#define MAP_ALGO_UTIL_H

#include <tuple>
#include <lcommon/geometry.h>
#include <lanarts_defines.h>
#include <lcommon/math_util.h>

namespace map_algorithms {

template <typename SolidityF, typename SmellF>
struct FindBestNearbyTile {
    /* Function */
    SolidityF is_solid;
    SmellF get_smell;

    void _best_near_xy(Pos xy, float& iter_best, Pos &iter_loc) {
        BBox area{xy - Pos{1, 1}, Size{3, 3}};
        FOR_EACH_BBOX(area, x, y) {
            if ((xy.x == x && xy.y == y) || is_solid(x, y)) {
                continue;
            }
            Pos d = Pos {x, y} - xy;
            if (abs(d.x) == 1 && abs(d.y) == 1) {
                // Check if diagonally blocked:
                if (is_solid(x, xy.y) && is_solid(xy.x, y)) {
                    continue;
                }
            }
            float smell = get_smell(x, y);
            if (smell > iter_best) {
                iter_best = smell;
                iter_loc = {x, y};
            }
        }
    }

    std::tuple<float, PosF> compute(const BBox &bbox) {
        //Set up coordinate min and max
        int mingrid_x = bbox.x1 / TILE_SIZE, mingrid_y = bbox.y1 / TILE_SIZE;
        int maxgrid_x = bbox.x2 / TILE_SIZE, maxgrid_y = bbox.y2 / TILE_SIZE;
        //Make sure coordinates do not go out of bounds
        BBox area = {bbox.x1 / TILE_SIZE, bbox.y1 / TILE_SIZE, bbox.x2 / TILE_SIZE + 1, bbox.y2 / TILE_SIZE + 1};

        float score = 0;
        float n_weight = 0;
        float acc_x = 0, acc_y = 0;
        FOR_EACH_BBOX(area, xx, yy) {
            int sx = std::max(xx * TILE_SIZE, bbox.x1);
            int sy = std::max(yy * TILE_SIZE, bbox.y1);
            int ex = std::min((xx + 1) * TILE_SIZE, bbox.x2);
            int ey = std::min((yy + 1) * TILE_SIZE, bbox.y2);

            float best = 0;
            Pos least_xy = {xx, yy};
            if (!is_solid(xx,yy)) {
                _best_near_xy({xx, yy}, best, least_xy);
                PosF dir = (least_xy) - Pos(xx, yy);
                int sub_area = (ex - sx) * (ey - sy) + 1;
                /*Make sure all interpolated directions are possible*/
                acc_x += dir.x * sub_area;
                acc_y += dir.y * sub_area;
                score += best * sub_area;
                n_weight += sub_area;
            }
        }
        float mag = sqrt(acc_x * acc_x + acc_y * acc_y);
        PosF dir;
        if (mag != 0) {
            dir = {acc_x, acc_y};
        }
        return {n_weight == 0 ? 0 : score / n_weight, dir};
    }
};

template <typename SolidityF, typename SmellF>
inline auto find_best_near_algo(const SolidityF& is_solid, const SmellF& get_smell) {
    return FindBestNearbyTile<SolidityF, SmellF> {is_solid, get_smell};
}
}

#endif
