/*
 * WanderMap.h:
 *  Keeps a lazily-updated map of potential locations to wander.
 *  The algorithm goes like this:
 *   - Each monster is either WANDER_AWAY'ing from, or WANDER_TOWARDS'ing, a certain square
 *   - Monster at x,y wants to wander, checks the WanderMap
 *   - For each surrounding square ...
 *   	... If there is data associated with this x,y
 *   		... we grab this information:
 *				- source (x,y)
 *				- distance to source, only taking valid paths
 *		... Otherwise (data doesn't exist ) ...
 *			... starting from x,y as the source, fill each square with the distance to source
 *			... set this location as the location to WANDER_AWAY from
 *		- Toss away any squares that would require crossing a solid square to reach
 *		- If we are WANDER_AWAY, take all the squares - call them SQUARE_SET - with ...
 *			 a different source -or- a greater distance from the current square
 *		- If we are WANDER_TOWARDS, take all the squares - call them SQUARE_SET - with ...
 *			the same source AND a smaller distance from the current square
 *		Pick a random square from SQUARE_SET, move towards that square.
 */

#ifndef WANDERMAP_H_
#define WANDERMAP_H_

#include <vector>

#include <lcommon/geometry.h>

#include "util/Grid.h"

#include "BoolGridRef.h"
#include "FloodFillPaths.h"

const int STEPS_BEFORE_RECALCULATE = 100;

enum WanderDirection {
	WANDER_AWAY, WANDER_TOWARDS
};

struct WanderMapSquare {
	short distance_to_source;
};

class WanderMap {
public:
	WanderMap(const BoolGridRef& solidity = BoolGridRef(), const Size& size =
			Size(), const Size& division_size = Size(10, 10));
	~WanderMap();

	void initialize(const BoolGridRef& solidity, const Size& size,
			const Size& division_size = Size(10, 10));

			// Returns a reference from the Cache
			const std::vector<Pos>& candidates(const Pos& xy,
			WanderDirection direction);

	void ensure_filled();

	void debug_draw();
private:
	bool HACK_FOR_NOW_filled;

	WanderMapSquare& get(const Pos& xy);

	void generate_wander_map_section(const Pos& source_xy);
	void ensure_square_valid(const Pos& xy);

	struct Cache { // Data that can be freed if needed, not serialized
		FloodFillPaths paths_from_source;
		std::vector<Pos> candidates;
	};

	/* Data */

	Cache _cache;

	Size _division_size;

	/* Shared with game tile structure */
	BoolGridRef _solidity;

	Grid<WanderMapSquare> _squares;
};

#endif /* WANDERMAP_H_ */
