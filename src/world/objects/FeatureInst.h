/*
 * FeatureInst.h:
 *  Represents a permanent feature on the floor.
 *  Can be pure decoration but does not appear like eg AnimatedInst
 */

#ifndef FEATUREINST_H_
#define FEATUREINST_H_

#include "../../gamestats/items.h"

#include "GameInst.h"

class FeatureInst: public GameInst {
public:
	enum feature_t {
		DOOR_OPEN, DOOR_CLOSED, DECORATION
	};
	enum {
		RADIUS = 15, DEPTH = 100
	};
	FeatureInst(int x, int y, feature_t feature, sprite_id spriteid = -1) :
			GameInst(x, y, solid, false, DEPTH), feature(feature), spriteid(
					spriteid) {
	}
	virtual ~FeatureInst();
	virtual void init(GameState *gs);
	virtual void deinit(GameState *gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	void player_interact(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual FeatureInst* clone() const;
private:
	feature_t feature;
	sprite_id spriteid;
};

static bool feature_exists_near(GameState* gs, const Pos& p);

#endif /* FEATUREINST_H_ */
