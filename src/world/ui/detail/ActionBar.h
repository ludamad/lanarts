/*
 * ActionBar.h:
 *  Holds the contents of the action bar.
 */

#ifndef ACTIONBAR_H_
#define ACTIONBAR_H_

#include <vector>

#include "../../../gamestats/items.h"

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
	ActionBar(int x, int y, int w, int h, int max_actions = MAX_ACTIONS);

	ActionSlot& get(int ind) {
		return actions.at(ind);
	}

	bool is_within_equipped(int mx, int my);
	bool is_within_actionbar(int mx, int my);
	/* Return which slot the mouse is hovering over */
	int get_selected_slot(int mx, int my);

	void draw(GameState* gs) const;
	void step(GameState* gs);
private:
	int x, y, w, h;
	std::vector<ActionSlot> actions;
};

#endif /* ACTIONBAR_H_ */
