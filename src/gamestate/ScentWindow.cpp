#include "ScentWindow.h"
#include "map_algo_util.h"

ScentWindow::ScentWindow(BBox area) : area(area) {
    map.resize(area.size());
}

void ScentWindow::add_smell(Pos xy, float smell) {
    if (area.contains(xy)) {
        Pos raw_xy = xy - area.left_top();
        map[raw_xy] += smell;
    }
}

std::tuple<float, PosF> towards_least_smell(GameState* gs, ScentWindow& window, CombatGameInst* inst);
std::tuple<float, PosF> towards_most_smell(GameState* gs, ScentWindow& window, CombatGameInst* inst);

void compute_diffusion(const ScentWindow& input, ScentWindow& output) {
    FOR_EACH_BBOX(output.area, x, y) {

    }
}
