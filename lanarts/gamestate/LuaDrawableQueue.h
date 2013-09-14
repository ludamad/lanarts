/*
 * LuaDepthQueue.h:
 *  Efficiently stores lua drawables in descending order of depth.
 *  A lua drawable is either a closure, or an object with a draw method.
 *  The position at which to draw is passed as an argument.
 *
 *  This is cleared at the end of every draw phase.
 */

#ifndef LUADRAWABLEQUEUE_H_
#define LUADRAWABLEQUEUE_H_

#include <vector>
#include <map>

#include <lcommon/geometry.h>

#include <luawrap/LuaValue.h>
#include <luawrap/LuaField.h>

class GameState;

/* Internal convenience types */
struct _LuaDrawableQueueEntry {
	int lua_drawable_index; // Points to LuaDrawableQueue::_drawable_array
	int next_entry_index; // Points to LuaDrawableQueue::_entry_pool, -1 if end-of-list
	PosF position;
};

struct _LuaDrawableList {
	int first_entry_index, last_entry_index; // Points to LuaDrawableQueue::_entry_pool, -1 if non-existent
	_LuaDrawableList() {
		first_entry_index = last_entry_index = -1;
	}
};

class LuaDrawableQueue {
public:
	void add(const LuaField& lua_drawable, int depth, const PosF& position);
	void clear();

	/* NOTE: Does not follow C++ iterator design! */
	class Iterator {
	public:
		Iterator(const LuaDrawableQueue& queue);

		LuaField get_current_object() const;
		int get_current_depth() const;
		void draw_current() const;
		void fetch_next();
		bool is_done() const;
	private:
		const _LuaDrawableQueueEntry* entry_at(int idx) const;

		const LuaDrawableQueue& _queue;
		std::map<int, _LuaDrawableList>::const_iterator _depth_map_iter;
		const _LuaDrawableQueueEntry* _current_entry;
	};

	Iterator get_iterator() const;
private:
	void link_entry(int entry_index, int depth);
	void init(lua_State* L); // Called the first time an object is added

	// Lua array of drawable objects
	LuaValue _drawable_array;
	// Cached {x,y} table object
	LuaValue _cached_position;

	std::vector<_LuaDrawableQueueEntry> _entry_pool;
	std::map<int, _LuaDrawableList> _depth_to_list;
};

#endif /* LUADRAWABLEQUEUE_H_ */
