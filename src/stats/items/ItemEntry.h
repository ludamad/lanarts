/*
 * ItemEntry.h:
 *  Describes an item entry.
 *  This class can represent equipment or consumables such as scrolls etc
 *  It is the base class for various kinds of equipment. Consumables use it directly.
 */

#ifndef ITEMENTRY_H_
#define ITEMENTRY_H_

#include <string>

#include "../data/BaseDataEntry.h"

#include "LuaAction.h"

class ItemEntry: public BaseDataEntry {
public:
	ItemEntry() :
			stackable(true) {
	}
	virtual ~ItemEntry() {
	}

	enum equip_type {
		NONE, WEAPON, PROJECTILE, ARMOUR
	};
	enum id_type {
		ALWAYS_KNOWN, POTION, SCROLL
	};
	// Cost when appearing in shops, if (0,0) will not appear in shops.
	Range shop_cost;

	LuaAction use_action;

	bool stackable;
	// Extra information if this item is a piece of equipment
	equip_type equipment_type;
	int equipment_id;
	id_type identify_type;
};

#endif /* ITEMENTRY_H_ */
