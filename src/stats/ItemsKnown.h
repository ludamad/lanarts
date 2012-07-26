/*
 * ItemsKnown.h:
 *   Consumable items that have been identified through use
 *   Items that can carry various enchantments are treated differently
 *   Enchanted potions would have separately unknown enchantments,
 *   and be a different kind of item
 */

#ifndef ITEMSKNOWN_H_
#define ITEMSKNOWN_H_

#include <set>

#include "../lanarts_defines.h"

class ItemsKnown {
public:
	typedef std::set<item_id>::const_iterator iterator;
	ItemsKnown() {
	}

	void mark_as_known(item_id enemy) {
		items_known.insert(enemy);
	}

	iterator begin() const {
		return items_known.begin();
	}
	iterator end() const {
		return items_known.end();
	}

	bool is_known(item_id id) const {
		return items_known.find(id) != items_known.end();
	}

	int amount() const {
		return items_known.size();
	}

private:
	std::set<item_id> items_known;
};

#endif /* ITEMSKNOWN_H_ */
