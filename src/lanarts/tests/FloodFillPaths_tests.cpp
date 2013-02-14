#include <lcommon/unittest.h>

#include <lcommon/mtwist.h> // For random gen
#include "pathfind/FloodFillPaths.h"

static void populate_grid(int seed, BoolGridRef grid) {
	MTwist mtwist(seed);

	for (int y = 0; y < grid->height(); y++) {
		for (int x = 0; x < grid->width(); x++) {
			bool is_solid = (mtwist.rand(5) == 0);
			grid[Pos(x,y)] = is_solid;
		}
	}
}

SUITE(FloodFillPaths_tests) {
	TEST(SanityTest) {

	}
}
