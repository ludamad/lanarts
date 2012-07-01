/*
 * ActionBar.h:
 *  Handles drawing and state of the action bar.
 */

#ifndef ACTIONBAR_H_
#define ACTIONBAR_H_

#include <vector>

#include "../../../gamestats/items.h"

#include "../../../util/ActionQueue.h"

#include "../../GameAction.h"

class GameState;

struct ActionSlot {
	ActionSlot() :
			is_spell(false) {
	}
	ActionSlot(spell_id spell) :
			is_spell(true), spell(spell) {
	}
	ActionSlot(const Item& item, const Projectile& projectile) :
			is_spell(false), spell(-1), item(item), projectile(projectile) {
	}

	bool is_spell;
	spell_id spell; //Specifies spell to use
	Item item; //Specifies item to search for in inventory and use
	//If item is a weapon, this specifies a projectile to equip as well
	Projectile projectile;
};

const int MAX_ACTIONS = 32;

class ActionBar {
public:
	ActionBar(const BBox& bbox, int max_actions = MAX_ACTIONS);

	ActionSlot& get(int ind) {
		return actions.at(ind);
	}

	/* Returns whether an action was processed */
	bool handle_io(GameState* gs, ActionQueue& queued_actions);

	void draw(GameState* gs) const;
	void step(GameState* gs);
private:

	BBox bbox;
	std::vector<ActionSlot> actions;
};

#endif /* ACTIONBAR_H_ */
