/*
 *GameInst.h:
 * Base object of the game object inheritance heirarchy
 * Life cycle of GameInst initialization:
 * ->Constructor
 * ->GameState::add_instance(obj) called
 * 		->GameInstSet::add_instance(obj) called from GameState::add_instance, sets id
 * 		->GameInst::init(GameState) called from GameState::add_instance, does further initialization
 * ->
 */

#ifndef GAMEINST_H_
#define GAMEINST_H_

#include <cassert>

#include <luawrap/LuaValue.h>
#include <lcommon/geometry.h>
#include <lcommon/math_util.h>

// Effects were originally just for CombatGameInst's, but their benefit
// as a means to add more granular customization to objects was noted.
// Now every GameInst holds them.
// This allows much more clean interaction with, e.g., modifying a projectile object
// from a spell. Spells already heavily use the effect system, so the idea of dynamically
// modifying, say, a store in the same manner is enticing.

#include "stats/effects.h"
#include "stats/stat_modifiers.h"

#include "lanarts_defines.h"

struct lua_State;
class GameState;
class GameMapState;
class SerializeBuffer;
//Base class for game instances

class GameInst {
public:
	/* Reference count functions*/
	static void retain_reference(GameInst* inst);
	static void free_reference(GameInst* inst);

	GameInst(float x, float y, float radius, bool solid = true, int depth = 0) :
			reference_count(0), id(0), last_x(iround(x)), last_y(iround(y)), x(x), y(y), radius(
					radius), target_radius(radius), depth(depth), solid(solid), destroyed(
					false), destroy_pending(false), current_floor(-1) {
		if (this->radius > 14)
			this->radius = 14;
	}

	virtual ~GameInst();
	/* Initialize the object further, 'id' will be set*/
	virtual void init(GameState* gs);
	/* Deinitialize the object, removing eg child instances*/
	virtual void deinit(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void post_draw(GameState* gs);

	virtual void copy_to(GameInst* inst) const;

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	void serialize_lua(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize_lua(GameState* gs, SerializeBuffer& serializer);
	virtual GameInst* clone() const;
	//Used for integrity checking
	virtual unsigned int integrity_hash();
	virtual void update_position(float newx, float newy);
	virtual std::vector<StatusEffect> base_status_effects(GameState* gs);

	bool try_callback(const char* callback);
	void lua_lookup(lua_State* L, const char* key);
	GameMapState* get_map(GameState* gs);

	BBox bbox() {
		return BBox(iround(x - radius), iround(y - radius), iround(x + radius),
				iround(y + radius));
	}

	PosF pos() {
		return PosF(x, y);
	}
	Pos ipos() {
		return Pos(iround(x), iround(y));
	}
	//Used for keeping object from being deleted arbitrarily
	//Important for the GameInst lua binding
	int reference_count;

	/*Should probably keep these public, many functions operate on these*/
	obj_id id;
	int last_x, last_y;
	float x, y, radius, target_radius;
	int depth;
	bool solid, destroyed, destroy_pending;
    int times_serialized = 0; // Debug variable, incremented every time object is read from disk;
	level_id current_floor;
	// Serialized / deserialized in a separate pass because they have Lua references to objects throughout the system:
	LuaValue lua_variables;
	EffectStats effects;
};

typedef bool (*col_filterf)(GameInst* o1, GameInst* o2);

#endif /* GAMEINST_H_ */
