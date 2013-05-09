#include <lcommon/unittest.h>
#include <lcommon/Timer.h>

#include <lcommon/mtwist.h> // For random gen
#include "pathfind/FloodFillPaths.h"

SUITE(FloodFillPaths_tests) {

	static void populate_grid(int seed, const BoolGridRef& grid) {
		MTwist mtwist(seed);

		for (int y = 0; y < grid->height(); y++) {
			for (int x = 0; x < grid->width(); x++) {
				bool is_solid = (mtwist.rand(5) == 0);
				(*grid)[Pos(x, y)] = is_solid;
			}
		}
	}

	// Utility methods used to debug test problems if they occur
	static void print_grid(Grid<bool>& grid) {
		printf("<GRID>\n");
		for (int y = 0; y < grid.height(); y++) {
			for (int x = 0; x < grid.width(); x++) {
				if ( grid[Pos(x, y)] ) {
					printf("X ");
				} else {
					printf("- ");
				}
			}
			printf("\n");
		}
		printf("</GRID>\n");
		fflush(stdout);
	}
	static void print_path(Grid<bool>& grid, Grid<bool>& path) {
		printf("<GRID>\n");
		for (int y = 0; y < grid.height(); y++) {
			for (int x = 0; x < grid.width(); x++) {
				if (path[Pos(x,y)]) {
					printf("P ");
				} else if ( grid[Pos(x, y)] ) {
					printf("X ");
				} else {
					printf("- ");
				}
			}
			printf("\n");
		}
		printf("</GRID>\n");
		fflush(stdout);
	}

	static void print_marked(FloodFillPaths& paths) {
		Pos center = BBox(Pos(), paths.size()).center();
		printf("<MARKED>\n");
		for (int y = 0; y < paths.height(); y++) {
			for (int x = 0; x < paths.width(); x++) {
				if (center == Pos(x,y)) {
					printf("C ");
				} else if (!paths.node_at(Pos(x, y))->open ) {
					printf("M ");
				} else {
					printf("- ");
				}
			}
			printf("\n");
		}
		printf("</MARKED>\n");
		fflush(stdout);
	}

	static Pos follow(FloodFillPaths& paths, const Pos& from_xy) {
		CHECK(from_xy.x >= 0 && from_xy.x < paths.width());
		CHECK(from_xy.y >= 0 && from_xy.y < paths.height());

		FloodFillNode* node = paths.node_at(from_xy);
		return Pos(from_xy.x + node->dx, from_xy.y + node->dy);
	}

	static void assert_has_path(FloodFillPaths& paths, const Pos& source_xy,
			Pos from_xy) {

		if (paths.node_at(from_xy)->open) {
			return; // Unreachable, don't attempt to follow
		}

		Grid<bool> already_visited(paths.size(), false);

		while (true) {
			CHECK(!already_visited[from_xy]);
			already_visited[from_xy] = true;

			from_xy = follow(paths, from_xy);

			if (source_xy == from_xy) {
				return; /* Found our way*/
			}
		}

		unit_test_assert("Should never occur!", false);
	}


	TEST(test_paths_are_correct) {
		Size TEST_SIZE(30, 30);
		BBox area(Pos(0, 0), TEST_SIZE);
		Pos source_xy = area.center();
		Timer timer;

		BoolGridRef grid(new Grid<bool>(TEST_SIZE));
		FloodFillPaths paths(grid);

		for (int seed = 1; seed <= 11; seed++) {
			populate_grid(seed, grid);

			(*grid)[source_xy] = false; // Ensure not solid

			paths.fill_paths_tile_region(source_xy, area);

			int ind = 0;
			for (int y = 0; y < paths.height(); y++) {
				for (int x = 0; x < paths.width(); x++) {
					ind++;
					assert_has_path(paths, source_xy, Pos(x, y));
				}
			}
		}
		printf("TIMER SAYS test_paths_are_correct took %2.fms\n", timer.get_microseconds() / 1000.0f);
	}
}
