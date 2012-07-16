/**
 * GameInstSet.h:
 *  Collection class that keeps track of game objects and assigns instance-id's.
 *  Releases reference to destroyed objects at end of step.
 *  Use instance IDs to refer to objects that may or may not still be alive.
 *  Use GameInstRef to objects that -must- be kept undeleted.
 *  The object may still be removed from the game world.
 */

#ifndef GAMEINSTSET_H_
#define GAMEINSTSET_H_

#include <cstdlib>
#include <vector>
#include "../util/game_basic_structs.h"
#include "objects/GameInst.h"
#include <map>

class GameState;

class GameInstSet {
public:
	enum {
		DEFAULT_CAPACITY = 64, REGION_SIZE = 96
	//cell is REGION_SIZE by REGION_SIZE
	};
	GameInstSet(int w, int h, int capacity = DEFAULT_CAPACITY);
	~GameInstSet();

	//The main part of the API:
	/*Add an instance, optionally specifying its id*/
	obj_id add_instance(GameInst* inst, int id = 0);
	void remove_instance(GameInst* inst);
	void step(GameState* state);

	//Returns NULL if no unit found
	GameInst* get_instance(int id) const;

	int object_radius_test(GameInst* obj, GameInst** objs = NULL, int obj_cap =
			0, col_filterf f = NULL, int x = -1, int y = -1, int radius = -1);

	//Used to synchronize game-id's across network play where objects are created on some systems but not all
	void skip_next_id() {
		next_id++;
	}
	//Allocate all instances to one vector, traversed according to depth order
	std::vector<GameInst*> to_vector() const;
	size_t size() const {
		return unit_amnt;
	}
	//Return a semi-unique hash of the instances contained in the structure
	unsigned int hash() const;
	bool check_copy_integrity(const GameInstSet& inst_set) const;

	void copy_to(GameInstSet& inst_set) const;

	void clear();
private:

	//Internal Structures:
	//Encapsulates instances and the data needed to perform collision lookups by area
	struct InstanceState {
		GameInst* inst;
		//These pointers are invalidated upon hashmap reallocation
		InstanceState* next_in_grid, *prev_in_grid;
		InstanceState* next_same_depth, *prev_same_depth;
		InstanceState() {
			memset(this, 0, sizeof(InstanceState));
		}
		//Used in settools.h
		void operator=(GameInst* inst) {
			this->inst = inst;
			next_in_grid = NULL;
			prev_in_grid = NULL;
			next_same_depth = NULL;
			prev_same_depth = NULL;
		}
	};

	//List of instances with the same rendering depth
	struct InstanceLinkedList {
		InstanceState* start_of_list, *end_of_list;
		InstanceLinkedList() :
				start_of_list(NULL), end_of_list(NULL) {
		}
	};

	typedef std::map<int, InstanceLinkedList> DepthMap;

	/* Internal Data */

	// Map to the first object of a certain depth
	DepthMap depthlist_map;

	// Destroyed objects marked for reference release
	// This will typically result in object deletion
	// unless additional references are retained to it
	std::vector<GameInst*> deallocation_list;

	// Hashset portion
	int next_id, unit_amnt, unit_capacity;
	InstanceState* unit_set;

	// Grid portion
	int grid_w, grid_h;
	//Holds units IDs, 0 if empty
	InstanceLinkedList* unit_grid;

	// Internal structure upkeep functions
	void __remove_instance(InstanceState* state);
	void __update_collision_position(InstanceState* state, const Pos& p1, const Pos& p2);
	void reallocate_internal_data();
	void update_statepointer_for_reallocate_(InstanceState** stateptr);
	void update_depthlist_for_reallocate_(InstanceLinkedList& list);

	void update_instance_for_step(InstanceState* state, GameInst* inst);

	void add_to_depthlist(InstanceState* state, InstanceLinkedList& list);
	void remove_from_depthlist(InstanceState* inst, InstanceLinkedList& list);

	void add_to_collisionlist(InstanceState* inst, InstanceLinkedList& list);
	void remove_from_collisionlist(InstanceState* inst,
			InstanceLinkedList& list);

	/* Integrity check */
	bool within_bounds_check(const Pos& c);

	//Used to allow access to internal data/functions for our hash set implementation utility class
	friend class GameInstSetFunctions;
};

#endif /* GAMEINSTSET_H_ */
