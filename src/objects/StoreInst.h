/*
 * StoreInst.h:
 *  Represents a store NPC or store tile
 */

#ifndef STOREINST_H_
#define STOREINST_H_

#include "interface/sidebar/StoreContent.h"

#include "../GameInst.h"

#include "StoreInventory.h"

class StoreInst: public GameInst {
public:
	enum {
		RADIUS = 15, DEPTH = 100
	};
	StoreInst(const Pos& pos, bool solid, sprite_id spriteid,
			const StoreInventory& inv, int spr_frame = 0) :
			GameInst(pos.x, pos.y, RADIUS, solid, DEPTH), last_seen_spr(-1), spriteid(
					spriteid), inv(inv), spr_frame(spr_frame) {
	}
	StoreInventory& inventory() {
		return inv;
	}
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual StoreInst* clone() const;

	void set_as_seen() {
	    last_seen_spr = spriteid;
	}

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);
private:
	StoreContent sidebar_display;
	StoreInventory inv;
	sprite_id last_seen_spr;
	sprite_id spriteid;
	int spr_frame;
};

#endif /* STOREINST_H_ */
