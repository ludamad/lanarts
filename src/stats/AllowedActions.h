/*
 * AllowedActions.h:
 *  What the player can do at a given step
 */

#ifndef ALLOWEDACTIONS_H_
#define ALLOWEDACTIONS_H_

struct AllowedActions {
	bool can_use_rest, can_use_stairs, can_use_items, can_use_spells;
	AllowedActions() :
			can_use_rest(true), can_use_stairs(true), can_use_items(true), can_use_spells(
					true) {
	}
	AllowedActions only_in_both(const AllowedActions& aa) const {
		AllowedActions ret;
		ret.can_use_rest = aa.can_use_rest && this->can_use_rest;
		ret.can_use_stairs = aa.can_use_stairs && this->can_use_stairs;
		ret.can_use_items = aa.can_use_items && this->can_use_items;
		ret.can_use_spells = aa.can_use_spells && this->can_use_spells;
		return ret;
	}
};

#endif /* ALLOWEDACTIONS_H_ */
