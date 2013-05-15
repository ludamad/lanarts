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

#include <lcommon/SerializeBuffer.h>
#include <lcommon/math_util.h>

#include "objects/AnimatedInst.h"
#include "objects/InstTypeEnum.h"

#include "GameInstSet.h"
#include "GameState.h"
#include "hashset_util.h"

GameInst* const GAMEINST_TOMBSTONE = (GameInst*)1;

static bool valid_inst(GameInst* inst) {
	return inst > GAMEINST_TOMBSTONE;
}

GameInstSet::GameInstSet(int w, int h, int capacity) {
	grid_w = w / REGION_SIZE + 1, grid_h = h / REGION_SIZE + 1;
	next_id = 1;
	unit_amnt = 0;
	unit_capacity = capacity;
	unit_set.resize(unit_capacity);
	unit_grid.resize(grid_w * grid_h);
}
GameInstSet::~GameInstSet() {

	for (int i = 0, j = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst)) {
			GameInst::free_reference(inst);
		}
	}

	for (int i = 0; i < deallocation_list.size(); i++) {
		GameInst::free_reference(deallocation_list[i]);
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
	if (*stateptr) {
		*stateptr = tset_find<GameInstSetFunctions>((*stateptr)->inst->id,
				&unit_set[0], unit_capacity);
		LANARTS_ASSERT(*stateptr != NULL);
	}
}
void GameInstSet::update_depthlist_for_reallocate_(InstanceLinkedList& list) {
	update_statepointer_for_reallocate_(&list.start_of_list);
	update_statepointer_for_reallocate_(&list.end_of_list);
}
void GameInstSet::reallocate_internal_data() {
	unit_capacity *= 2;
	std::vector<InstanceState> new_set(unit_capacity);

	tset_add_all<GameInstSetFunctions>(&unit_set[0], unit_set.size(),
			&new_set[0], new_set.size());
	unit_set.swap(new_set);

	//Fix pointers for grid
	for (int i = 0; i < unit_capacity; i++) {
		if (unit_set[i].inst) {
			update_statepointer_for_reallocate_(&unit_set[i].prev_in_grid);
			update_statepointer_for_reallocate_(&unit_set[i].next_in_grid);
			update_statepointer_for_reallocate_(&unit_set[i].prev_same_depth);
			update_statepointer_for_reallocate_(&unit_set[i].next_same_depth);
		}
	}

	DepthMap::iterator it = depthlist_map.begin();
	for (; it != depthlist_map.end(); it++) {
		update_depthlist_for_reallocate_(it->second);
	}

	for (int i = 0; i < grid_w * grid_h; i++) {
		update_depthlist_for_reallocate_(unit_grid[i]);
	}
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
	InstanceState* state = tset_find<GameInstSetFunctions>(inst->id,
			&unit_set[0], unit_capacity);
	__remove_instance(state);
	deallocation_list.push_back(inst);
}

void GameInstSet::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write_int(grid_w);
	serializer.write_int(grid_h);

	serializer.write_int(size());
	serializer.write_int(next_id);
	DepthMap::const_iterator it = depthlist_map.end();
	for (int ind = 0; it != depthlist_map.begin();) {
		--it;
		InstanceState* state = it->second.start_of_list;
		while (state) {
			GameInst* inst = state->inst;
			serializer.write_int(get_inst_type(inst));
			serializer.write_int(inst->id);
			inst->serialize(gs, serializer);
//			if (gs->game_settings().network_debug_mode) {
				serializer.write_int(inst->integrity_hash());
//			}
			state = state->next_same_depth;
		}
	}
}

static void safe_deserialize(GameInst* inst, GameState* gs,
		SerializeBuffer& serializer) {
	int lx = inst->last_x, ly = inst->last_y;
	inst->deserialize(gs, serializer);
	inst->last_x = lx, inst->last_y = ly;

}
void GameInstSet::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read_int(grid_w);
	serializer.read_int(grid_h);

	//Resize and clear
	unit_grid.resize(grid_w * grid_h);
	clear();

	int amnt;
	serializer.read_int(amnt);
	serializer.read_int(next_id);

	for (int i = 0; i < amnt; i++) {
		InstType type;
		int id;
		serializer.read_int(type);
		serializer.read_int(id);
		printf("Deserializing id=%d\n", id);
		GameInst* inst = get_instance(id);
		bool has_inst = inst != NULL;
		if (!has_inst) {
			inst = from_inst_type(type);
			inst->deserialize(gs, serializer);
			inst->last_x = inst->x;
			inst->last_y = inst->y;
			inst->id = id;
			add_instance(inst, inst->id);

		} else {
			safe_deserialize(inst, gs, serializer);
		}
//		if (gs->game_settings().network_debug_mode) {
		bool seq = serializer_equals_read(serializer, inst->integrity_hash());
		if (!seq) {
			lanarts_quit();
			abort();
		}
//		}
	}
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
	InstanceState* state = &unit_set[0];

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

	GameInst::retain_reference(inst);

	return inst->id;
}

void GameInstSet::step(GameState* gs) {

	perf_timer_begin(FUNCNAME);
	for (int i = 0; i < deallocation_list.size(); i++) {
		GameInst::free_reference(deallocation_list[i]);
	}
	deallocation_list.clear();
	for (int i = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst)) {
			inst->destroyed = false;
			event_log("Step-event for id %d\n", inst->id);
			inst->step(gs);
			update_instance_for_step(&unit_set[i], inst);
		}
	}
	perf_timer_end(FUNCNAME);
}

GameInst* GameInstSet::get_instance(int id) const {
	InstanceState* is = tset_find<GameInstSetFunctions>(id, &unit_set[0],
			unit_capacity);
	if (is)
		return is->inst;
	return NULL;
}

std::vector<GameInst*> GameInstSet::to_vector() const {
	std::vector<GameInst*> ret(size(), NULL);

	//Begin with a large positive number
	int draw_depth_check = (1 << 20);

	DepthMap::const_iterator it = depthlist_map.end();
	for (int ind = 0; it != depthlist_map.begin();) {
		--it;
		InstanceState* state = it->second.start_of_list;
		while (state) {
			ret[ind++] = state->inst;
#ifndef NDEBUG
			if (state->inst->depth > draw_depth_check) {
				printf(
						"WARNING instance depth out of order for instance: %d, depth %d vs %d\n",
						state->inst->id, state->inst->depth, draw_depth_check);
			}
			draw_depth_check = state->inst->depth;
#endif
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
				GameInst::free_reference(inst);
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
			GameInst::free_reference(inst);
		}
	}
	next_id = 1;
	unit_amnt = 0;
	depthlist_map.clear();
	memset(&unit_set[0], 0, unit_capacity * sizeof(InstanceState));
	memset(&unit_grid[0], 0, grid_w * grid_h * sizeof(InstanceLinkedList));
}

//TODO: Make collisionlist entry positions deterministic -or- make collision functions always return the same object
//this will be important when copying over state updates in client side prediction
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
