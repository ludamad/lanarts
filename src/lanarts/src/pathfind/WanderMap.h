/*
 * WanderMap.h:
 *  Keeps a lazily-updated map of potential locations to wander.
 *  The algorithm goes like this:
 *   - Each monster is either WANDER_AWAY'ing from, or WANDER_TOWARDS'ing, a certain square
 *   - Monster at x,y wants to wander, checks the WanderMap
 *   - For each surrounding square ...
 *   	... If there is data associated with this x,y and it is not too outdated ..
 *   		... we grab this information:
 *				- source (x,y)
 *				- distance to source, only taking valid paths
 *		... Otherwise (data doesn't exist or is 'out-dated' [1] ) ...
 *			... starting from x,y as the source, fill each square with the distance to source
 *			... set this location as the location to WANDER_AWAY from
 *		- Toss away any squares that would require crossing a solid square to reach
 *		- If we are WANDER_AWAY, take all the squares - call them SQUARE_SET - with ...
 *			 a different source -or- a greater distance from the current square
 *		- If we are WANDER_TOWARDS, take all the squares - call them SQUARE_SET - with ...
 *			the same source AND a smaller distance from the current square
 *		Pick a random square from SQUARE_SET, move towards that square.
 *
 *	[1] -- Out-dated means here that it if the source was generated STEPS_BEFORE_RECALCULATE steps ago, we create it again
 */

#ifndef WANDERMAP_H_
#define WANDERMAP_H_

#include <vector>

#include <lcommon/geometry.h>

#include "FloodFillPaths.h"

const int STEPS_BEFORE_RECALCULATE = 100;

struct WanderMapSource {
	Pos xy;
	int expiry_frame; // When to regenerate this source
};

struct WanderMapSquare {
	short source_index; // index for WanderMapSource
	short distance_to_source;
};

class WanderMap {
public:
	WanderMap(const Size& size);
	~WanderMap();


private:
	WanderMapSquare& get(const Pos& xy);

	struct Cache { // Data that can be freed if needed, not serialized
		FloodFillPaths _paths_from_source;
		std::vector<Pos> _candidates;
	};

	Cache _cache;

	Size _size;
	std::vector<WanderMapSource> _sources;
	std::vector<WanderMapSquare> _squares;
};

#endif /* WANDERMAP_H_ */
