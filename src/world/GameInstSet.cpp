/**
 * GameInstSet.cpp:
 *  Collection class that keeps track of game objects and assigns instance-id's.
 *  Releases reference to destroyed objects at end of step.
 *  Use instance IDs to refer to objects that may or may not still be alive.
 *  Use GameInstRef to objects that -must- be kept undeleted.
 *  The object may still be removed from the game world.
 */

#include <cstring>
#include <cstdio>
#include <typeinfo>

#include "GameInstSet.h"
#include "../util/hashset_util.h"
#include "../util/math_util.h"

#include "utility_objects/AnimatedInst.h"

GameInst* const GAMEINST_TOMBSTONE = (GameInst*)1;

static bool valid_inst(GameInst* inst) {
	return inst > GAMEINST_TOMBSTONE;
}

GameInstSet::GameInstSet(int w, int h, int capacity) {
	grid_w = w / REGION_SIZE + 1, grid_h = h / REGION_SIZE + 1;
	next_id = 1;
	unit_amnt = 0;
	unit_capacity = capacity;
	unit_set = new InstanceState[unit_capacity];
	memset(unit_set, 0, unit_capacity * sizeof(InstanceState));
	unit_grid = new InstanceLinkedList[grid_w * grid_h];
}
GameInstSet::~GameInstSet() {

	for (int i = 0, j = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst))
			delete inst;
	}

	delete[] unit_grid;
	delete[] unit_set;
	for (int i = 0; i < deallocation_list.size(); i++) {
		deallocation_list[i]->free_reference();
	}
}
struct GameInstSetFunctions { //Helper class
	typedef GameInstSet::InstanceState V;
	static bool isNull(const V& v) {
		return v.inst == NULL;
	}
	static bool isRemoved(const V& v) {
		return v.inst == GAMEINST_TOMBSTONE;
	}
	static void remove(V& v) {
		v.inst = GAMEINST_TOMBSTONE;
	}
	static bool equal(const V& v1, int obj_id) {
		return valid_inst(v1.inst) && v1.inst->id == obj_id;
	}
	static bool equal(const V& v1, GameInst* inst) {
		return v1.inst == inst;
	}
	static bool equal(const V& v1, const V& v2) {
		return v1.inst == v2.inst;
	}
	static size_t hash(const V& v) {
		return (size_t)v.inst->id;
	}
	static size_t hash(GameInst* inst) {
		return (size_t)inst->id;
	}
	static size_t hash(obj_id id) {
		return (size_t)id;
	}
};

void GameInstSet::update_statepointer_for_reallocate_(
		InstanceState** stateptr) {
	if (*stateptr)
		*stateptr = tset_find<GameInstSetFunctions>((*stateptr)->inst->id,
				unit_set, unit_capacity);
}
void GameInstSet::update_depthlist_for_reallocate_(InstanceLinkedList& list) {
	update_statepointer_for_reallocate_(&list.start_of_list);
	update_statepointer_for_reallocate_(&list.end_of_list);
}
void GameInstSet::reallocate_internal_data() {
	unit_capacity *= 2;
	InstanceState* old_set = unit_set, *new_set =
			new InstanceState[unit_capacity];
	for (int i = 0; i < unit_capacity; i++) {
		new_set[i].inst = NULL;
	}

	tset_add_all<GameInstSetFunctions>(old_set, unit_capacity / 2, new_set,
			unit_capacity);
	unit_set = new_set;

	//Fix pointers for grid
	for (int i = 0; i < unit_capacity; i++) {
		if (new_set[i].inst) {
			update_statepointer_for_reallocate_(&new_set[i].prev_in_grid);
			update_statepointer_for_reallocate_(&new_set[i].next_in_grid);
			update_statepointer_for_reallocate_(&new_set[i].prev_same_depth);
			update_statepointer_for_reallocate_(&new_set[i].next_same_depth);
		}
	}
	DepthMap::iterator it = depthlist_map.begin();
	for (; it != depthlist_map.end(); it++) {
		update_depthlist_for_reallocate_(it->second);
	}

	for (int i = 0; i < grid_w * grid_h; i++) {
		update_depthlist_for_reallocate_(unit_grid[i]);
	}

	delete[] old_set;

}
static int get_xyind(const Pos& c, int grid_w) {
	return (c.y / GameInstSet::REGION_SIZE) * grid_w
			+ c.x / GameInstSet::REGION_SIZE;
}

void GameInstSet::__remove_instance(InstanceState* state) {
	GameInst* inst = state->inst;
	InstanceLinkedList& list = unit_grid[get_xyind(
			Pos(inst->last_x, inst->last_y), grid_w)];

	remove_from_collisionlist(state, list);
	remove_from_depthlist(state, depthlist_map[inst->depth]);

	this->unit_amnt--;
	state->inst = GAMEINST_TOMBSTONE;
}
void GameInstSet::remove_instance(GameInst* inst) {
	if (inst->destroyed)
		return;
	inst->destroyed = true;
	InstanceState* state = tset_find<GameInstSetFunctions>(inst->id, unit_set,
			unit_capacity);
	__remove_instance(state);
	deallocation_list.push_back(inst);
}

bool GameInstSet::within_bounds_check(const Pos& c) {
	return !(c.x < 0 || c.y < 0 || c.x >= grid_w * REGION_SIZE
			|| c.y >= grid_h * REGION_SIZE);
}

obj_id GameInstSet::add_instance(GameInst* inst, int id) {
	if (tset_should_resize(unit_amnt, unit_capacity))
		this->reallocate_internal_data();

	Pos c(inst->last_x, inst->last_y);
	//Will be set to the current state object in 'add'
	InstanceState* state = unit_set;

	if (id == 0 && inst->id != 0) {
		fprintf(stderr, "Adding instance with id of %d; not 0!\n", inst->id);
		LANARTS_ASSERT(false);
	}

	inst->id = id ? id : (next_id++);
	//Add an object with the assumption that this object does not currently exist (_noequal)
	if (tset_add_noequal<GameInstSetFunctions>(inst, state, unit_capacity))
		unit_amnt++;

	if (true || inst->solid) {
		LANARTS_ASSERT(within_bounds_check(c));
		InstanceLinkedList& unit_list = unit_grid[get_xyind(c, grid_w)];
		add_to_collisionlist(state, unit_list);
	}
	add_to_depthlist(state, depthlist_map[inst->depth]);

	inst->retain_reference();

	return inst->id;
}

void GameInstSet::step(GameState* gs) {
	for (int i = 0; i < deallocation_list.size(); i++) {
		deallocation_list[i]->free_reference();
	}
	deallocation_list.clear();
	for (int i = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst)) {
			inst->destroyed = false;
			inst->step(gs);
			update_instance_for_step(&unit_set[i], inst);
		}
	}
}
GameInst* GameInstSet::get_instance(int id) const {
	InstanceState* is = tset_find<GameInstSetFunctions>(id, unit_set,
			unit_capacity);
	if (is)
		return is->inst;
	return NULL;
}

std::vector<GameInst*> GameInstSet::to_vector() const {
	std::vector<GameInst*> ret(size(), NULL);
//
//	for (int i = 0, j = 0; i < unit_capacity; i++) {
//		GameInst* inst = unit_set[i].inst;
//		if (valid_inst(inst))
//			ret[j++] = inst;
//	}

	DepthMap::const_iterator it = depthlist_map.end();
	for (int ind = 0; it != depthlist_map.begin();) {
		--it;
		InstanceState* state = it->second.start_of_list;
		while (state) {
			ret[ind++] = state->inst;
			state = state->next_same_depth;
		}
	}
	return ret;
}

unsigned int GameInstSet::hash() const {
	unsigned int hash = 0xbabdabe;
	std::vector<GameInst*> as_vector = to_vector();
	for (int i = 0; i < as_vector.size(); i++) {
		GameInst* inst = as_vector[i];
		if (!dynamic_cast<AnimatedInst*>(inst)) {
			hash ^= inst->integrity_hash();
			hash ^= hash * 31337; //Ad hoc hashing yay
		}
	}
	return hash;
}

void GameInstSet::copy_to(GameInstSet& inst_set) const {

	DepthMap::const_iterator it = depthlist_map.end();
	//Synch live objects
	for (int ind = 0; it != depthlist_map.begin();) {
		--it;
		InstanceState* state = it->second.start_of_list;
		while (state) {
			GameInst* inst = state->inst;
			obj_id id = inst->id;
			GameInst* oinst = inst_set.get_instance(id);
			if (oinst == NULL || typeid(inst) != typeid(oinst)) {
				inst_set.add_instance(inst->clone(), id);
				if (oinst)
					oinst->destroyed = true;
			}
			state = state->next_same_depth;
		}
	}
	//Remove dead objects
	for (int i = 0; i < inst_set.unit_capacity; i++) {
		InstanceState* state = &inst_set.unit_set[i];
		GameInst* oinst = state->inst;
		if (valid_inst(oinst)) {
			if (!oinst->destroyed) {
				GameInst* inst = get_instance(oinst->id);
				if (inst != NULL) {
					inst_set.__update_collision_position(state,
							Pos(oinst->x, oinst->y), Pos(inst->x, inst->y));
					inst->copy_to(oinst);
				} else
					inst_set.__remove_instance(state);
				delete inst;
			}

		}
	}
	inst_set.next_id = this->next_id;
	LANARTS_ASSERT(check_copy_integrity(inst_set));
}

int GameInstSet::object_radius_test(GameInst* obj, GameInst** objs, int obj_cap,
		col_filterf f, int x, int y, int radius) {
	int rad = radius == -1 ? obj->target_radius : radius;
	x = x == -1 ? obj->x : x;
	y = y == -1 ? obj->y : y;

	int mingrid_x = (x - rad) / REGION_SIZE, mingrid_y = (y - rad)
			/ REGION_SIZE;
	int maxgrid_x = (x + rad) / REGION_SIZE, maxgrid_y = (y + rad)
			/ REGION_SIZE;
	int minx = squish(mingrid_x, 0, grid_w), miny = squish(mingrid_y, 0,
			grid_h);
	int maxx = squish(maxgrid_x, 0, grid_w), maxy = squish(maxgrid_y, 0,
			grid_h);

	int obj_n = 0;

	for (int yy = miny; yy <= maxy; yy++) {
		int index = yy * grid_w + minx;
		for (int xx = minx; xx <= maxx; xx++) {
			InstanceLinkedList& unit_list = unit_grid[index++];
			InstanceState* ptr = unit_list.start_of_list;
			if (!ptr)
				continue;

			while (ptr) {
				GameInst* inst = ptr->inst;
				if (obj != inst) {
					int radsqr = (inst->target_radius + rad)
							* (inst->target_radius + rad);
					int dx = inst->x - x, dy = inst->y - y;
					int dsqr = dx * dx + dy * dy;
					//want to test sqrt(dsqr) < orad+rad
					//therefore we test dsqr < (orad+rad)*(orad+rad)
					if (dsqr < radsqr
							&& ((!f && inst->solid) || (f && f(obj, inst)))) {
						if (obj_cap == 0)
							return 1;
						objs[obj_n] = inst;
						obj_n++;
						if (obj_n >= obj_cap)
							return obj_n;
					}
				}
				ptr = ptr->next_in_grid;
			}
		}
	}
	return obj_n;
}

void GameInstSet::clear() {
	for (int i = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst)) {
			delete inst;
		}
	}
	next_id = 1;
	unit_amnt = 0;
	memset(unit_set, 0, unit_capacity * sizeof(InstanceState));
}

//TODO: Make collisionlist entry positions deterministic -or- make collision functions always return the same object
void GameInstSet::__update_collision_position(InstanceState* state,
		const Pos& p1, const Pos& p2) {
	GameInst* inst = state->inst;

	int old_bucket = get_xyind(p1, grid_w), new_bucket = get_xyind(p2, grid_w);
	if (old_bucket != new_bucket) {
		//remove from previous unit grid lookup
		remove_from_collisionlist(state, unit_grid[old_bucket]);
		//add to next unit lookup
		add_to_collisionlist(state, unit_grid[new_bucket]);
	}
}

void GameInstSet::update_instance_for_step(InstanceState* state,
		GameInst* inst) {
	if (inst->destroyed)
		return;
	if (true || inst->solid) {
		Pos last_pos(inst->last_x, inst->last_y), new_pos(inst->x, inst->y);
		LANARTS_ASSERT(within_bounds_check(last_pos));
		LANARTS_ASSERT(within_bounds_check(new_pos));
		__update_collision_position(state, last_pos, new_pos);
	}
	inst->last_x = inst->x, inst->last_y = inst->y;
}

void GameInstSet::add_to_depthlist(InstanceState* inst,
		InstanceLinkedList& list) {
	inst->next_same_depth = NULL;
	if (list.start_of_list == NULL) {
		inst->prev_same_depth = NULL;
		list.start_of_list = inst;
		list.end_of_list = inst;
	} else {
		inst->prev_same_depth = list.end_of_list;
		list.end_of_list->next_same_depth = inst;
		list.end_of_list = inst;
	}
}

void GameInstSet::remove_from_depthlist(InstanceState* inst,
		InstanceLinkedList& list) {
	InstanceState* next = inst->next_same_depth;
	InstanceState* prev = inst->prev_same_depth;
	if (prev == NULL)
		list.start_of_list = next;
	else
		prev->next_same_depth = next;
	if (next == NULL)
		list.end_of_list = prev;
	else
		next->prev_same_depth = prev;
}

void GameInstSet::add_to_collisionlist(InstanceState* inst,
		InstanceLinkedList& list) {
//	LANARTS_ASSERT(inst->inst->solid);
	inst->next_in_grid = NULL;
	if (list.start_of_list == NULL) {
		inst->prev_in_grid = NULL;
		list.start_of_list = inst;
		list.end_of_list = inst;

	} else {
		inst->prev_in_grid = list.end_of_list;
		list.end_of_list->next_in_grid = inst;
		list.end_of_list = inst;
	}
}

bool GameInstSet::check_copy_integrity(const GameInstSet & inst_set) const {
	std::vector<GameInst*> v1 = this->to_vector(), v2 = inst_set.to_vector();
	int size = v1.size();
	if (next_id != inst_set.next_id)
		return false;
	if (size != v2.size())
		return false;

	for (int i = 0; i < size; i++) {
		GameInst* i1 = v1[i], *i2 = v2[i];
		if (i1->id != i2->id)
			return false;
		if (i1->last_x != i2->last_x)
			return false;

		if (i1->last_y != i2->last_y)
			return false;

		if (i1->x != i2->x)
			return false;

		if (i1->y != i2->y)
			return false;
	}
	return true;
}

void GameInstSet::remove_from_collisionlist(InstanceState* inst,
		InstanceLinkedList& list) {
	InstanceState* next = inst->next_in_grid;
	InstanceState* prev = inst->prev_in_grid;
	if (prev == NULL)
		list.start_of_list = next;
	else
		prev->next_in_grid = next;
	if (next == NULL)
		list.end_of_list = prev;
	else
		next->prev_in_grid = prev;
}
