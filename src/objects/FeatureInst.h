/*
 * FeatureInst.h:
 *  Represents a permanent feature on the floor.
 *  Can be pure decoration but does not disappear like eg AnimatedInst
 */

#ifndef FEATUREINST_H_
#define FEATUREINST_H_

#include "stats/items/items.h"

#include "GameInst.h"

class FeatureInst: public GameInst {
public:
	enum feature_t {
		DOOR_OPEN, DOOR_CLOSED, PORTAL, OTHER
	};
	enum {
		RADIUS = 15, DEPTH = 100
	};
	FeatureInst(const Pos& pos, feature_t feature, bool solid = false,
			sprite_id spriteid = -1, int depth = DEPTH, int sprite_frame = 0);
	virtual ~FeatureInst();
	virtual void init(GameState *gs);
	virtual void deinit(GameState *gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	void player_interact(GameState* gs, GameInst* inst);
	virtual void copy_to(GameInst* inst) const;
	virtual FeatureInst* clone() const;

        void set_as_seen() {
            last_seen_spr = spriteid;
        }
	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);

	feature_t feature_type() const {
		return feature;
	}
    bool has_been_seen() const {
        return (last_seen_spr > -1);
    }
        bool has_been_used() const {
            return used_yet;
        }
private:
	feature_t feature;
	sprite_id last_seen_spr;
	sprite_id spriteid;
	int sprite_frame;
	bool used_yet = false;
};

static bool feature_exists_near(GameState* gs, const Pos& p);

#endif /* FEATUREINST_H_ */
