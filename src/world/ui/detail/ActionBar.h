/*
 * ActionBar.h:
 *  Holds the contents of the action bar.
 */

#ifndef ACTIONBAR_H_
#define ACTIONBAR_H_

#include "../../../gamestats/items.h"

struct ActionSlot {
	spell_id spell; //Specifies spell to use
	Item item; //Specifies item to search for in inventory and use
	//If item is a weapon, this specifies a projectile to equip as well
	Projectile projectile;
};

class ActionBar {
public:
	ActionBar();


	ActionSlot& get(int ind) {
		return actions.at(ind);
	}
private:
	std::vector<ActionSlot> actions;
};

#endif /* ACTIONBAR_H_ */
