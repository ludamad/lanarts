/*
 * StoreInst.h:
 *  Represents a store NPC or store tile
 */

#ifndef STOREINST_H_
#define STOREINST_H_

#include "../../interface/sidebar/StoreContent.h"

#include "../GameInst.h"

#include "StoreInventory.h"

class StoreInst: public GameInst {
public:
	enum {
		RADIUS = 15, DEPTH = 100
	};
	StoreInst(const Pos& pos, bool solid, sprite_id spriteid,
			const StoreInventory& inv) :
			GameInst(pos.x, pos.y, RADIUS, solid, DEPTH), last_seen_spr(-1), spriteid(
					spriteid), inv(inv) {
	}
	StoreInventory& inventory() {
		return inv;
	}
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual StoreInst* clone() const;
private:
	StoreContent sidebar_display;
	StoreInventory inv;
	sprite_id last_seen_spr;
	sprite_id spriteid;
};

#endif /* STOREINST_H_ */
