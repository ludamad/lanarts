/*
 * Sidebar.h:
 *  Handles drawing & state of side bar
 */

#ifndef SIDEBAR_H_
#define SIDEBAR_H_

#include "../../../util/game_basic_structs.h"

#include "Minimap.h"
#include "SidebarNavigator.h"

class Sidebar {
public:
	Sidebar(const BBox& sidebar_box);

private:
	BBox sidebar_box;
	Minimap minimap;
	SidebarNavigator navigator;
};

#endif /* SIDEBAR_H_ */
