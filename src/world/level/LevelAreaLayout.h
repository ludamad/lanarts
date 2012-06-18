/*
 * LevelAreaLayout.h:
 *  Describes the hierarchical layout of areas in a room
 */

#ifndef LEVELAREALAYOUT_H_
#define LEVELAREALAYOUT_H_

#include <vector>

#include "../../util/game_basic_structs.h"

class LevelArea {
private:
	std::vector<LevelArea> sub_areas;
};

class LevelAreaLayout {
public:
	LevelAreaLayout();
private:
	LevelArea root_area;
};

#endif /* LEVELAREALAYOUT_H_ */
